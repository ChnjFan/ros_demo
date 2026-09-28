# ROS 1 完全教程

> **基于 [Autolabor ROS 教程](https://www.autolabor.com.cn/book/ROSTutorials/) 的学习笔记整理。**
>
> 免费、零基础、理论与实践相结合。从"Hello World"到搭建一台真实机器人，系统掌握 ROS 1 开发与通信机制。

---

## 学习理念

本教程遵循 **Diátaxis 框架** 的 **Tutorial（教程）** 原则：

| 原则 | 体现 |
|------|------|
| **学习导向** | 每章以具体目标驱动，不是知识罗列 |
| **动手实操** | C++ / Python 双语代码示例，边学边做 |
| **循序渐进** | 从环境搭建到真实机器人，难度平滑上升 |
| **理论结合实践** | 每个通信机制都有"理论模型 + 基本操作 + 自定义消息"三段式 |

> **教程不是参考手册**。如需查阅命令参数或原理细节，请跳转：
> - [ROS 架构详解](../arch.md) — 概念与架构深入解释
> - [ROS 1 通信机制详解](../comm.md) — 通信原理与协议栈参考
> - [ROS 常用命令速查](../ros_cmd.md) — 命令参数速查手册

---

## 前置条件

| 要求 | 说明 |
|------|------|
| **操作系统** | Ubuntu 20.04 (推荐) 或已安装 WSL2 的 Windows |
| **ROS 版本** | ROS Noetic (ROS 1 最后一个 LTS 版本) |
| **编程基础** | 了解 C++ 或 Python 的基本语法（双语示例） |
| **Linux 基础** | 会使用终端、cd、ls 等基本命令 |
| **硬件** | 普通 PC 即可，第 1-7 章全程使用仿真 |

---

## 章节导航

### 第 1 章：ROS 概述与环境搭建

> 理解 ROS 是什么，搭建开发环境，跑通第一个程序。

| 小节 | 内容 | 状态 |
|------|------|:----:|
| [1.1 ROS 简介](chapter01/01-introduction.md) | ROS 概念、设计目标、发展历程 | ✅ |
| [1.2 ROS 安装](chapter01/02-installation.md) | 虚拟机 + Ubuntu + ROS 安装、测试 | ✅ |
| [1.3 ROS 快速体验](chapter01/03-helloworld.md) | HelloWorld 实现 (C++ / Python) | ✅ |
| [1.4 ROS 集成开发环境搭建](chapter01/04-ide-setup.md) | 终端、VScode、launch 演示 | ✅ |
| [1.5 ROS 架构](chapter01/05-architecture.md) | 文件系统、文件系统命令、计算图 | ✅ |

---

### 第 2 章：ROS 通信机制

> **核心章节**。话题通信、服务通信、参数服务器，含 C++ / Python 双语实现。

| 小节 | 内容 | 状态 |
|------|------|:----:|
| [2.1 话题通信](chapter02/06-topic.md) | 理论模型 → 基本操作 (C++/Python) → 自定义 msg | ✅ |
| [2.2 服务通信](chapter02/07-service.md) | 理论模型 → 自定义 srv → 调用 (C++/Python) | ✅ |
| [2.3 参数服务器](chapter02/08-parameter.md) | 理论模型 → 参数操作 (C++/Python) | ✅ |
| [2.4 常用命令](chapter09/09-commands.md) | rosnode / rostopic / rosmsg / rosservice / rossrv / rosparam | ⬜ |
| [2.5 通信机制实操](chapter10/10-communication-practice.md) | 话题发布 / 话题订阅 / 服务调用 / 参数设置 | ⬜ |
| [2.6 通信机制比较](chapter11/11-communication-comparison.md) | 四种通信方式对比与选型 | ⬜ |

---

### 第 3 章：ROS 通信机制进阶

> 深入 API 层：初始化、对象模型、回调、时间、头文件与模块导入。

| 小节 | 内容 | 状态 |
|------|------|:----:|
| [3.1 常用 API](chapter12/12-api.md) | 初始化关闭、话题服务对象、回调函数、时间 | ⬜ |
| [3.2 ROS 中的头文件与源文件](chapter13/13-headers-sources.md) | 自定义头文件调用、自定义源文件调用 | ⬜ |
| [3.3 Python 模块导入](chapter14/14-python-module.md) | Python 模块导入机制 | ⬜ |

---

### 第 4 章：ROS 运行管理

> 零散但常用的知识点：launch 文件、命名空间、重映射、分布式通信。

| 小节 | 内容 | 状态 |
|------|------|:----:|
| [4.1 ROS 元功能包](chapter15/15-metapackages.md) | 元功能包概念与使用 | ⬜ |
| [4.2 ROS 节点管理 launch 文件](chapter16/16-launch.md) | launch / node / include / remap / param / rosparam / group / arg 标签 | ⬜ |
| [4.3 ROS 工作空间覆盖](chapter17/17-workspace-overlay.md) | 工作空间覆盖机制 | ⬜ |
| [4.4 ROS 节点名称重名](chapter18/18-node-name-conflict.md) | rosrun / launch / 编码 设置命名空间与重映射 | ⬜ |
| [4.5 ROS 话题名称设置](chapter19/19-topic-name.md) | rosrun / launch / 编码 设置话题名称 | ⬜ |
| [4.6 ROS 参数名称设置](chapter20/20-param-name.md) | rosrun / launch / 编码 设置参数 | ⬜ |
| [4.7 ROS 分布式通信](chapter21/21-distributed.md) | 多机分布式通信配置 | ⬜ |

---

### 第 5 章：ROS 常用组件

> 实用功能模块：TF 坐标变换、rosbag、rqt 工具箱。

| 小节 | 内容 | 状态 |
|------|------|:----:|
| [5.1 TF 坐标变换](chapter22/22-tf.md) | 坐标 msg / 静态 / 动态 / 多坐标 / TF2 | ⬜ |
| [5.2 rosbag](chapter23/23-rosbag.md) | 命令行使用 / 编码使用 | ⬜ |
| [5.3 rqt 工具箱](chapter24/24-rqt.md) | rqt_graph / rqt_console / rqt_plot / rqt_bag | ⬜ |

---

### 第 6 章：机器人系统仿真

> 在 Rviz 和 Gazebo 中创建机器人模型、仿真传感器、控制运动。

| 小节 | 内容 | 状态 |
|------|------|:----:|
| [6.1 概述](chapter25/25-simulation-overview.md) | 仿真流程总览 | ⬜ |
| [6.2 URDF 集成 Rviz 基本流程](chapter26/26-urdf-rviz.md) | URDF 与 Rviz 集成 | ⬜ |
| [6.3 URDF 语法详解](chapter27/27-urdf-syntax.md) | robot / link / joint 标签 | ⬜ |
| [6.4 URDF 优化 xacro](chapter28/28-xacro.md) | Xacro 语法与完整流程 | ⬜ |
| [6.5 Rviz 中控制机器人模型运动](chapter29/29-rviz-control.md) | Arbotix 使用 | ⬜ |
| [6.6 URDF 集成 Gazebo](chapter30/30-urdf-gazebo.md) | URDF 与 Gazebo 集成、仿真环境搭建 | ⬜ |
| [6.7 URDF、Gazebo 与 Rviz 综合应用](chapter31/31-simulation-integration.md) | 运动控制 / 雷达 / 摄像头 / Kinect | ⬜ |

---

### 第 7 章：机器人导航（仿真）

> 在仿真环境中实现 SLAM 建图、定位、路径规划。

| 小节 | 内容 | 状态 |
|------|------|:----:|
| [7.1 概述](chapter32/32-navigation-overview.md) | 导航模块、坐标系、条件说明 | ⬜ |
| [7.2 导航实现](chapter33/33-navigation-implementation.md) | SLAM 建图 / 地图服务 / 定位 / 路径规划 | ⬜ |
| [7.3 导航相关消息](chapter34/34-navigation-messages.md) | 地图 / 里程计 / 坐标变换 / 定位 / 雷达 / 相机 | ⬜ |

---

### 第 8 章：机器人平台设计

> 从 0 到 1 搭建一台实体机器人：Arduino 底盘、电机驱动、PID 控制、传感器集成。

| 小节 | 内容 | 状态 |
|------|------|:----:|
| [8.1 概述](chapter35/35-robot-overview.md) | 机器人平台总体设计 | ⬜ |
| [8.2 Arduino 基础](chapter36/36-arduino-basics.md) | 开发环境、语法、演示 | ⬜ |
| [8.3 电机驱动](chapter37/37-motor-driver.md) | 电机控制、测速、PID 调速 | ⬜ |
| [8.4 底盘实现](chapter38/38-chassis.md) | Arduino 端编码器、电机驱动、PID 控制 | ⬜ |
| [8.5 控制系统](chapter39/39-control-system.md) | 树莓派、分布式框架、SSH、ros_arduino_bridge | ⬜ |
| [8.6 传感器](chapter40/40-sensors.md) | 激光雷达、相机、集成 | ⬜ |

---

### 第 9 章：机器人导航（实体）

> 将仿真导航移植到真实机器人，含 VS Code 远程开发。

| 小节 | 内容 | 状态 |
|------|------|:----:|
| [9.1 概述](chapter41/41-real-navigation-overview.md) | 实体导航流程总览 | ⬜ |
| [9.2 VS Code 远程开发](chapter42/42-vscode-remote.md) | 远程开发环境配置 | ⬜ |
| [9.3 导航实现](chapter43/43-real-navigation.md) | 准备工作 / SLAM / 地图 / 定位 / 路径规划 | ⬜ |

---

### 第 10 章：ROS 进阶

> Action 通信、动态参数、pluginlib、nodelet。

| 小节 | 内容 | 状态 |
|------|------|:----:|
| [10.1 Action 通信](chapter44/44-action.md) | 自定义 action → C++ / Python 实现 | ⬜ |
| [10.2 动态参数](chapter45/45-dynamic-reconfigure.md) | 客户端 / 服务端 (C++ / Python) | ⬜ |
| [10.3 pluginlib](chapter46/46-pluginlib.md) | 插件库使用 | ⬜ |
| [10.4 nodelet](chapter47/47-nodelet.md) | 使用演示 / 实现 | ⬜ |

---

### 第 11 章：ROS 项目

> 综合实战项目，串联全书知识点。

| 小节 | 内容 | 状态 |
|------|------|:----:|
| [11.1 项目实战](chapter48/48-projects.md) | 综合项目开发 | ⬜ |

---

## 学习路线图

```
第 1 章               第 2 章               第 3 章
ROS 概述与搭建    →   通信机制(核心)    →   通信进阶
                                           │
                                           ▼
第 5 章               第 4 章
常用组件          ←   运行管理
(TF/rosbag/rqt)       (launch/命名空间)
       │
       ▼
第 6 章               第 7 章
机器人仿真          →   导航(仿真)
(URDF/Gazebo)         (SLAM/定位/规划)
                         │
                         ▼
                  第 8 章               第 9 章
                  机器人平台设计    →   导航(实体)
                  (Arduino/传感器)      (真实机器人)
                                           │
                                           ▼
                                    第 10 章              第 11 章
                                    ROS 进阶          →   项目实战
                                    (Action/动态参数)     (综合)
```

---

## 使用建议

| 符号 | 含义 |
|:----:|------|
| ⬜ | 待撰写 |
| ✅ | 已完成 |
| 💡 | 关键提示 / 最佳实践 |
| ⚠️ | 常见陷阱 / 注意事项 |
| 🔧 | 动手练习 |
| 📖 | 延伸阅读（指向参考文档） |

### 学习节奏

| 学习者类型 | 建议路径 |
|-----------|---------|
| **零基础** | 按顺序 1 → 11 章 |
| **有 Linux 基础** | 跳过 1.1、1.2，从 1.3 开始 |
| **有 ROS 基础** | 重点看第 2、3 章，其余按需查阅 |
| **做仿真项目** | 1 → 2 → 5 → 6 → 7 |
| **做实体机器人** | 1 → 2 → 5 → 8 → 9 |

---

## 相关文档

| 文档 | 类型 | 用途 |
|------|------|------|
| [ROS 架构详解](../arch.md) | 解释 | 深入理解 ROS 架构设计 |
| [ROS 1 通信机制详解](../comm.md) | 参考 | 通信原理与协议栈细节 |
| [ROS 常用命令速查](../ros_cmd.md) | 参考 | 命令参数快速查阅 |
| [项目 README](../README.md) | — | 工程说明与快速开始 |
| [Autolabor 原教程](https://www.autolabor.com.cn/book/ROSTutorials/) | — | 原始教程网站 |

---

## 开始学习

准备好了吗？让我们从认识 ROS 开始：

→ **[第 1.1 节：ROS 简介](chapter01/01-introduction.md)**
