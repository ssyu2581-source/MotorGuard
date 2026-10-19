# MotorGuard 系统架构

## 1. 总体框图

```text
24V 电机 <-> BTS7960 <-> STM32F407
                 ^
                 | PWM / DIR / EN
INA240 -> ADC+DMA
ADXL345 -> SPI
霍尔传感器 -> TIM 输入捕获
DS18B20 -> 1-Wire
OLED / 编码器 / 蜂鸣器 / 继电器
UART <-> RS485 / Modbus
USB <-> 上位机
microSD <-> 日志
```

## 2. 数据流

```text
电流 ADC 20kHz -> 双缓冲 -> 环形缓冲 -> 时域特征
振动 SPI 3.2kHz -> 环形缓冲 -> Hann 窗 -> FFT 2048 -> 频域特征
转速 TIM IC 1kHz -> RPM
温度 10Hz -> 温度
                         |
                         v
              mg_fault_detector 状态机
                         |
          +--------------+--------------+
          |                             |
    继电器/电机使能                  Modbus/USB/日志
```

## 3. FreeRTOS 任务

| 任务 | 优先级 | 周期 | 主要工作 |
|---|---:|---:|---|
| `AcqTask` | 5 | 20kHz 触发 | ADC+DMA、SPI 振动、环形缓冲 |
| `ControlTask` | 4 | 1kHz | 速度闭环、堵转/过流/超速保护 |
| `DspTask` | 3 | 每帧 | 加窗、FFT、特征提取 |
| `DiagTask` | 2 | 每帧 | 故障状态机、报警决策 |
| `CommTask` | 1 | 10–100ms | Modbus RTU、USB CDC |
| `UiLogTask` | 1 | 100ms/事件 | OLED、编码器、SD 日志 |

同步机制：

- ADC DMA 半传输/全传输中断 -> `vTaskNotifyGiveFromISR`
- SPI DRDY 外部中断 -> 任务通知
- 数据块 -> 队列或流缓冲
- SPI/I2C 总线 -> 互斥量
- 系统状态 -> 事件组
- IWDG -> 多任务心跳检查

## 4. 内存预算

| 项目 | 大小 |
|---|---:|
| 电流环形缓冲 4096 float | 16KB |
| 振动环形缓冲 4096 float | 16KB |
| FFT real/imag/magnitude 2048 float | 24KB |
| 任务栈和队列 | 约 16–24KB |
| 合计 | 约 72–80KB |

STM32F407 有 192KB SRAM，余量足够。第一版全部使用静态分配，避免堆碎片。

## 5. 关键设计

- 电流采样由 TIM 硬件触发 ADC，DMA 双缓冲，保证采样间隔稳定。
- 振动数据先进入环形缓冲，DSP 任务按 50% 重叠取 2048 点。
- FFT 使用 CMSIS-DSP 或本项目 Radix-2 实现，Hann 窗减少泄漏。
- 故障判定采用“阈值 + 持续性计数 + 清除计数”，避免单帧误报。
- 任何故障都先切断电机使能，再上报；故障清除需要连续正常帧。
- 看门狗由多个任务心跳共同喂狗，避免单任务卡死仍被喂狗。

## 6. 建议引脚

| 功能 | 引脚 | 外设 |
|---|---|---|
| 电机 PWM | PA8 | TIM1_CH1 |
| 电机 IN1/IN2 | PB0/PB1 | GPIO |
| 驱动 EN | PB2 | GPIO |
| 电流采样 | PA0 | ADC1_IN0 |
| 振动 SPI | PA5/PA6/PA7 | SPI1 SCK/MISO/MOSI |
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

实际引脚以开发板和外设可用性为准，先保证不冲突。

## 7. 安全状态

- 上电默认：继电器断开、电机 PWM=0。
- 过流/堵转/超速/过温/传感器故障：立即切断电机使能。
- 故障清除：连续 `clear_frames` 帧正常后才恢复。
- 看门狗复位后保持安全状态，不自动重启电机。
- 急停开关直接切驱动使能，不只依赖软件。