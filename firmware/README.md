# STM32/FreeRTOS 接入说明

## 1. 建议工程

- MCU：STM32F407VET6 或 NUCLEO-F411RE
- IDE：STM32CubeIDE 或 Keil MDK
- 配置：HAL + FreeRTOS + CMSIS-DSP
- 调试：ST-Link + SWD + 串口

## 2. 加入本项目代码

把以下目录加入 STM32 工程：

```text
include/mg
src/core
firmware/app
firmware/bsp
```

`mg_core` 不依赖 HAL，可以直接在 STM32 上编译。`firmware/bsp/bsp.h` 是硬件抽象层，需要在 `bsp_stm32.c` 中实现以下函数：

```c
bool bsp_read_current(float *amps);
bool bsp_read_vibration(float *g);
bool bsp_read_rpm(float *rpm);
bool bsp_read_temperature(float *celsius);
void bsp_set_motor_duty(float duty);
void bsp_set_relay(bool closed);
```

## 3. CubeMX 配置

- RCC：外部晶振，系统时钟 168MHz
- USART1：115200，8N1，中断接收
- TIM1：PWM 20kHz，驱动电机
- TIM2：输入捕获，测霍尔转速
- ADC1：PA0，12 位，DMA 循环/双缓冲，TIM 触发
- SPI1：ADXL345，模式 3，约 2–5MHz
- SPI2：microSD
- I2C1：OLED
- USB FS：CDC
- IWDG：约 500ms–2s
- FreeRTOS：任务和队列使用静态创建

## 4. 任务建议

| 任务 | 优先级 | 栈 | 周期 | 说明 |
|---|---:|---:|---:|---|
| `AcqTask` | 5 | 512 words | 20kHz 触发 | ADC DMA、SPI 振动、环形缓冲 |
| `ControlTask` | 4 | 256 words | 1ms | 速度闭环、安全切断 |
| `DspTask` | 3 | 1024 words | 每帧 | Hann 窗、FFT、特征 |
| `DiagTask` | 2 | 512 words | 每帧 | 故障状态机、报警 |
| `CommTask` | 1 | 512 words | 10–100ms | Modbus/USB |
| `UiLogTask` | 1 | 512 words | 100ms | OLED、SD 日志 |

第一版建议全部静态分配，使用 `xTaskCreateStatic`、`xQueueCreateStatic`。

## 5. 建议引脚

| 功能 | 引脚 | 外设 |
|---|---|---|
| 电机 PWM | PA8 | TIM1_CH1 |
| 电机 IN1/IN2 | PB0/PB1 | GPIO |
| 驱动 EN | PB2 | GPIO |
| 电流采样 | PA0 | ADC1_IN0 |
| 振动 SPI | PA5/PA6/PA7 | SPI1 |
| 振动 CS | PB12 | GPIO |
| 霍尔输入 | PA1 | TIM2_CH2 |
| OLED I2C | PB8/PB9 | I2C1 |
| Modbus UART | PA9/PA10 | USART1 |
| USB | PA11/PA12 | USB FS |
| SD SPI | PB13/PB14/PB15 | SPI2 |
| SD CS | PB10 | GPIO |
| 继电器 | PC0 | GPIO |
| 蜂鸣器 | PC1 | GPIO |
| 编码器 | PC2/PC3 | GPIO/EXTI |
| 急停 | PC4 | GPIO/EXTI |

## 6. Bring-up 顺序

1. 只上电，测 3.3V/5V/24V，确认无短路。
2. 烧录点灯和串口打印，确认时钟和下载正常。
3. TIM PWM 输出，示波器看频率/占空比，再接电机。
4. ADC+DMA 采电流，信号源或静态电流校准。
5. SPI 读 ADXL345，确认 WHO_AM_I 和数据。
6. TIM 输入捕获测转速，和光电/霍尔信号对比。
7. 上 FreeRTOS，先跑任务和队列，再迁移采集。
8. 接 FFT/诊断，先喂合成数据，再接真实传感器。
9. 接 Modbus/USB 和 SD 日志。
10. 做 24h 稳定性测试。

## 7. 安全

- 上电默认继电器断开、PWM=0。
- 急停直接切驱动 EN。
- 过流/堵转/超速/过温/传感器故障立即切断。
- 看门狗复位后保持安全状态，不自动重启电机。
- 电机加防护罩，24V 电源限流，不要带电插拔。