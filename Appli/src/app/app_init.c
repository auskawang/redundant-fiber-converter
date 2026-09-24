/**
 * @file app_init.c
 * @brief Power sequencing and reset line control implementation.
 */

#include "app_init.h"
#include "board_pins.h"
#include "stm32n6xx_hal.h"
#include "diagnostics.h"

void app_init_power_sequence(void)
{
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

void app_init_reset_switch(void)
{
    HAL_GPIO_WritePin(PIN_SWITCH_RESET_N_PORT, PIN_SWITCH_RESET_N_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(PIN_SWITCH_RESET_N_PORT, PIN_SWITCH_RESET_N_PIN, GPIO_PIN_SET);
    HAL_Delay(50);
}

void app_init_reset_phys(void)
{
    HAL_GPIO_WritePin(PIN_PHY_RESET_N_PORT, PIN_PHY_RESET_N_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(PIN_PHY_RESET_N_PORT, PIN_PHY_RESET_N_PIN, GPIO_PIN_SET);
    HAL_Delay(20);
}
