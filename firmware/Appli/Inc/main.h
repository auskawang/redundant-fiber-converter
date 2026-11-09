/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined ( __ICCARM__ )
#  define CMSE_NS_CALL  __cmse_nonsecure_call
#  define CMSE_NS_ENTRY __cmse_nonsecure_entry
#else
#  define CMSE_NS_CALL  __attribute((cmse_nonsecure_call))
#  define CMSE_NS_ENTRY __attribute((cmse_nonsecure_entry))
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32n6xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* Function pointer declaration in non-secure*/
#if defined ( __ICCARM__ )
typedef void (CMSE_NS_CALL *funcptr)(void);
#else
typedef void CMSE_NS_CALL (*funcptr)(void);
#endif

/* typedef for non-secure callback functions */
typedef funcptr funcptr_NS;

/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SFP0_TX_FAULT_Pin GPIO_PIN_1
#define SFP0_TX_FAULT_GPIO_Port GPIOP
#define SFP0_MOD_DEF0_Pin GPIO_PIN_0
#define SFP0_MOD_DEF0_GPIO_Port GPIOP
#define SFP0_LOS_Pin GPIO_PIN_2
#define SFP0_LOS_GPIO_Port GPIOP
#define STM_SFP0_LS_EN_Pin GPIO_PIN_3
#define STM_SFP0_LS_EN_GPIO_Port GPIOP
#define STM_SFP1_LS_FLT_Pin GPIO_PIN_15
#define STM_SFP1_LS_FLT_GPIO_Port GPIOP
#define SFP1_TX_FAULT_Pin GPIO_PIN_14
#define SFP1_TX_FAULT_GPIO_Port GPIOP
#define SFP1_MOD_DEF0_Pin GPIO_PIN_12
#define SFP1_MOD_DEF0_GPIO_Port GPIOP
#define SFP1_LOS_Pin GPIO_PIN_11
#define SFP1_LOS_GPIO_Port GPION
#define STM_SFP1_LS_EN_Pin GPIO_PIN_10
#define STM_SFP1_LS_EN_GPIO_Port GPION
#define STM_SFP0_LS_FLT_Pin GPIO_PIN_0
#define STM_SFP0_LS_FLT_GPIO_Port GPION
#define SFP1_TX_DISABLE_Pin GPIO_PIN_9
#define SFP1_TX_DISABLE_GPIO_Port GPION
#define SFP1_TX_FAULT_LED_Pin GPIO_PIN_7
#define SFP1_TX_FAULT_LED_GPIO_Port GPIOA
#define SFP1_MOD_DEF0_LED_Pin GPIO_PIN_9
#define SFP1_MOD_DEF0_LED_GPIO_Port GPIOG
#define SFP0_TX_DISABLE_Pin GPIO_PIN_11
#define SFP0_TX_DISABLE_GPIO_Port GPIOG
#define SFP0_TX_FAULT_LED_Pin GPIO_PIN_4
#define SFP0_TX_FAULT_LED_GPIO_Port GPIOA
#define SFP0_MOD_DEF0_LED_Pin GPIO_PIN_12
#define SFP0_MOD_DEF0_LED_GPIO_Port GPION

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
