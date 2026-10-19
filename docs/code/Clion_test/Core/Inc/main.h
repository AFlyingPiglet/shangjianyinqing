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

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum {
  CAR_MODE_IDLE = 0,
  CAR_MODE_MANUAL,
  CAR_MODE_TEST
} CarMode_t;

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
uint32_t GetTick_ms(void);

void Car_SafeStop(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LIFT_PWM_Pin GPIO_PIN_0
#define LIFT_PWM_GPIO_Port GPIOA
#define DIR_L_Pin GPIO_PIN_0
#define DIR_L_GPIO_Port GPIOB
#define DIR_R_Pin GPIO_PIN_1
#define DIR_R_GPIO_Port GPIOB
#define PS2_CS_Pin GPIO_PIN_12
#define PS2_CS_GPIO_Port GPIOB
#define PS2_CLK_Pin GPIO_PIN_13
#define PS2_CLK_GPIO_Port GPIOB
#define PS2_DATA_Pin GPIO_PIN_14
#define PS2_DATA_GPIO_Port GPIOB
#define PS2_CMD_Pin GPIO_PIN_15
#define PS2_CMD_GPIO_Port GPIOB
#define MOTOR_LF_Pin GPIO_PIN_8
#define MOTOR_LF_GPIO_Port GPIOA
#define MOTOR_RF_Pin GPIO_PIN_9
#define MOTOR_RF_GPIO_Port GPIOA
#define MOTOR_LB_Pin GPIO_PIN_10
#define MOTOR_LB_GPIO_Port GPIOA
#define MOTOR_RB_Pin GPIO_PIN_11
#define MOTOR_RB_GPIO_Port GPIOA
#define RELAY_EN_Pin GPIO_PIN_5
#define RELAY_EN_GPIO_Port GPIOB
#define LIMIT_BOT_Pin GPIO_PIN_6
#define LIMIT_BOT_GPIO_Port GPIOB
#define LIMIT_TOP_Pin GPIO_PIN_7
#define LIMIT_TOP_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
