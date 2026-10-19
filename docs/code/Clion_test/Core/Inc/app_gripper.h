//
// Created by 23943 on 2026/10/11.
//

#ifndef ONECAR_APP_GRIPPER_H
#define ONECAR_APP_GRIPPER_H

#include <stdint.h>

/* 一号车 · 夹爪（MG90S）驱动接口 */
void Gripper_Init (void);           // 启动 PWM
void Gripper_Open (void);           // 全开
void Gripper_Close(void);           // 全闭
void Gripper_Set  (uint8_t deg);    // 0=全开，180=全闭，按实物标定

#endif /* ONECAR_APP_GRIPPER_H */