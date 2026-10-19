# MotorGuard

基于 STM32F407 + FreeRTOS 的电机驱动—诊断一体化边缘控制器。

## 当前进度

- [x] 环形缓冲区
- [x] Modbus CRC16
- [x] Radix-2 FFT
- [x] 时域/频域特征提取
- [x] 故障诊断状态机
- [x] 主机端单元测试
- [x] 端到端仿真器
- [x] BSP 抽象与故障切断控制器
- [ ] STM32 HAL/FreeRTOS BSP
- [ ] 采集/控制/通信任务
- [ ] 自定义 PCB

## 构建与测试

建议在 ASCII 路径下构建，避免 Ninja 在中文路径上报错：

```powershell
cd MotorGuard
cmake -S . -B build_ascii -G Ninja
cmake --build build_ascii
ctest --test-dir build_ascii --output-on-failure
```

旧 `build/` 是中文路径下生成的缓存，可忽略；统一使用 `build_ascii/`。

## 运行仿真器

```powershell
.\build_ascii\mg_simulator.exe
```

预期输出：

```text
normal     ... fault=NONE
imbalance  ... fault=IMBALANCE
bearing    ... fault=BEARING
overload   ... fault=OVERLOAD
```

## 目录

- `include/mg/`：硬件无关核心接口
- `src/core/`：核心算法与协议实现
- `tests/`：主机端单元测试
- `tools/`：合成信号仿真器
- `firmware/bsp/`：硬件抽象接口
- `firmware/app/`：应用控制器与配置
- `docs/`：BOM、架构、计划、测试与面试资料

## 说明

Windows 下建议把项目放在纯英文路径，避免 Ninja 在中文路径上报错。