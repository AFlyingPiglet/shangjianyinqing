#include <stdint.h>
//
// Created by 23943 on 2026/10/11.
//
#ifndef CLION_TEST_APP_ARM_H
#define CLION_TEST_APP_ARM_H
#ifndef APP_ARM_H
#define APP_ARM_H
typedef enum { ARM_ELBOW_180 = 0, ARM_ELBOW_135, ARM_ELBOW_90 } ArmMode_t;
void Arm_Init(void);
void Arm_SetMode(ArmMode_t m);   // 180° / 135° / 90°
void Arm_SetPitch(uint8_t deg);  // MG995 俯仰 0~180
#endif
#endif //CLION_TEST_APP_ARM_H
