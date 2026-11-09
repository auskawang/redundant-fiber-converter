/* USER CODE BEGIN */
/**
 * @file sfp_driver.h
 * @brief SFP optical transceiver I2C communication (SFF-8472 DDM & SFF-8074 EEPROM).
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
    SFP_PORT_PRIMARY = 0,   /**< Cage connected to switch port 5. */
    SFP_PORT_BACKUP = 1,    /**< Cage connected to switch port 6. */
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

/** Zero optical power has no finite dBm value. Legacy fixed-point sentinel. */
#define SFP_POWER_DBM_TENTHS_ZERO INT32_MIN

typedef struct {
    bool gpio_valid;       /**< All three GPIO signals were sampled successfully. */
    bool module_present;
    bool los;
    bool tx_fault;
    bool ddm_valid;        /**< This call acquired ready, internally calibrated DDM. */
    float rx_power_dbm;    /**< -INFINITY for a valid zero-power reading. */
    float tx_power_dbm;    /**< -INFINITY for a valid zero-power reading. */
    float tx_bias_current_ma;
    float temperature_c;
    float supply_voltage_v;
} sfp_status_t;

/**
 * Acquire a fresh per-cage snapshot. Call from the main loop/task, NEVER an ISR:
 * the I2C backend blocks. Serialize calls with board reconfiguration.
 * Output is cleared on every call; telemetry is NAN unless ddm_valid is true.
 * GPIO remains usable when gpio_valid is true even if DDM acquisition fails.
 * LOS/TX_FAULT do not invalidate a correctly acquired DDM measurement.
 * Returns SFP_OK only with valid DDM; otherwise an SFP_ERR_* code.
 * Supports internally calibrated modules; external calibration is rejected.
 */
int sfp_driver_get_status(sfp_port_t port, sfp_status_t *status);

typedef struct {
    int32_t  temperature_millicelsius;  /**< SFP internal temperature (mC) */
    uint32_t supply_voltage_mv;         /**< Supply voltage in mV */
    uint32_t tx_bias_current_ua;        /**< TX Laser bias current in uA */
    int32_t  tx_power_dbm_tenths;       /**< TX Optical power in 0.1 dBm */
    int32_t  rx_power_dbm_tenths;       /**< RX Optical power (RSSI) in 0.1 dBm */
} sfp_ddm_data_t;

/**
 * @brief Sets both cages' LEDs OFF after CubeMX GPIO initialization.
 */
void sfp_driver_init(void);

/**
 * @brief Checks if SFP optical module is physically inserted.
 * @return True if MOD_ABS indicates module presence.
 */
bool sfp_driver_is_present(sfp_port_t port);

/**
 * @brief Reads DDM diagnostic telemetry registers from address A2h.
 * @param[out] ddm Pointer to output telemetry structure.
 * @return 0 on success, negative error code on failure.
 */
int sfp_driver_read_ddm(sfp_port_t port, sfp_ddm_data_t *ddm);

/**
 * @brief Reads vendor and module identification from address A0h.
 * @param[out] buffer Output buffer.
 * @param length Bytes to read.
 * @return 0 on success, negative error code on failure.
 */
int sfp_driver_read_id(sfp_port_t port, uint8_t *buffer, uint16_t length);

/* Fixed CubeMX GPIO mapping. Active-low presence; active-high LOS/TX_FAULT. */
int sfp_driver_read_gpio_status(sfp_port_t port, bool *present, bool *los, bool *tx_fault);
int sfp_driver_set_tx_disable(sfp_port_t port, bool disable);

/* Active-high LEDs: true = ON, false = OFF.
 * Automatic status polling reapplies presence/fault indication. */
int sfp_driver_set_mod_led(sfp_port_t port, bool on);
int sfp_driver_set_fault_led(sfp_port_t port, bool on);

/* Task processing polls both cages' GPIO status and refreshes LEDs without I2C. */
void sfp_driver_process(void);
#ifdef __cplusplus
}
#endif

#endif /* SFP_DRIVER_H */
/* USER CODE END */
