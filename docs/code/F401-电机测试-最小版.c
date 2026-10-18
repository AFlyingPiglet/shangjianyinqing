/* ============================================================
 * F401 电机测试 · 最小版（单独截取，可直接当 main.c 用）
 *
 * 行为：电机正转 2 秒 → 停 1 秒 → 反转 2 秒 → 停 1 秒，循环
 *
 * 接线：
 *   A6 (PA6)  → DRV8833 AIN1
 *   A7 (PA7)  → DRV8833 AIN2
 *   3V3       → DRV8833 STBY（必须为高）
 *   VM → 电池正，GND → 电池负（电池负与板子 G 共地！）
 *   AO1 / AO2 → 电机两根线
 *
 * 原理：HSI 16MHz；TIM3 输出 20kHz PWM（PA6=CH1、PA7=CH2）。
 *   正转 = CH1 给占空比、CH2=0；反转相反；停止 = 两路都 0。
 * ============================================================ */
#include "main.h"

static TIM_HandleTypeDef htim3;

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* TIM3：PA6=CH1、PA7=CH2，20kHz PWM（16MHz / 800） */
static void MotorPWM_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  __HAL_RCC_TIM3_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin       = GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  htim3.Instance               = TIM3;
  htim3.Init.Prescaler         = 0;
  htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim3.Init.Period            = 799;         /* 16MHz/800 = 20kHz */
  htim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode     = TIM_OCMODE_PWM1;
  sConfigOC.Pulse      = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* speed: -100 ~ +100。正转=CH1 有占空比、CH2=0；反转相反；0=停 */
static void Motor_Set(int16_t speed)
{
  uint32_t duty;

  if (speed >  100) speed =  100;
  if (speed < -100) speed = -100;
  duty = (uint32_t)((speed >= 0 ? speed : -speed) * 800 / 100);

  if (speed > 0)
  {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, duty);
  }
  else if (speed < 0)
  {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, duty);
  }
  else
  {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
  }
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MotorPWM_Init();
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

  while (1)
  {
    Motor_Set(80);   HAL_Delay(2000);   /* 正转 2 秒 */
    Motor_Set(0);    HAL_Delay(1000);   /* 停 1 秒 */
    Motor_Set(-80);  HAL_Delay(2000);   /* 反转 2 秒 */
    Motor_Set(0);    HAL_Delay(1000);   /* 停 1 秒 */
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
