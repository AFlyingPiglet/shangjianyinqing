//
// Created by 23943 on 2026/10/11.
//
// app_lift.h
#ifndef ONECAR_APP_LIFT_H
#define ONECAR_APP_LIFT_H

#include <stdint.h>

void     Lift_Init(void);                 // 启动 PWM、占空比清零、读限位校正初始高度
void     Lift_SetHeight(uint16_t mm);     // 设置目标高度（自动钳位到 106~260）
uint16_t Lift_GetHeight(void);            // 返回估算的当前高度
void     Lift_Update(void);               // 主循环调用：限位检测 + 高度估算 + PWM 输出

/* 调试用：立即停 PWM，并返回当前限位状态（1=触发，0=未触发） */
uint8_t  Lift_GetLimitBot(void);
uint8_t  Lift_GetLimitTop(void);

#endif /* ONECAR_APP_LIFT_H */