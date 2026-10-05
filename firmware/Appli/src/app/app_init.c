/**
 * @file app_init.c
 * @brief Power sequencing and reset line control implementation.
 *
 * Implements power sequencing for switch
 */

#include "app_init.h"
#include "board_pins.h"
#include "stm32n6xx_hal.h"
#include "diagnostics.h"

void app_init_power_sequence(void)
{
    //enable load switch 3.3V -> ethernet switch
    HAL_GPIO_WritePin(STM_ETH_SW_LS_EN_PORT, STM_ETH_SW_LS_EN_PIN, GPIO_PIN_SET);
    HAL_Delay(20);  //TODO: replace with recommended value
    app_init_reset_switch();
}

void app_init_reset_switch(void)
{
    HAL_GPIO_WritePin(ETH_SW_RESET_N_PORT, ETH_SW_RESET_N_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);  //TODO: replace with appropriate delay between switching reset line

    HAL_GPIO_WritePin(ETH_SW_RESET_N_PORT, ETH_SW_RESET_N_PIN, GPIO_PIN_SET);

    HAL_Delay(15);  //TODO: replace with required time for switch PLL and clock to be ready
}
