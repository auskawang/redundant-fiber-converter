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

typedef struct {
    int32_t  temperature_millicelsius;  /**< SFP internal temperature (mC) */
    uint32_t supply_voltage_mv;         /**< Supply voltage in mV */
    uint32_t tx_bias_current_ua;        /**< TX Laser bias current in uA */
    int32_t  tx_power_dbm_tenths;       /**< TX Optical power in 0.1 dBm */
    int32_t  rx_power_dbm_tenths;       /**< RX Optical power (RSSI) in 0.1 dBm */
} sfp_ddm_data_t;

/**
 * @brief Initializes the SFP I2C driver interface.
 */
void sfp_driver_init(void);

/**
 * @brief Checks if SFP optical module is physically inserted.
 * @return True if MOD_ABS indicates module presence.
 */
bool sfp_driver_is_present(void);

/**
 * @brief Reads DDM diagnostic telemetry registers from address A2h.
 * @param[out] ddm Pointer to output telemetry structure.
 * @return 0 on success, negative error code on failure.
 */
int sfp_driver_read_ddm(sfp_ddm_data_t *ddm);

/**
 * @brief Reads vendor and module identification from address A0h.
 * @param[out] buffer Output buffer.
 * @param length Bytes to read.
 * @return 0 on success, negative error code on failure.
 */
int sfp_driver_read_id(uint8_t *buffer, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* SFP_DRIVER_H */
