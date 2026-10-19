//
// Created by 23943 on 2026/10/11.
//

#ifndef CLION_TEST_APP_CHASSIS_H
#define CLION_TEST_APP_CHASSIS_H

#include <stdint.h>

void Chassis_Init(void);
void Chassis_SetLeft(int16_t left);
void Chassis_SetRight(int16_t right);
void Chassis_SetLR(int16_t left,int16_t right);

void Chassis_Drive(int16_t fwd,int16_t rev);

int16_t Chassis_GetLeft(void);
int16_t Chassis_GetRight(void);
void Chassis_GetLR(int16_t *left, int16_t *right);

void Chassis_Stop(void);

#endif //CLION_TEST_APP_CHASSIS_H