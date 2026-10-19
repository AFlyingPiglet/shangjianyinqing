//
// Created by 23943 on 2026/10/11.
//
// app_ps2.c  —— 软件 SPI：CS=PB12, CLK=PB13, DATA=PB14(MISO), CMD=PB15(MOSI)
#include "app_ps2.h"
#include "main.h"

#define PS2_CS_L()  HAL_GPIO_WritePin(PS2_CS_GPIO_Port, PS2_CS_Pin, GPIO_PIN_RESET)
#define PS2_CS_H()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)
#define PS2_CLK_L() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET)
#define PS2_CLK_H() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET)
#define PS2_CMD_L() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET)
#define PS2_CMD_H() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET)
#define PS2_DAT_R() HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14)

static uint8_t spi_xfer(uint8_t tx) {
    uint8_t rx = 0;
    for (int i = 0; i < 8; i++) {
        (tx & (1 << i)) ? PS2_CMD_H() : PS2_CMD_L();
        PS2_CLK_L();
        __NOP(); __NOP();
        rx |= (PS2_DAT_R() ? 1 : 0) << i;
        PS2_CLK_H();
        __NOP(); __NOP();
    }
    return rx;
}

void PS2_Init(void) {
    PS2_CS_H(); PS2_CLK_H(); PS2_CMD_H();
}

static uint8_t ps2_poll(uint8_t *buf9) {
    PS2_CS_L();
    spi_xfer(0x01); spi_xfer(0x42); spi_xfer(0x00);
    buf9[3] = spi_xfer(0x00);
    buf9[4] = spi_xfer(0x00);
    buf9[5] = spi_xfer(0x00);
    buf9[6] = spi_xfer(0x00);
    buf9[7] = spi_xfer(0x00);
    buf9[8] = spi_xfer(0x00);
    PS2_CS_H();

    if (buf9[3] != 0x5A) return 0;   // 无手柄 / 未对频
    return 1;
}

uint8_t PS2_Read(PS2_Data_t *o) {
    uint8_t b[9] = {0};
    if (b[3] != 0x5A) return 0;
    if (!ps2_poll(b)) return 0;

    o->select   = !(b[3] & 0x01);
    o->start    = !(b[3] & 0x08);
    o->up       = !(b[3] & 0x10);
    o->right    = !(b[3] & 0x20);
    o->down     = !(b[3] & 0x40);
    o->left     = !(b[3] & 0x80);

    o->l2 = !(b[4] & 0x01); o->r2 = !(b[4] & 0x02);
    o->l1 = !(b[4] & 0x04); o->r1 = !(b[4] & 0x08);
    o->triangle = !(b[4] & 0x10); o->circle = !(b[4] & 0x20);
    o->cross    = !(b[4] & 0x40); o->square  = !(b[4] & 0x80);

    /* 右摇杆 */
    o->rx = (int16_t)b[5] - 128;
    o->ry = (int16_t)b[6] - 128;
    /* 左摇杆 */
    o->ly = (int16_t)b[7] - 128;
    o->lx = (int16_t)b[8] - 128;

    /* 归一化到 -100~100，去掉中心死区 */
    o->lx = o->lx * 100 / 127; o->ly = o->ly * 100 / 127;
    o->rx = o->rx * 100 / 127; o->ry = o->ry * 100 / 127;
    return 1;
}