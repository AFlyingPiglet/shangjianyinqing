//
// Created by 23943 on 2026/10/11.
//

#ifndef CLION_TEST_APP_POWER_H
#define CLION_TEST_APP_POWER_H
// app_power.h
#ifndef APP_POWER_H
#define APP_POWER_H
#include <stdint.h>
void Power_Init(void);
void Power_Task(void);            // 主循环调用：看门狗 / 低压保护
uint8_t Power_IsLost(void);       // 手柄失联
#endif
#endif //CLION_TEST_APP_POWER_H
