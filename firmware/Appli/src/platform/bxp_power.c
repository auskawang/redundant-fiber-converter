#include "bxp_gpio.h"
#include "board_pins.h"
#include "stm32n6xx_hal.h"
#include "diagnostics.h"
void bxp_gpio_power_sequence(void)
{
    if (!bxp_gpio_is_ready()) return;
    diagnostics_log_event(DIAG_SEV_INFO, "AppInit: Starting power sequencing");

    /* 1. Assert power rail enables */
    HAL_GPIO_WritePin(PIN_POWER_SEQ_EN_PORT, PIN_POWER_SEQ_EN_PIN, GPIO_PIN_SET);
    HAL_Delay(10);

    /* 2. Hold RESET_N lines low during rail stabilization */
    HAL_GPIO_WritePin(PIN_SWITCH_RESET_N_PORT, PIN_SWITCH_RESET_N_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PIN_PHY_RESET_N_PORT, PIN_PHY_RESET_N_PIN, GPIO_PIN_RESET);
    HAL_Delay(25);

    /* 3. Release Switch Reset */
    HAL_GPIO_WritePin(PIN_SWITCH_RESET_N_PORT, PIN_SWITCH_RESET_N_PIN, GPIO_PIN_SET);
    HAL_Delay(10);

    /* 4. Release PHY Reset */
    HAL_GPIO_WritePin(PIN_PHY_RESET_N_PORT, PIN_PHY_RESET_N_PIN, GPIO_PIN_SET);
    HAL_Delay(50);

    diagnostics_log_event(DIAG_SEV_INFO, "AppInit: Hardware out of reset");
}

void bxp_gpio_reset_switch(void)
{
    if (!bxp_gpio_is_ready()) return;
    HAL_GPIO_WritePin(PIN_SWITCH_RESET_N_PORT, PIN_SWITCH_RESET_N_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(PIN_SWITCH_RESET_N_PORT, PIN_SWITCH_RESET_N_PIN, GPIO_PIN_SET);
    HAL_Delay(50);
}

void bxp_gpio_reset_phys(void)
{
    if (!bxp_gpio_is_ready()) return;
    HAL_GPIO_WritePin(PIN_PHY_RESET_N_PORT, PIN_PHY_RESET_N_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(PIN_PHY_RESET_N_PORT, PIN_PHY_RESET_N_PIN, GPIO_PIN_SET);
    HAL_Delay(20);
}
