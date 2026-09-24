/**
 * @file sfp_gpio.c
 * @brief Implementation of SFP discrete pin controls.
 */

#include "sfp_gpio.h"
#include "board_pins.h"
#include "stm32n6xx_hal.h"
#include "path_manager.h"
#include "diagnostics.h"

void sfp_gpio_set_tx_disable(bool disable)
{
    HAL_GPIO_WritePin(PIN_SFP_TX_DISABLE_PORT, PIN_SFP_TX_DISABLE_PIN,
                      disable ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool sfp_gpio_is_los(void)
{
    /* LOS is active HIGH */
    return (HAL_GPIO_ReadPin(PIN_SFP_LOS_PORT, PIN_SFP_LOS_PIN) == GPIO_PIN_SET);
}

bool sfp_gpio_is_module_present(void)
{
    /* MOD_ABS is active LOW when inserted */
    return (HAL_GPIO_ReadPin(PIN_SFP_MOD_ABS_PORT, PIN_SFP_MOD_ABS_PIN) == GPIO_PIN_RESET);
}

bool sfp_gpio_is_tx_fault(void)
{
    /* TX_FAULT is active HIGH */
    return (HAL_GPIO_ReadPin(PIN_SFP_TX_FAULT_PORT, PIN_SFP_TX_FAULT_PIN) == GPIO_PIN_SET);
}

void sfp_gpio_los_isr_callback(void)
{
    if (sfp_gpio_is_los())
    {
        diagnostics_log_event(DIAG_SEV_ERROR, "SFP: Hardware LOS asserted!");
        path_manager_trigger_event(PATH_EVENT_LINK_DOWN);
    }
    else
    {
        diagnostics_log_event(DIAG_SEV_INFO, "SFP: Hardware LOS de-asserted");
        path_manager_trigger_event(PATH_EVENT_RESTORE);
    }
}
