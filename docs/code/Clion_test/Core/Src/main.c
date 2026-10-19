/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* USER CODE END Header *//*Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>              /* printf 重定向需要 */
#include <string.h>
#include "app_chassis.h"        /* 差速底盘：Chassis_Init/Drive/Stop/GetLR */
#include "app_ps2.h"            /* PS2 手柄：PS2_Init/PS2_Read */
#include "app_lift.h"           /* 举升丝杆：Lift_Init/SetHeight/Update/GetHeight */
#include "app_arm.h"            /* 俯仰舵机：Arm_Init/SetMode/SetPitch */
#include "app_gripper.h"        /* 夹爪舵机：Gripper_Init/Open/Close/Set */
#include "app_power.h"          /* 无线继电器 + 失联保护：Power_Init/Task/IsLost */
#include "app_globals.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* 主循环节拍：1ms */
#define LOOP_PERIOD_MS      1U

/* 手柄失联判定阈值：连续 500ms 读不到手柄，就停车 */
#define PS2_LOST_TIMEOUT_MS 500U

/* 举升手动微调步长（每按一次三角/叉，目标高度 ±3mm） */
#define LIFT_STEP_MM        3

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static PS2_Data_t g_ps2;                        /* 当前一帧手柄数据 */
static uint32_t   g_ps2_last_ok   = 0;          /* 上一次成功读到手柄的时刻 */
static uint8_t    g_ps2_lost      = 0;          /* 手柄是否已判定失联 */
static uint32_t   g_tick          = 0;          /* 主循环 1ms 节拍计时 */
static uint8_t    g_pitch_deg     = 90;         /* 俯仰舵机当前角度（50~140 之间微调） */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void App_Init(void);                     /* 各子系统初始化 */
static void App_Loop(void);                     /* 1ms 控制循环 */
static void App_HandlePS2(void);                /* PS2 按键 → 各子系统指令 */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  各子系统初始化：底盘、举升、舵机、夹爪、PS2、电源
  * @note   在 MX_xxx_Init() 全部调用之后调用
  */
static void App_Init(void)
{
    Chassis_Init();      /* 启动 TIM1 四路 PWM，占空比清零 */
    Lift_Init();         /* 启动 TIM2 一路 PWM，占空比清零 */
    Arm_Init();          /* 启动 TIM3 两路 PWM，俯仰中位、夹爪全开 */
    Gripper_Init();      /* 与 Arm_Init 共用 TIM3，这里确保夹爪初始化 */
    PS2_Init();          /* 软件 SPI 空闲电平：CS=高、CLK=低、CMD=高 */
    Power_Init();        /* 无线继电器使能，记录上电时刻 */

    /* 全局 1ms 节拍基准 */
    g_tick        = HAL_GetTick();
    g_ps2_last_ok = HAL_GetTick();

    printf("[OneCar] App_Init done\r\n");
}

/**
  * @brief  把 PS2 一帧数据翻译成各子系统的动作
  * @note   约定：
  *         左摇杆 上下 -> 前进/后退（ly）
  *         右摇杆 左右 -> 原地转向（rx）
  *         三角   -> 举升 +3mm
  *         叉     -> 举升 -3mm
  *         方     -> 夹爪开
  *         圆     -> 夹爪闭
就                                                                                                                                                                                                                                                    *         L1/R1  -> 俯仰角 -/+ 1°
  */
static void App_HandlePS2(void)
{
    /* ---------- 底盘：左摇杆 ly 控制前后，右摇杆 rx 控制转向 ---------- */
    /* 注意：PS2 的 ly 上推为负值，所以取负号让"上推 = 前进" */
    Chassis_Drive((int16_t)(-g_ps2.ly), (int16_t)(g_ps2.rx));

    /* ---------- 举升：三角/叉 微调目标高度 ---------- */
    if (g_ps2.triangle) {
        Lift_SetHeight((uint16_t)(Lift_GetHeight() + LIFT_STEP_MM));
    }
    if (g_ps2.cross) {
        int16_t next = (int16_t)Lift_GetHeight() - LIFT_STEP_MM;
        Lift_SetHeight((uint16_t)(next < 0 ? 0 : next));  /* 防下溢 */
    }

    /* ---------- 夹爪：方开、圆闭 ---------- */
    if (g_ps2.square) Gripper_Open();
    if (g_ps2.circle) Gripper_Close();

    /* ---------- 俯仰：L1 减、R1 加，限制在 50~140° ---------- */
    if (g_ps2.l1 && g_pitch_deg > 50)  g_pitch_deg--;
    if (g_ps2.r1 && g_pitch_deg < 140) g_pitch_deg++;
    Arm_SetPitch(g_pitch_deg);
}

/**
  * @brief  1ms 控制循环：读手柄 → 分发指令 → 子系统更新 → 失联保护
  */
static void App_Loop(void)
{
    /* ---- 1ms 节拍控制 ---- */
    if ((HAL_GetTick() - g_tick) < LOOP_PERIOD_MS) return;
    g_tick = HAL_GetTick();

    /* ---- 读手柄 ---- */
    g_ps2_ok = PS2_Read(&g_ps2);
    if (g_ps2_ok) {
        g_ps2_last_ok = HAL_GetTick();
        g_ps2_lost    = 0;
    } else if ((HAL_GetTick() - g_ps2_last_ok) > PS2_LOST_TIMEOUT_MS) {
        g_ps2_lost    = 1;
    }

    /* ---- 分发 ---- */
    if (g_ps2_ok && !g_ps2_lost && !Power_IsLost()) {
        App_HandlePS2();
    } else {
        /* 失联或低压：只停车，不断电（断电由无线继电器负责） */
        Chassis_Stop();
    }

    /* ---- 子系统后台任务 ---- */
    Lift_Update();       /* 限位检测 + 高度估算 */
    Power_Task();        /* 手柄失联判定 + 低压保护（软逻辑） */
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* USER CODE BEGIN 1 */
    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    /* 加这三行，上电立刻拉高 SWD 引脚，防止被误配置成普通 IO */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USER CODE BEGIN Init */
    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */
    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_TIM1_Init();      /* 底盘 4 路 PWM */
    MX_TIM2_Init();      /* 举升 1 路 PWM */
    MX_TIM3_Init();      /* 舵机 2 路 PWM */
    MX_USART2_UART_Init();

    /* USER CODE BEGIN 2 */
    App_Init();          /* 各子系统初始化 */
    printf("[OneCar] boot ok, build %s %s\r\n", __DATE__, __TIME__);
    Lift_Init();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        App_Loop();      /* 1ms 控制循环 */
        /* USER CODE END 3 */
    }
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
}
/* USER CODE END 3 */

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
      * in the RCC_OscInitTypeDef structure.
      */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState       = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL     = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
      */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK   |
                                        RCC_CLOCKTYPE_SYSCLK |
                                        RCC_CLOCKTYPE_PCLK1  |
                                        RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */


int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 10);
    return ch;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* 出错了，全车停，然后死循环 */
    Chassis_Stop();
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    printf("[ASSERT] %s:%lu\r\n", file, line);
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */