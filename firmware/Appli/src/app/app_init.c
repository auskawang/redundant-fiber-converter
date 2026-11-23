/* USER CODE BEGIN */
/**
 * @file app_init.c
 * @brief Power sequencing and reset line control implementation.
 *
 * Implements power sequencing for switch
 */

#include "app_init.h"
#include "stm32n6xx_hal.h"
#include "main.h"
#include "diagnostics.h"

#define T_3V3_ETH_SW_LS_SETTLE 30
#define T_CLK_START_MAX 100
void app_init_power_sequence(void)
{
    //     MCU POWER SEQUENCING
    // STM_ETH_SW_LS_EN  = LOW
    // STM_1V15_EN       = LOW
    // STM_ETH_SW_NRESET = LOW       // open-drain: hold reset   ---------- these pins are defaulted LOW
    // wait_until(3V3_PG is HIGH) --------- 3V3_PG is not an input to the STM32 according to schematic, so probably just have to insert a long delay instead
    // STM_ETH_SW_LS_EN = HIGH
    // wait(T_3V3_ETH_SW_LS_SETTLE)   // monotonic ramps; all rails up within 100 ms
    // STM_1V15_EN = HIGH to enable the 1.15 V switch rail
    // wait_until(BUCK_1V15_PG is HIGH)
    // wait(T_CLK_START_MAX + 2 ms)  // reset LOW ≥2 ms after rails + clock stable
    // STM_ETH_SW_NRESET = Hi-Z      // supervisor adds 140–280 ms before release
    // wait(280 ms)                 // allow maximum delay; rails must remain valid
    // LAN_init_and_check_ready()
    // // T_3V3_ETH_SW_LS_SETTLE and T_CLK_START_MAX remain TBD.


    HAL_Delay(200);  //replacement of checking that the 3V3_PG is high
    HAL_GPIO_WritePin(STM_ETH_SW_LS_EN_GPIO_Port, STM_ETH_SW_LS_EN_Pin, GPIO_PIN_SET); //enable load switch 3.3V -> ethernet switch
    HAL_Delay(T_3V3_ETH_SW_LS_SETTLE);   // monotonic ramps; all rails up within 100 ms
    HAL_GPIO_WritePin(STM_1V15_EN_GPIO_Port, STM_1V15_EN_Pin, GPIO_PIN_SET); //enable stm 1v15
    while (HAL_GPIO_ReadPin(BUCK_1V15_PG_GPIO_Port, BUCK_1V15_PG_Pin) != GPIO_PIN_SET) {} // Wait for the enabled 1.15 V buck to report power-good.
    HAL_Delay(T_CLK_START_MAX + 2);  //TODO: replace with (T_CLK_START_MAX + 2 ms)
    app_init_reset_switch();
}

void app_init_reset_switch(void)
{
    HAL_GPIO_WritePin(STM_ETH_SW_RESET_N_GPIO_Port, STM_ETH_SW_RESET_N_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);  //TODO: replace with appropriate delay between switching reset line
    HAL_GPIO_WritePin(STM_ETH_SW_RESET_N_GPIO_Port, STM_ETH_SW_RESET_N_Pin, GPIO_PIN_SET); //set nreset to Hi-Z
    HAL_Delay(280);
}
/* USER CODE END */
