#include "peer_comm.h"

#include <string.h>

enum {
    FRAME_SIZE = 60,
    REQUEST = 1, READY, COMMIT, DONE,
    RETRY_MS = 25,
    PREPARE_MS = 1000,
    SWITCH_DELAY_MS = 300,
    VERIFY_MS = 1500
};

static bool due(uint32_t now, uint32_t deadline)
{
    return now - deadline < UINT32_C(0x80000000);
}

static void put32(uint8_t *out, uint32_t value)
{
    out[0] = (uint8_t)(value >> 24);
    out[1] = (uint8_t)(value >> 16);
    out[2] = (uint8_t)(value >> 8);
    out[3] = (uint8_t)value;
}

static uint32_t get32(const uint8_t *in)
{
    return ((uint32_t)in[0] << 24) | ((uint32_t)in[1] << 16) |
           ((uint32_t)in[2] << 8) | in[3];
}

static uint32_t next_id(PeerComm *p)
{
    ++p->counter;
    if (!p->counter) {
        ++p->counter;
    }
    return p->counter;
}

static bool unicast(const uint8_t mac[6])
{
    const uint8_t zero[6] = {0};
    return !(mac[0] & 1u) && memcmp(mac, zero, sizeof(zero)) != 0;
}

bool peer_comm_init(PeerComm *p, const PeerConfig *config, FailoverState *s, uint32_t now)
{
    if (!p || !config || !s || !config->boot_id || !config->transmit ||
        !config->set_path || !unicast(config->local_mac) || !unicast(config->remote_mac) ||
        !memcmp(config->local_mac, config->remote_mac, 6)) {
        return false;
    }

    memset(p, 0, sizeof(*p));
    p->config = *config;
    p->failover = s;
    p->last_tick = now;
    return true;
}

static bool standby_ready(const PeerComm *p, uint8_t target)
{
    const FailoverState *s = p->failover;
    return target < FIBER_COUNT && target != s->active &&
           s->health[target] == LINK_GOOD &&
           s->good_ms[target] >= RECOVERY_CONFIRM_MS;
}

static int send_message(PeerComm *p, uint8_t type, uint32_t delay)
{
    uint8_t frame[FRAME_SIZE] = {0};
    memcpy(frame, p->config.remote_mac, 6);
    memcpy(frame + 6, p->config.local_mac, 6);
    frame[12] = 0x88;
    frame[13] = 0xb5;
    memcpy(frame + 14, "PFC1", 4);
    frame[18] = type;
    frame[19] = p->target;
    put32(frame + 20, p->origin);
    put32(frame + 24, p->request);
    put32(frame + 28, p->responder_boot);
    put32(frame + 32, p->token);
    put32(frame + 36, delay);
    return p->config.transmit(p->config.context, frame, sizeof(frame));
}

static void cancel(PeerComm *p)
{
    FailoverState *s = p->failover;
    if (s->pending) {
        failover_ack(s, s->target, 0);
    }
    p->phase = PEER_IDLE;
}

static void apply_path(PeerComm *p, uint32_t now)
{
    FailoverState *s = p->failover;
    s->pending = 1;
    s->target = p->target;
    p->apply_failed = p->config.set_path(p->config.context, p->target) != 0;
    failover_ack(s, p->target, !p->apply_failed);
    p->phase = p->apply_failed ? PEER_UNCERTAIN : PEER_VERIFYING;
    p->deadline = now + VERIFY_MS;
    p->next_tx = now;
}

void peer_comm_tick(PeerComm *p, const FiberInput fibers[FIBER_COUNT], uint32_t now)
{
    if (!p || !fibers) {
        return;
    }

    FailoverState *s = p->failover;
    failover_tick(s, fibers, now - p->last_tick);
    p->last_tick = now;
    if (p->phase == PEER_IDLE && s->pending) {
        if (!standby_ready(p, s->target)) {
            return;
        }
        p->phase = PEER_REQUESTING;
        p->target = s->target;
        p->origin = p->config.boot_id;
        p->request = next_id(p);
        p->responder_boot = 0;
        p->token = 0;
        p->confirmed = false;
        p->deadline = now + PREPARE_MS;
        p->next_tx = now;
    }

    if (due(now, p->deadline)) {
        switch (p->phase) {
        case PEER_REQUESTING:
        case PEER_PREPARED:
            cancel(p);
            return;
        case PEER_COMMITTING:
        case PEER_ARMED:
            apply_path(p, now);
            break;
        case PEER_VERIFYING:
            p->phase = p->confirmed ? PEER_IDLE : PEER_UNCERTAIN;
            return;
        default:
            break;
        }
    }

    if (!due(now, p->next_tx)) {
        return;
    }
    switch (p->phase) {
    case PEER_REQUESTING:
        if (s->health[s->active] == LINK_GOOD || !standby_ready(p, p->target)) {
            cancel(p);
            return;
        }
        send_message(p, REQUEST, 0);
        break;
    case PEER_PREPARED:
        if (!standby_ready(p, p->target)) {
            cancel(p);
            return;
        }
        send_message(p, READY, 0);
        break;
    case PEER_COMMITTING:
        send_message(p, COMMIT, p->deadline - now);
        break;
    case PEER_VERIFYING:
        send_message(p, DONE, 0);
        break;
    default:
        return;
    }
    p->next_tx = now + RETRY_MS;
}

bool peer_comm_receive(PeerComm *p, const uint8_t *frame, uint16_t length, uint32_t now)
{
    if (!p || !frame || length < FRAME_SIZE ||
        memcmp(frame, p->config.local_mac, 6) ||
        memcmp(frame + 6, p->config.remote_mac, 6) ||
        frame[12] != 0x88 || frame[13] != 0xb5 || memcmp(frame + 14, "PFC1", 4)) {
        return false;
    }

    uint8_t type = frame[18];
    uint8_t target = frame[19];
    uint32_t origin = get32(frame + 20);
    uint32_t request = get32(frame + 24);
    uint32_t responder_boot = get32(frame + 28);
    uint32_t token = get32(frame + 32);
    uint32_t delay = get32(frame + 36);

    if (target >= FIBER_COUNT || !origin || !request || type < REQUEST || type > DONE ||
        (type == REQUEST && (token || responder_boot)) ||
        (type != REQUEST && (!token || !responder_boot)) ||
        (type == COMMIT ? (!delay || delay > SWITCH_DELAY_MS) : delay != 0)) {
        return false;
    }

    if (type == REQUEST) {
        bool yields = p->phase == PEER_REQUESTING &&
                      memcmp(p->config.remote_mac, p->config.local_mac, 6) < 0;
        if ((p->phase == PEER_IDLE || yields) && !p->failover->hold_ms &&
            standby_ready(p, target)) {
            p->phase = PEER_PREPARED;
            p->target = target;
            p->origin = origin;
            p->request = request;
            p->responder_boot = p->config.boot_id;
            p->token = next_id(p);
            p->confirmed = false;
            p->deadline = now + PREPARE_MS;
            p->next_tx = now;
        }
        return true;
    }

    if (target != p->target || origin != p->origin || request != p->request ||
        due(now, p->deadline)) {
        return true;
    }

    if (type == READY && p->phase == PEER_REQUESTING &&
        p->failover->health[p->failover->active] == LINK_FAILED &&
        standby_ready(p, target)) {
        p->responder_boot = responder_boot;
        p->token = token;
        /* Schedule locally only after the transport accepts a commit frame. */
        if (send_message(p, COMMIT, SWITCH_DELAY_MS) == 0) {
            p->phase = PEER_COMMITTING;
            p->deadline = now + SWITCH_DELAY_MS;
            p->next_tx = now + RETRY_MS;
        }
        return true;
    }

    if (token != p->token || responder_boot != p->responder_boot) {
        return true;
    }
    if (type == COMMIT && p->phase == PEER_PREPARED && standby_ready(p, target)) {
        p->phase = PEER_ARMED;
        p->deadline = now + delay;
    }
    if (type == DONE && p->phase == PEER_VERIFYING) {
        p->confirmed = true;
    }
    return true;
}
