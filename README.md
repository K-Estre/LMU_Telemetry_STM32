# LMU_Telemetry_STM32

演示视频：[Bilibili - 用 STM32 手搓 HY 仪表盘，把 LMU 遥测塞进小屏里](https://www.bilibili.com/video/BV11697B4Esw/)

![项目演示图](./demo.png)

## 项目简介
这是一个将 **Le Mans Ultimate / rFactor 2 Internals Plugin** 遥测数据实时显示到 **STM32F407 LCD 仪表盘** 上的项目。

整个系统由两部分组成：

- **上位机插件**：从 LMU / rF2 插件接口中读取档位、转速、车速、温度、圈速、油量、踏板等遥测数据，并通过 UART 发送给下位机。
- **下位机仪表盘**：基于 STM32F407 解析串口数据，通过 FSMC 模拟 8080 时序驱动 LCD，完成赛车风格仪表盘显示。

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
- 备注：rF2插件抓取的遥测信息与LMU游戏内MFD显示不一致，部分遥测值无法获取（虚拟能量、HY组别电机输出功率/剩余电量、车辆调校数据TC/ABS/防倾杆/刹车比/刹车迁移/尾翼等），该工程主要目的为学习和测试

## 工程结构
```text
LMU_Telemetry_STM32/
├─ LMU_Dash_F407/          # STM32F407 下位机显示工程
└─ rF2_telemetry_plugin/   # 上位机遥测插件工程
```

### 1. `LMU_Dash_F407`
STM32 端工程，主要负责：

- UART 接收和解帧
- 遥测结构体更新
- LCD 驱动与界面绘制
- 仪表盘动态区域局部刷新

### 2. `rF2_telemetry_plugin`
上位机插件工程，主要负责：

- 读取 LMU / rF2 Internals Plugin 遥测数据
- 单位转换、限幅与格式整理
- 按定长帧协议通过 UART 输出

## 数据链路
```text
Le Mans Ultimate / rF2
        ↓
Internals Plugin
        ↓
rF2_telemetry_plugin
        ↓ UART
STM32F407
        ↓ FSMC(8080)
LCD 仪表盘
```

## 通信与显示实现说明
### UART 通信
- 使用定长帧协议
- 帧头为 `0xAA 0x55`
- 上位机发送完整遥测帧，下位机进行状态机解帧

### 下位机接收逻辑
- 当前版本使用 **UART DMA 持续接收串口数据**
- 主循环按固定周期轮询 DMA 写入位置
- 将新增字节按顺序送入状态机进行解帧
- 解出完整帧后更新最新遥测结构体
- LCD 再根据最新值进行局部刷新

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
4. 配置上位机串口并连接 STM32
5. 进入游戏后即可在 LCD 上看到实时遥测显示

## 项目特点
- 基于游戏遥测的外接赛车仪表盘方案
- 上位机插件 + 下位机显示完整链路
- UART 定长帧协议传输
- STM32F407 通过 FSMC 模拟 8080 时序驱动 LCD
- 仪表盘界面采用静态背景 + 动态区域刷新设计
- 支持赛车风格转速灯和多区域遥测可视化显示

## 后续优化方向
- 优化上位机抓取遥测数据的方式，可以参考tinypedal等遥测插件通过LMU的API获取更准确的遥测数据


## 说明
该项目主要用于个人学习、嵌入式开发实践以及赛车遥测显示方案验证。

