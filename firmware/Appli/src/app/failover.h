#ifndef FAILOVER_H
#define FAILOVER_H
#include <stdint.h>
#define FIBER_COUNT 2
#define FAIL_DBM (-18.0f)
#define RECOVERY_MARGIN_DB (2.0f)
#define FAILURE_CONFIRM_MS (3000u)
#define RECOVERY_CONFIRM_MS (3000u)
#define HOLD_DOWN_MS (5000u)
typedef enum { LINK_FAILED=0, LINK_GOOD=1 } LinkHealth;
typedef struct { float rx_dbm; uint8_t los, fault, valid, present; } FiberInput;
typedef struct {
    uint8_t active;
    uint8_t pending;
    uint8_t target;
    uint32_t bad_ms[FIBER_COUNT];
    uint32_t good_ms[FIBER_COUNT];
    uint32_t hold_ms;
    LinkHealth health[FIBER_COUNT];
} FailoverState;
void failover_init(FailoverState *state);
void failover_tick(FailoverState *state, const FiberInput fibers[FIBER_COUNT], uint32_t elapsed_ms);
void failover_ack(FailoverState *state, uint8_t target, uint8_t success);

#endif /* FAILOVER_H_ */

