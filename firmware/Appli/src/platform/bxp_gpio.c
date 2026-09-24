/**
 * @file bxp_gpio.c
 * @brief STM32N6 GPIO configuration and EXTI ISR routing implementation.
 */

#include "bxp_gpio.h"
#include "board_pins.h"
#include "sfp_gpio.h"
#include "lan96455_irq.h"
#include "stm32n6xx_hal.h"

void bxp_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO port clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

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
    GPIO_InitStruct.Pin = PIN_SFP_MOD_ABS_PIN | PIN_SFP_TX_FAULT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(PIN_SFP_MOD_ABS_PORT, &GPIO_InitStruct);

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
        sfp_gpio_los_isr_callback();
    }
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == PIN_SFP_LOS_PIN)
    {
        sfp_gpio_los_isr_callback();
    }
    else if (GPIO_Pin == PIN_SWITCH_IRQ_N_PIN)
    {
        lan96455_irq_handler_callback();
    }
}
