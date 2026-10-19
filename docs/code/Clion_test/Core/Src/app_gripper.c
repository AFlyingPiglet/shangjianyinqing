//
// Created by 23943 on 2026/10/11.
//
// app_gripper.c
#include "app_gripper.h"
#include "tim.h"

#define GRIP_OPEN_DEG   30
#define GRIP_CLOSE_DEG  110

void Gripper_Init(void) {
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    Gripper_Open();
}
void Gripper_Set(uint8_t deg) {
    if (deg > 180) deg = 180;
    uint32_t us = 500 + (2000 * deg) / 180;
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, us);
}
void Gripper_Open(void)  { Gripper_Set(GRIP_OPEN_DEG); }
void Gripper_Close(void) { Gripper_Set(GRIP_CLOSE_DEG); }