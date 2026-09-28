/**
 * @file board_pins.h
 * @brief Board-level pin definitions, GPIO ports, and hardware interface mappings.
 */

#ifndef BOARD_PINS_H
#define BOARD_PINS_H

#include <stdint.h>
/* Existing repository mappings: enable only after schematic review. */
#ifndef BXP_BOARD_PINS_CONFIRMED
#define BXP_BOARD_PINS_CONFIRMED 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Power Sequencing & Reset Pins */
#define PIN_POWER_SEQ_EN_PORT       GPIOD
#define PIN_POWER_SEQ_EN_PIN        GPIO_PIN_0

#define PIN_SWITCH_RESET_N_PORT     GPIOD
#define PIN_SWITCH_RESET_N_PIN      GPIO_PIN_1

#define PIN_PHY_RESET_N_PORT        GPIOD
#define PIN_PHY_RESET_N_PIN         GPIO_PIN_2

/* SFP Optical Control & Status Pins */
#define PIN_SFP_TX_DISABLE_PORT     GPIOE
#define PIN_SFP_TX_DISABLE_PIN      GPIO_PIN_2

#define PIN_SFP_LOS_PORT            GPIOE
#define PIN_SFP_LOS_PIN             GPIO_PIN_3
#define PIN_SFP_LOS_EXTI_IRQn       EXTI3_IRQn

#define PIN_SFP_MOD_ABS_PORT        GPIOE
#define PIN_SFP_MOD_ABS_PIN         GPIO_PIN_4

#define PIN_SFP_TX_FAULT_PORT       GPIOE
#define PIN_SFP_TX_FAULT_PIN        GPIO_PIN_5

/* SFP I2C Interface (DDM & EEPROM access) */
#define SFP_I2C_INSTANCE            I2C1
#define PIN_SFP_I2C_SCL_PORT        GPIOB
#define PIN_SFP_I2C_SCL_PIN         GPIO_PIN_6
#define PIN_SFP_I2C_SDA_PORT        GPIOB
#define PIN_SFP_I2C_SDA_PIN         GPIO_PIN_7

/* LAN9646SF / LAN96455 SPI Control Pins */
#define SWITCH_SPI_INSTANCE         SPI1
#define PIN_SWITCH_CS_N_PORT        GPIOA
#define PIN_SWITCH_CS_N_PIN         GPIO_PIN_4
#define PIN_SWITCH_SCK_PORT         GPIOA
#define PIN_SWITCH_SCK_PIN          GPIO_PIN_5
#define PIN_SWITCH_MISO_PORT        GPIOA
#define PIN_SWITCH_MISO_PIN         GPIO_PIN_6
#define PIN_SWITCH_MOSI_PORT        GPIOA
#define PIN_SWITCH_MOSI_PIN         GPIO_PIN_7

#define PIN_SWITCH_IRQ_N_PORT       GPIOB
#define PIN_SWITCH_IRQ_N_PIN        GPIO_PIN_0
#define PIN_SWITCH_IRQ_EXTI_IRQn    EXTI0_IRQn

/* Inter-STM32 Peer Link (Direct Sync / Heartbeat) */
#define PIN_PEER_SYNC_OUT_PORT      GPIOC
#define PIN_PEER_SYNC_OUT_PIN       GPIO_PIN_8

#define PIN_PEER_SYNC_IN_PORT       GPIOC
#define PIN_PEER_SYNC_IN_PIN        GPIO_PIN_9
#define PIN_PEER_SYNC_EXTI_IRQn     EXTI9_IRQn

#ifdef __cplusplus
}
#endif

#endif /* BOARD_PINS_H */
