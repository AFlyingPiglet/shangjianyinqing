//
// Created by 23943 on 2026/10/11.
//
// app_power.c
#include "app_power.h"
#include "main.h"
#include "app_globals.h"

/* 继电器信号脚（若需要 MCU 参与使能）PB5；纯硬件方案可删 */
#define RELAY_EN_H() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET)
#define RELAY_EN_L() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET)

static uint32_t s_last_ok = 0;
static uint8_t  s_lost    = 0;

void Power_Init(void) {
    RELAY_EN_H();          // 上电默认允许供电
    s_last_ok = HAL_GetTick();
}

void Power_Task(void)
{
    if (g_ps2_ok) {                      // ← 用
        s_last_ok = HAL_GetTick();
        s_lost = 0;
    } else if (HAL_GetTick() - s_last_ok > 500) {
        s_lost = 1;
    }
}

uint8_t Power_IsLost(void) { return s_lost; }