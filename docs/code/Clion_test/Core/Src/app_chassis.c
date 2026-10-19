//
// Created by 23943 on 2026/10/11.
//
#include "app_chassis.h"
#include "main.h"
#include "gpio.h"
#include "tim.h"

#define PWM_ARR     3599U      // 20kHz
#define DEAD_ZONE   4

static int16_t s_left  = 0;
static int16_t s_right = 0;

static void set_side(uint8_t side, int16_t pct)
{
    if (pct >  100) pct =  100;
    if (pct < -100) pct = -100;

    uint32_t duty = (uint32_t)((pct >= 0 ? pct : -pct) * PWM_ARR / 100);

    if (side == 0) {
        HAL_GPIO_WritePin(DIR_L_GPIO_Port, DIR_L_Pin, pct >= 0 ? GPIO_PIN_RESET : GPIO_PIN_SET);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, duty);
    } else {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, duty);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, duty);
    }
    /* 若用方向脚，在这里补 GPIO 写 */
}

void Chassis_Init(void)
{
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
    Chassis_Stop();
}

void Chassis_SetLeft (int16_t left)  { s_left  = left;  set_side(0, left);  }
void Chassis_SetRight(int16_t right) { s_right = right; set_side(1, right); }
void Chassis_SetLR   (int16_t left, int16_t right)
{
    Chassis_SetLeft(left);    // 复用，避免两处写通道
    Chassis_SetRight(right);
}

void Chassis_Drive(int16_t fwd, int16_t turn)
{
    if (fwd  > -DEAD_ZONE && fwd  < DEAD_ZONE) fwd  = 0;
    if (turn > -DEAD_ZONE && turn < DEAD_ZONE) turn = 0;
    Chassis_SetLR(fwd + turn, fwd - turn);
}

int16_t Chassis_GetLeft (void) { return s_left;  }
int16_t Chassis_GetRight(void) { return s_right; }
void Chassis_GetLR(int16_t *left, int16_t *right)
{
    if (left)  *left  = s_left;
    if (right) *right = s_right;
}

void Chassis_Stop(void)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
    s_left = s_right = 0;
}