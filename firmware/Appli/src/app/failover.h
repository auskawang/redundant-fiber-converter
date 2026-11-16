#ifndef FAILOVER_H
#define FAILOVER_H
#include <stdint.h>
#define FIBER_COUNT 2
#define FAIL_DBM (-18.0f)
#define RECOVERY_MARGIN_DB (2.0f)
#define POWER_DROP_FAIL_DB (6.0f)
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
    float baseline_dbm[FIBER_COUNT];
    float power_drop_db[FIBER_COUNT];
    uint8_t baseline_valid[FIBER_COUNT];
    uint8_t power_drop_valid[FIBER_COUNT];
} FailoverState;
void failover_init(FailoverState *state);
/* Capture a known-good sample for one fiber. Returns 1 on success.
 * A rejected sample leaves the existing reference and reading unchanged.
 * References are RAM-only and survive ticks and path changes until reinitialized.
 */
uint8_t failover_capture_baseline(FailoverState *state, uint8_t fiber, const FiberInput *sample);
/* After each tick, use power_drop_db[i] only when power_drop_valid[i] is set. */
void failover_tick(FailoverState *state, const FiberInput fibers[FIBER_COUNT], uint32_t elapsed_ms);
void failover_ack(FailoverState *state, uint8_t target, uint8_t success);

#endif /* FAILOVER_H_ */

