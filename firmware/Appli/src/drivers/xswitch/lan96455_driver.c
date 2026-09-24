/**
 * @file lan96455_driver.c
 * @brief Switch driver implementation using SPI peripheral.
 */

#include "lan96455_driver.h"
#include "lan96455_regs.h"
#include "bxp_spi.h"
#include "diagnostics.h"

int lan96455_init(void)
{
    uint32_t chip_id = 0;
    if (lan96455_read_reg(LAN96455_REG_CHIP_ID, &chip_id) != 0)
    {
        diagnostics_log_event(DIAG_SEV_ERROR, "Switch: Failed to read Chip ID via SPI");
        return -1;
    }

    diagnostics_log_event(DIAG_SEV_INFO, "Switch: LAN96455 detected and operational");
    return 0;
}

int lan96455_read_reg(uint32_t reg_addr, uint32_t *value)
{
    if (!value)
    {
        return -1;
    }
    return bxp_spi_read_reg(reg_addr, value);
}

int lan96455_write_reg(uint32_t reg_addr, uint32_t value)
{
    return bxp_spi_write_reg(reg_addr, value);
}

int lan96455_flush_mac_table(void)
{
    return lan96455_write_reg(LAN96455_REG_MAC_FLUSH_CTRL, LAN96455_MAC_FLUSH_DYNAMIC);
}
