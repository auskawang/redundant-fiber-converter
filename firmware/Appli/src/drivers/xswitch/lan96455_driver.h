/**
 * @file lan96455_driver.h
 * @brief LAN9646 / LAN9645x SPI communication and core register access.
 */

#ifndef LAN96455_DRIVER_H
#define LAN96455_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Orchestrates the complete 4-phase startup and initialization sequence.
 * @return 0 on success, negative error code on failure.
 */
int lan96455_init(void);

/**
 * @brief Side function: Verifies bidirectional SPI communication with the switch via write-and-readback.
 * @note Standalone diagnostic check; not part of the active startup sequence.
 * @return 0 on success, negative error code on failure.
 */
int lan96455_verify_spi_comm(void);

/**
 * @brief Phase 2: Issues a soft switch core reset, polls until completed, and sets operational padding.
 * @return 0 on success, negative error code on failure.
 */
int lan96455_soft_reset(void);

/**
 * @brief Phase 3: Disables core, triggers internal SRAM BIST/init, polls for ready, and re-enables core.
 * @return 0 on success, negative error code on failure.
 */
int lan96455_ram_init(void);

/**
 * @brief Phase 4: Configures interrupts, TCAM, queue flushing, and port routing.
 * @return 0 on success, negative error code on failure.
 */
int lan96455_subsystem_init(void);

/**
 * @brief Reads a 32-bit register from the switch via SPI with operational padding bytes.
 * @param reg_addr 32-bit register address.
 * @param[out] value Pointer to read output.
 * @return 0 on success, negative error code on failure.
 */
int lan96455_read_reg(uint32_t reg_addr, uint32_t *value);

/**
 * @brief Writes a 32-bit register to the switch via SPI.
 * @param reg_addr 32-bit register address.
 * @param value Value to write.
 * @return 0 on success, negative error code on failure.
 */
int lan96455_write_reg(uint32_t reg_addr, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif /* LAN96455_DRIVER_H */
