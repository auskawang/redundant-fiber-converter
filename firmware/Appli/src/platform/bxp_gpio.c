/**
 * @file bxp_gpio.c
 * @brief STM32N6 GPIO configuration and EXTI ISR routing implementation.
 */

#include "bxp_gpio.h"
#include "board_pins.h"
#include "sfp_gpio.h"
#include "lan96455_irq.h"
#include "stm32n6xx_hal.h"

static volatile unsigned los_pending, switch_pending;
void bxp_gpio_process(void) {
 uint32_t mask=__get_PRIMASK();
 __disable_irq();
 unsigned los=los_pending, sw=switch_pending;
 los_pending=0; switch_pending=0;
 __set_PRIMASK(mask);
 if (los) sfp_gpio_los_isr_callback();
 if (sw) lan96455_irq_handler_callback();
}
bool bxp_gpio_is_ready(void) { return BXP_BOARD_PINS_CONFIRMED != 0; }
void bxp_gpio_init(void)
{
    if (!BXP_BOARD_PINS_CONFIRMED) return;
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO port clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    HAL_GPIO_WritePin(PIN_SWITCH_CS_N_PORT, PIN_SWITCH_CS_N_PIN, GPIO_PIN_SET);
    GPIO_InitStruct.Pin = PIN_SWITCH_CS_N_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(PIN_SWITCH_CS_N_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(PIN_SFP_TX_DISABLE_PORT, PIN_SFP_TX_DISABLE_PIN, GPIO_PIN_SET);

    HAL_GPIO_WritePin(PIN_POWER_SEQ_EN_PORT, PIN_POWER_SEQ_EN_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PIN_SWITCH_RESET_N_PORT, PIN_SWITCH_RESET_N_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PIN_PHY_RESET_N_PORT, PIN_PHY_RESET_N_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PIN_PEER_SYNC_OUT_PORT, PIN_PEER_SYNC_OUT_PIN, GPIO_PIN_RESET);

    /* 1. Power sequencing & reset outputs (Active HIGH/LOW) */
    GPIO_InitStruct.Pin = PIN_POWER_SEQ_EN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(PIN_POWER_SEQ_EN_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = PIN_SWITCH_RESET_N_PIN;
    HAL_GPIO_Init(PIN_SWITCH_RESET_N_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = PIN_PHY_RESET_N_PIN;
    HAL_GPIO_Init(PIN_PHY_RESET_N_PORT, &GPIO_InitStruct);

    /* 2. SFP Control Outputs */
    GPIO_InitStruct.Pin = PIN_SFP_TX_DISABLE_PIN;
    HAL_GPIO_Init(PIN_SFP_TX_DISABLE_PORT, &GPIO_InitStruct);

    /* 3. SFP Status Inputs (MOD_ABS, TX_FAULT) */
    GPIO_InitStruct.Pin = PIN_SFP_MOD_ABS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(PIN_SFP_MOD_ABS_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = PIN_SFP_TX_FAULT_PIN;
    HAL_GPIO_Init(PIN_SFP_TX_FAULT_PORT, &GPIO_InitStruct);

    /* 4. SFP LOS Interrupt (EXTI on rising and falling edge) */
    GPIO_InitStruct.Pin = PIN_SFP_LOS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(PIN_SFP_LOS_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(PIN_SFP_LOS_EXTI_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(PIN_SFP_LOS_EXTI_IRQn);

    /* 5. Switch Interrupt (EXTI falling edge) */
    GPIO_InitStruct.Pin = PIN_SWITCH_IRQ_N_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(PIN_SWITCH_IRQ_N_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(PIN_SWITCH_IRQ_EXTI_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(PIN_SWITCH_IRQ_EXTI_IRQn);

    /* 6. Peer Sync In/Out */
    GPIO_InitStruct.Pin = PIN_PEER_SYNC_OUT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(PIN_PEER_SYNC_OUT_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = PIN_PEER_SYNC_IN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(PIN_PEER_SYNC_IN_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(PIN_PEER_SYNC_EXTI_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(PIN_PEER_SYNC_EXTI_IRQn);
}

/**
 * @brief EXTI line detection callback from HAL.
 */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == PIN_SFP_LOS_PIN)
    {
        los_pending=1;
    }
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == PIN_SFP_LOS_PIN)
    {
        los_pending=1;
    }
    else if (GPIO_Pin == PIN_SWITCH_IRQ_N_PIN)
    {
        switch_pending=1;
    }
}

void bxp_gpio_peer_toggle(void) { if (bxp_gpio_is_ready()) HAL_GPIO_TogglePin(PIN_PEER_SYNC_OUT_PORT,PIN_PEER_SYNC_OUT_PIN); }
void EXTI3_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(PIN_SFP_LOS_PIN); }
void EXTI0_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(PIN_SWITCH_IRQ_N_PIN); }
void EXTI9_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(PIN_PEER_SYNC_IN_PIN); }
