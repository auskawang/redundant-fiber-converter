/**
 * @file lan96455_irq.c
 * @brief Switch interrupt handling and status decoding implementation.
 */

#include "lan96455_irq.h"
#include "lan96455_driver.h"
#include "lan96455_regs.h"
#include "path_manager.h"
#include "diagnostics.h"

int lan96455_irq_enable(void)
{
    /* Enable link change and fatal error interrupt masks */
    return lan96455_write_reg(LAN96455_REG_INT_MASK, 0xFFFFFFFFU);
}

void lan96455_irq_handler_callback(void)
{
    uint32_t status = 0;
    if (lan96455_read_reg(LAN96455_REG_INT_STATUS, &status) == 0)
    {
        /* Clear pending interrupt */
        lan96455_write_reg(LAN96455_REG_INT_STATUS, status);

        diagnostics_log_event(DIAG_SEV_WARNING, "Switch: Hardware IRQ triggered");
    }
}
