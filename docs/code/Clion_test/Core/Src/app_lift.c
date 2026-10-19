//
// Created by 23943 on 2026/10/11.
//
// app_lift.c
#include "app_lift.h"
#include "main.h"      // User Label 宏：LIMIT_BOT_Pin / LIMIT_TOP_Pin / LIFT_DIR_*
#include "gpio.h"      // HAL_GPIO_ReadPin / HAL_GPIO_WritePin / GPIO_PIN_x
#include "tim.h"       // htim2


/* 举升机械范围（mm）：来自方案书"举升 106~260" */
#define LIFT_MIN_MM        106
#define LIFT_MAX_MM        260

/* 初始高度（mm）：上电时若限位没触发，默认按这个估算 */
#define LIFT_HOME_MM       150

/* TIM2 PWM：20kHz（PSC=0, ARR=3599） */
#define LIFT_ARR           3599U

/* 占空比：0~ARR。数值越大上升越快；下降可以慢一点，避免冲过限位 */
#define LIFT_DUTY_UP       2400U
#define LIFT_DUTY_DOWN     1800U

/* 高度估算：无编码器时的"积分速度"（mm/s），实测标定后替换 */
#define LIFT_SPEED_MM_S    30.0f

/* 主循环节拍（ms）：和 main.c 里的 LOOP_PERIOD_MS 一致 */
#define LIFT_LOOP_MS       1.0f


/* 底限位：PB6，上拉，低有效（读到 RESET = 触发） */
#define LIM_BOT()  (HAL_GPIO_ReadPin(LIMIT_BOT_GPIO_Port, LIMIT_BOT_Pin) == GPIO_PIN_RESET)

/* 顶限位：PB7，上拉，低有效 */
#define LIM_TOP()   (HAL_GPIO_ReadPin(LIMIT_TOP_GPIO_Port, LIMIT_TOP_Pin) == GPIO_PIN_RESET)


static uint16_t s_target = LIFT_HOME_MM;   // 目标高度（mm）
static uint16_t s_now    = LIFT_HOME_MM;   // 估算当前高度（mm）
static int8_t   s_dir    = 0;              // -1 下降 / 0 停 / +1 上升


/* 把目标高度钳位到机械范围 */
static uint16_t lift_clamp(uint16_t mm)
{
    if (mm < LIFT_MIN_MM) return LIFT_MIN_MM;
    if (mm > LIFT_MAX_MM) return LIFT_MAX_MM;
    return mm;
}

/* 立即停 PWM（占空比 0），并把方向状态清 0 */
static void lift_stop(void)
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
    s_dir = 0;
}

/* 写 PWM 占空比 */
static void lift_set_duty(uint32_t duty)
{
    if (duty > LIFT_ARR) duty = LIFT_ARR;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);
}


/**
  * @brief  举升初始化
  * @note   启动 PWM、占空比清零、用限位校正初始高度
  */
void Lift_Init(void)
{
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);

    /* 用限位状态校正初始高度估算 */
    if (LIM_BOT()) {
        s_now    = LIFT_MIN_MM;      /* 已在底，确定位置 */
        s_target = LIFT_HOME_MM;     /* 目标回到中位 */
    } else if (LIM_TOP()) {
        s_now    = LIFT_MAX_MM;
        s_target = LIFT_HOME_MM;
    } else {
        /* 两个限位都没触发：位置未知，按中位估算（等第一次碰限位再校正） */
        s_now    = LIFT_HOME_MM;
        s_target = LIFT_HOME_MM;
    }

    s_dir = 0;
}

/**
  * @brief  设置目标高度
  * @param  mm  目标高度（mm），内部会钳位到 106~260
  */
void Lift_SetHeight(uint16_t mm)
{
    s_target = lift_clamp(mm);
}

/**
  * @brief  返回估算的当前高度（mm）
  */
uint16_t Lift_GetHeight(void)
{
    return s_now;
}

/**
  * @brief  主循环调用：限位检测 + 高度估算 + PWM 输出
  * @note   必须在 1ms 节拍里调用，LIFT_LOOP_MS 与节拍一致
  */
void Lift_Update(void)
{
    /* ---------- 1. 限位优先：到顶/到底立刻停，并校正估算值 ---------- */
    if (LIM_TOP()) {
        lift_stop();
        s_now = LIFT_MAX_MM;         /* 顶限位触发：位置确定 */
    }
    if (LIM_BOT()) {
        lift_stop();
        s_now = LIFT_MIN_MM;         /* 底限位触发：位置确定 */
    }

    /* ---------- 2. 判断方向 ---------- */
    /* 死区 ±2mm，避免目标=当前时 PWM 抖 */
    if (s_target > s_now + 2) {
        s_dir = +1;
    } else if (s_target < s_now - 2) {
        s_dir = -1;
    } else {
        s_dir = 0;
    }

    /* ---------- 3. 限位保护：往一个方向走时，碰到同方向限位就停 ---------- */
    if ((s_dir > 0) && LIM_TOP()) {
        lift_stop();
        return;
    }
    if ((s_dir < 0) && LIM_BOT()) {
        lift_stop();
        return;
    }

    /* ---------- 4. 输出 PWM ---------- */
    if (s_dir > 0) {
        lift_set_duty(LIFT_DUTY_UP);
    } else if (s_dir < 0) {
        lift_set_duty(LIFT_DUTY_DOWN);
    } else {
        lift_stop();
    }


    static uint16_t s_accum = 0;
    if (s_dir != 0) {
        s_accum++;
        if (s_accum >= 33) {                 /* 33ms ≈ 1mm */
            s_accum = 0;
            if (s_dir > 0) {
                if (s_now < LIFT_MAX_MM) s_now++;
            } else {
                if (s_now > LIFT_MIN_MM) s_now--;
            }
        }
    } else {
        s_accum = 0;                         /* 停住时清积分，避免误差累积 */
    }
}

/**
  * @brief  调试用：底限位状态
  */
uint8_t Lift_GetLimitBot(void)
{
    return LIM_BOT() ? 1 : 0;
}

/**
  * @brief  调试用：顶限位状态
  */
uint8_t Lift_GetLimitTop(void)
{
    return LIM_TOP() ? 1 : 0;
}