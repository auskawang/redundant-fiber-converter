#include "failover.h"
#include <string.h>
#include <limits.h>
static uint32_t add_sat(uint32_t a, uint32_t b) { return UINT32_MAX-a < b ? UINT32_MAX : a+b; }
void failover_init(FailoverState *s) { memset(s,0,sizeof(*s)); s->health[0]=LINK_GOOD; s->health[1]=LINK_GOOD; }
void failover_tick(FailoverState *s, const FiberInput f[FIBER_COUNT], uint32_t dt) {
    if (!s || !f) return;
    s->hold_ms = s->hold_ms > dt ? s->hold_ms-dt : 0;
    for (int i=0;i<FIBER_COUNT;i++) {
        uint8_t hard_fault = !f[i].present || !f[i].valid || f[i].los || f[i].fault;
        uint8_t bad = hard_fault || f[i].rx_dbm < FAIL_DBM;
        uint8_t recovered = !hard_fault && f[i].rx_dbm >= FAIL_DBM+RECOVERY_MARGIN_DB;
        s->bad_ms[i] = bad ? add_sat(s->bad_ms[i],dt) : 0;
        s->good_ms[i] = recovered ? add_sat(s->good_ms[i],dt) : 0;
        if (s->health[i] == LINK_GOOD && s->bad_ms[i]>=FAILURE_CONFIRM_MS) s->health[i]=LINK_FAILED;
        if (s->health[i] == LINK_FAILED && s->good_ms[i]>=RECOVERY_CONFIRM_MS) s->health[i]=LINK_GOOD;
    }
    uint8_t standby = 1u-s->active;
    if (!s->pending && s->hold_ms==0 && s->health[s->active]==LINK_FAILED && s->health[standby]==LINK_GOOD) {
        s->pending=1; s->target=standby;
    }
}
void failover_ack(FailoverState *s, uint8_t target, uint8_t success) {
    if (!s || !s->pending || target!=s->target) return;
    if (success) { s->active=target; s->hold_ms=HOLD_DOWN_MS; }
    else { s->hold_ms=HOLD_DOWN_MS; }
    s->pending=0;
}

