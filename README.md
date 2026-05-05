# LMU_Telemetry_STM32

演示视频：[Bilibili - 用 STM32 手搓 HY 仪表盘，把 LMU 遥测塞进小屏里](https://www.bilibili.com/video/BV11697B4Esw/)

![项目演示图](./demo.png)

## 项目简介
这是一个将 **Le Mans Ultimate / rFactor 2 Internals Plugin** 遥测数据实时显示到 **STM32F407 LCD 仪表盘** 上的项目。

整个系统由两部分组成：

- **上位机插件**：从 LMU / rF2 插件接口中读取档位、转速、车速、温度、圈速、油量、踏板等遥测数据，整理后通过 USB HID 发送给下位机。
- **下位机仪表盘**：基于 STM32F407 作为 USB HID Device 接收遥测数据，通过 FSMC 模拟 8080 时序驱动 LCD，完成赛车风格仪表盘显示。

## 当前功能
目前已经实现的显示内容包括：

- 档位
- 转速
- 车速
- 顶部转速灯
- 四轮刹车温度
- 四轮轮胎温度
- 水温 / 油温
- 最快圈 / 当前圈
- 剩余油量 / 油量百分比
- 油门 / 刹车踏板条

说明：
- 受 rF2 / LMU 插件接口限制，部分游戏内 MFD 数据无法直接获取，当前显示值与游戏内原始页面不一定完全一致
- 目前暂未实现或无法稳定获取的内容包括：虚拟能量、HY 组别电机输出功率 / 剩余电量、部分车辆调校参数（TC / ABS / 防倾杆 / 刹车比 / 刹车迁移 / 尾翼等）
- 该工程当前主要用于个人学习、嵌入式开发实践以及赛车遥测显示方案验证

## 工程结构
```text
LMU_Telemetry_STM32/
├─ LMU_Dash_F407/          # STM32F407 下位机显示工程
└─ rF2_telemetry_plugin/   # 上位机遥测插件工程
```

### 1. `LMU_Dash_F407`
STM32 端工程，主要负责：

- USB HID 接收与数据帧解析
- 遥测结构体更新
- LCD 驱动与界面绘制
- 仪表盘动态区域局部刷新

### 2. `rF2_telemetry_plugin`
上位机插件工程，主要负责：

- 读取 LMU / rF2 Internals Plugin 遥测数据
- 单位转换、限幅与格式整理
- 枚举 HID 设备并按定长报告发送遥测数据

## 数据链路
```text
Le Mans Ultimate / rF2
        ↓
Internals Plugin
        ↓
rF2_telemetry_plugin
        ↓ USB HID
STM32F407
        ↓ FSMC(8080)
LCD 仪表盘
```

## 通信与显示实现说明
### USB HID 通信
- PC 端作为 USB Host，STM32F407 作为 USB HID Device，通过开发板 `USB SLAVE` 接口通信
- 使用 **Custom HID Output Report** 传输遥测数据，单个报告长度为 `64` 字节
- `byte0` 为 `Report ID`，`byte1~byte35` 为原始遥测帧，保留 `0xAA 0x55` 帧头和固定字段布局，剩余字节补零
- 上位机按 `VID/PID` 枚举 HID 设备并发送报告，下位机在 HID OUT 回调中完成报告接收与数据解析

### 下位机接收逻辑
- 基于 STM32 USB OTG FS 外设和 ST USB Device Library 实现 Custom HID 设备
- 枚举完成后由 USB OUT Endpoint 接收上位机发送的 HID Report
- 在 `Custom HID OutEvent` 回调中校验 `Report ID` 和帧头后，按固定偏移解析车速、转速、温度、圈速、油量和踏板等字段
- 解析结果写入最新遥测结构体，主循环再根据新数据触发 LCD 局部刷新

### LCD 显示逻辑
- 静态背景初始化时绘制一次
- 动态内容根据数值变化局部刷新
- 大部分区域采用“先在缓冲区生成 GRAM 数据，再整块写入”的方式减少闪烁

## 开发环境
### 上位机插件
- Windows
- Visual Studio / MSVC（根据工程脚本配置）
- LMU / rFactor 2 Internals Plugin SDK

### 下位机
- STM32F407
- Keil MDK-ARM
- HAL 库
- FSMC 并口 LCD

## 编译说明
### 编译 STM32 工程
打开以下工程文件后使用 Keil 编译：

- `LMU_Dash_F407/Projects/MDK-ARM/LMU_Dash_F407.uvprojx`

### 编译上位机插件
在 `rF2_telemetry_plugin` 目录下运行：

```bat
build_plugin.bat
```

默认会生成：

- `rF2_telemetry_plugin/build/rF2_telemetry_plugin.dll`

## 使用说明
1. 编译并烧录 `LMU_Dash_F407` 到 STM32F407
2. 编译上位机插件 `rF2_telemetry_plugin.dll`
3. 将插件放入 LMU / rF2 的 `Plugins` 目录
4. 使用开发板 `USB SLAVE` 接口连接 PC，确保系统识别到 `LMU Telemetry HID` 设备
5. 进入游戏后即可在 LCD 上看到实时遥测显示

## 项目特点
- 基于游戏遥测的外接赛车仪表盘方案
- 上位机插件 + 下位机显示完整链路
- USB HID 免驱通信与定长报告传输
- STM32F407 通过 FSMC 模拟 8080 时序驱动 LCD
- 仪表盘界面采用静态背景 + 动态区域刷新设计
- 支持赛车风格转速灯和多区域遥测可视化显示

## 后续优化方向
- 优化上位机抓取遥测数据的方式，可以参考 TinyPedal 等项目通过 LMU API 获取更准确、更完整的遥测信息
- 完善 HID 数据报告的可靠性设计，例如增加校验字段、帧计数和主机侧异常重连策略，进一步提升通信稳定性
- 将当前手写 LCD 绘制逻辑逐步抽象为更清晰的界面层，后续可评估引入轻量级 GUI 框架或界面资源生成方案，提高界面设计效率和可维护性
- 适配不同车辆的转速灯策略与仪表盘布局，提高不同车型下的显示一致性和驾驶沉浸感
- 增加按键交互与多页面切换能力，根据驾驶状态在主界面、轮胎信息页、策略信息页等不同显示布局之间切换
- 利用 HID Input Report 增加 STM32 到上位机的状态回传能力，例如设备在线状态、按键输入或调试信息
