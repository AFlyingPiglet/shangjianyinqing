// app_globals.h
#ifndef ONECAR_APP_GLOBALS_H
#define ONECAR_APP_GLOBALS_H

#include <stdint.h>

/* 全局共享变量：跨文件用 */
extern uint8_t g_ps2_ok;    // PS2 手柄在线标志，0=失联 1=在线

#endif