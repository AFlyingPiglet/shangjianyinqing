//
// Created by 23943 on 2026/10/11.
//
// app_arm.c
#include "app_arm.h"
#include "tim.h"

/* 舵机 50Hz：PSC=71, ARR=19999 -> 20ms。脉宽 500~2500us 对应 0~180° */
#define SERVO_MIN 500
#define SERVO_MAX 2500

static void servo_set(uint32_t ch, uint8_t deg) {
    if (deg > 180) deg = 180;
    uint32_t us = SERVO_MIN + (SERVO_MAX - SERVO_MIN) * deg / 180;
    __HAL_TIM_SET_COMPARE(&htim3, ch, us);  // 注意：ARR 设为 20000-1 时，compare 单位就是 us
}

void Arm_Init(void) {
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); // 俯仰 MG995
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); // 夹爪 MG90S
    Arm_SetMode(ARM_ELBOW_180);
    Arm_SetPitch(90);
}

void Arm_SetMode(ArmMode_t m) {
    /* 一号车肘关节是"插销定位"机械三档（08 号图：180/135/90），
       电控只需要控制俯仰舵机；若后续改成舵机驱动肘，把这里换成角度即可 */
    (void)m;
}

void Arm_SetPitch(uint8_t deg) {
    servo_set(TIM_CHANNEL_1, deg);
}