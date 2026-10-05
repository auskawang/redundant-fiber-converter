/**
 * @file lan96455_driver.c
 * @brief Switch driver implementation using SPI peripheral.
 *
 * Implements the 4-phase startup sequence ported from the Linux MFD and DSA drivers.
 */

#include "lan96455_driver.h"
#include "lan96455_regs.h"
#include "bxp_spi.h"
#include "diagnostics.h"
#include "stm32n6xx_hal.h"

int lan96455_read_reg(uint32_t reg_addr, uint32_t *value)
{
    if (value == NULL)
    {
        return -1;
    }
    return bxp_spi_read_reg(reg_addr, value);
}

int lan96455_write_reg(uint32_t reg_addr, uint32_t value)
{
    return bxp_spi_write_reg(reg_addr, value);
}

int lan96455_verify_spi_comm(void)
{
    uint32_t val = 0;
    int ret = 0;

    /* Write padding bytes register and confirm reception by reading it back */
    uint32_t cfgstat_val = (LAN9645X_SPI_OPERATIONAL_PADDING & IF_CFGSTAT_PADDING_MASK) << IF_CFGSTAT_PADDING_SHIFT;
    ret = lan96455_write_reg(DEVCPU_GCB_IF_CFGSTAT, cfgstat_val);
    if (ret != 0)
    {
        return ret;
    }

    /* Verification readback after brief stabilization delay */
    HAL_Delay(5);
    ret = lan96455_read_reg(DEVCPU_GCB_IF_CFGSTAT, &val);
    if (ret != 0)
    {
        return ret;
    }

    /* Verify padding configured matches readback */
    if ((val & IF_CFGSTAT_PADDING_MASK) != (LAN9645X_SPI_OPERATIONAL_PADDING & IF_CFGSTAT_PADDING_MASK))
    {
        return -2; /* Padding mismatch verification failure */
    }

    return 0;
}

int lan96455_soft_reset(void)
{
    uint32_t val = 0;
    int ret = 0;
    uint32_t start_tick = 0;

    /* 1. Issue Soft Switch Core Reset */
    ret = lan96455_write_reg(DEVCPU_GCB_SOFT_RST, SOFT_SWC_RST);
    if (ret != 0)
    {
        return ret;
    }

    /* 2. Poll SOFT_RST until SOFT_SWC_RST self-clears.
     * Note: Soft reset reverts SPI block to default (15) padding bytes.
     */
    start_tick = HAL_GetTick();
    while (1)
    {
        ret = bxp_spi_read_reg_default_padding(DEVCPU_GCB_SOFT_RST, &val);
        if (ret == 0 && !(val & SOFT_SWC_RST))
        {
            break; /* Reset completed */
        }

        if ((HAL_GetTick() - start_tick) >= LAN9645X_GCB_RST_TIMEOUT_MS)
        {
            return -3; /* Reset timeout */
        }

        HAL_Delay(LAN9645X_POLL_INTERVAL_MS);
    }

    /* 3. Configure operational SPI padding bytes for normal register access */
    uint32_t cfgstat_val = (LAN9645X_SPI_OPERATIONAL_PADDING & IF_CFGSTAT_PADDING_MASK) << IF_CFGSTAT_PADDING_SHIFT;
    return lan96455_write_reg(DEVCPU_GCB_IF_CFGSTAT, cfgstat_val);
}

int lan96455_ram_init(void)
{
    uint32_t val = 0;
    int ret = 0;
    uint32_t start_tick = 0;

    /* 1. Disable Switch Core */
    ret = lan96455_write_reg(SYS_RESET_CFG, 0);
    if (ret != 0)
    {
        return ret;
    }

    /* 2. Trigger Internal SRAM BIST / Initialization */
    ret = lan96455_write_reg(SYS_RAM_INIT, SYS_RAM_INIT_RAM_INIT);
    if (ret != 0)
    {
        return ret;
    }

    /* 3. Poll until RAM_INIT self-clears to 0 */
    start_tick = HAL_GetTick();
    while (1)
    {
        ret = lan96455_read_reg(SYS_RAM_INIT, &val);
        if (ret == 0 && !(val & SYS_RAM_INIT_RAM_INIT))
        {
            break; /* RAM init completed */
        }

        if ((HAL_GetTick() - start_tick) >= LAN9645X_RAM_INIT_TIMEOUT_MS)
        {
            return -4; /* RAM init timeout */
        }

        HAL_Delay(LAN9645X_POLL_INTERVAL_MS);
    }

    /* 4. Release Packet Engine Core from Reset */
    return lan96455_write_reg(SYS_RESET_CFG, SYS_RESET_CFG_CORE_ENA);
}

int lan96455_subsystem_init(void)
{
    int ret = 0;

    /* 1. Flush extraction queues */
    ret = lan96455_write_reg(QS_XTR_FLUSH, QS_XTR_FLUSH_DRAIN);
    if (ret != 0)
    {
        return ret;
    }
    HAL_Delay(1);
    lan96455_write_reg(QS_XTR_FLUSH, 0);

    /* 2. TODO: Configure Interrupt Controller (lan9645x-oic) and GPIO multiplexing */

    /* 3. TODO: Initialize TCAM / VCAP blocks (IS0, IS1, IS2, ES0) */

    /* 4. TODO: Configure front ports, SerDes PHYs, and Host CPU / NPI port */

    return 0;
}

int lan96455_init(void)
{
    int ret = 0;

    /* Phase 2: Soft Switch Reset & Polling */
    ret = lan96455_soft_reset();
    if (ret != 0)
    {
        return ret;
    }

    /* Phase 3: Switch Core Reset & RAM Init */
    ret = lan96455_ram_init();
    if (ret != 0)
    {
        return ret;
    }

    /* Phase 4: Functional Register Configuration */
    ret = lan96455_subsystem_init();
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}
