/* USER CODE BEGIN */
/**
 * @file sfp_driver.h
 * @brief SFP optical transceiver I2C communication (SFF-8472 DDM & SFF-8074 EEPROM).
 * @ingroup sfp_driver
 */

/**
 * @defgroup sfp_driver SFP Driver
 * @brief SFF-8472 DDM telemetry acquisition for the
 *        Coherent FTLF8519P3BNL optical transceivers.
 * @{
 */

#ifndef SFP_DRIVER_H
#define SFP_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SFP_I2C_ADDR_A0     (0x50 << 1) /**< SFF-8074 serial ID memory */
#define SFP_I2C_ADDR_A2     (0x51 << 1) /**< SFF-8472 diagnostic monitoring memory */

typedef enum {
    SFP_PORT_PRIMARY = 0,
    SFP_PORT_BACKUP = 1,
    SFP_PORT_COUNT = 2
} sfp_port_t;

enum {
    SFP_OK = 0,
    SFP_ERR_ARGUMENT = -1,
    SFP_ERR_IO = -2,
    SFP_ERR_ABSENT = -3,
    SFP_ERR_UNSUPPORTED = -4,
    SFP_ERR_NOT_READY = -5,
    SFP_ERR_GPIO = -6
};

typedef struct {
    bool valid;
    bool module_present;
    bool los;
    bool tx_fault;
    float rx_power_dbm;
    float tx_power_dbm;
    float tx_bias_current_ma;
    float temperature_c;
    float supply_voltage_v;
} sfp_status_t;

/**
 * @brief Initializes the SFP subsystem.
 *
 * Enables power to both cages through load switch, then for each cage
 * verifies that the installed module supports internal
 * calibration, and waits for the module's diagnostic
 * data to become valid.
 * Finally updates the SFPs LEDs based on their status.
 *
 * @note Blocks in a polling loop until both cages report DDM ready.
 *       Call after CubeMX GPIO initialization.
 * @return ::SFP_OK on success, or a negative ::SFP_ERR_* code on failure.
 */
int sfp_driver_init(void);

/**
 * @brief Checks whether an SFP module is inserted in a cage.
 *
 * Reads the active-low MOD_DEF0 presence pin for the selected cage.
 *
 * @param[in] port SFP cage index (::SFP_PORT_PRIMARY or ::SFP_PORT_BACKUP).
 * @return @c true if the module is present; @c false if absent or @p port is invalid.
 */
bool sfp_is_port_present(sfp_port_t port);

/**
 * @brief Fetch telemetry for a SFP cage.
 *
 * @param[in]  port   SFP cage index (::SFP_PORT_PRIMARY or ::SFP_PORT_BACKUP).
 * @param[out] status Caller-allocated struct to populate. Must not be NULL.
 *
 * @return ::SFP_OK with valid telemetry on success.
 * @return ::SFP_ERR_ARGUMENT if @p status is NULL or @p port is out of range.
 * @return ::SFP_ERR_IO       if any I2C transaction fails.
 *
 * @warning Blocks on I2C. Must not be called from an interrupt context.
 */
int sfp_driver_get_diagnostics(sfp_port_t port, sfp_status_t *status);

#ifdef __cplusplus
}
#endif

#endif /* SFP_DRIVER_H */
/** @} */
/* USER CODE END */
