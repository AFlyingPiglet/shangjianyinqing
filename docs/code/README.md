# 代码总目录（CRTC2026 电控）

> 本目录集中存放电控相关代码与工程配置。主工程为一号车电控（Clion_test），其中 `app_*.c` 按功能模块拆分，便于对应到 `docs/1.x` 各功能文档。

## 目录

- `Clion_test/`：一号车电控主工程（STM32F103C8T6，CLion + CMake + STM32CubeMX，HAL 库）
  - `Core/Src/`、`Core/Inc/`：应用代码与 CubeMX 生成代码
  - `Clion_test.ioc`：CubeMX 配置（引脚/时钟/外设）
  - `CMakeLists.txt`、`cmake/`：构建配置
- `F401-电机测试-最小版.c`：单电机正反转测试（F401 测试板）

## 模块对照

| 模块 | 文件 | 作用 | 对应功能文档 |
|---|---|---|---|
| 手柄输入 | `Clion_test/Core/Src/app_ps2.c` | 软件 SPI 读 PS2 手柄（CS/CLK/DATA/CMD = PB12~PB15），解析按键与摇杆（0x5A 校验） | 1.6 / 1.7 |
| 底盘驱动 | `Clion_test/Core/Src/app_chassis.c` | TIM1 四路 PWM 差速混控（左右分组、死区 4%、20 kHz）、停止与占空比读取 | 1.3 / 1.6 / 1.7 |
| 机械臂舵机 | `Clion_test/Core/Src/app_arm.c` | TIM3 CH1 俯仰 MG995、CH2 夹爪 MG90S；肘部为插销机械三档 | 1.4 / 1.5 |
| 夹爪 | `Clion_test/Core/Src/app_gripper.c` | MG90S 开合（30° / 110°） | 1.4 / 1.5 / 1.9 / 2.2 |
| 举升 | `Clion_test/Core/Src/app_lift.c` | TIM2 N20 PWM（20 kHz）、上下限位（PB6/PB7）、高度估算与死区控制（1 ms 节拍） | 1.2 / 1.9 |
| 电源保护 | `Clion_test/Core/Src/app_power.c` | 继电器使能（PB5）；手柄信号丢失 >500 ms 判定断电保护 | 安全相关（1.6） |
| 主循环 | `Clion_test/Core/Src/main.c` | CubeMX 初始化 + App_Init / App_Loop（手柄 → 底盘/机械臂/举升/夹爪，1 ms 循环） | 总入口 |
| 全局状态 | `Clion_test/Core/Src/app_globals.c` | `g_ps2_ok` 手柄在线标志 | 主循环 |

## 构建

- MCU：STM32F103C8T6（一号车电控板）；工具链：CLion + STM32CubeCLT（arm-none-eabi-gcc / CMake / Ninja）。
- 打开 `Clion_test/` 工程，CMake 配置 `Debug-STM32`，工具链文件 `cmake/gcc-arm-none-eabi.cmake`。
- 引脚/时钟以外设配置为准：`Clion_test.ioc`。

## 备注

- 代码为 2026-10 版本快照；部分参数（如举升高度常量）需按现场实测标定。
- 二号车电控代码暂未纳入本目录。
