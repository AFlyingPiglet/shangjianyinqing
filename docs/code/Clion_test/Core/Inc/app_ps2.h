//
// Created by 23943 on 2026/10/11.
//

#ifndef CLION_TEST_APP_PS2_H
#define CLION_TEST_APP_PS2_H

#endif //CLION_TEST_APP_PS2_H
// app_ps2.h
#ifndef APP_PS2_H
#define APP_PS2_H
#include <stdint.h>
typedef struct {
    int16_t lx, ly, rx, ry;  // -100~100
    uint8_t up, down, left, right, triangle, circle, cross, square;
    uint8_t l1, l2, r1, r2, select, start;
} PS2_Data_t;

void PS2_Init(void);
uint8_t PS2_Read(PS2_Data_t *out); // 返回 1 成功
#endif