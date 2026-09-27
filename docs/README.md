# ROS Demo 学习笔记

基于 catkin 工作空间的 ROS 1 入门学习工程，包含 C++ / Python 双语言节点示例，以及系统化的中文文档。

## 文档导航

| 文档 | 类型 | 内容 |
|------|------|------|
| [ROS 1 完全教程](tutorial/README.md) | 📘 教程 | 面向零基础的 ROS 1 完整学习路径（11 章，48 课时） |
| [ROS 架构详解](arch.md) | 📖 解释 | 计算图模型、节点、四种通信机制概览、功能包结构、ROS 1 vs ROS 2 对比 |
| [ROS 1 通信机制详解](comm.md) | 📗 参考 | XMLRPC/TCPROS 协议栈、Topic/Service/Action/Parameter 原理与代码实战、回调机制、问题排查 |
| [ROS 常用命令速查手册](ros_cmd.md) | 📗 参考 | 节点/话题/服务/参数/rosbag 命令详解，调试场景与使用示例 |

> **阅读建议**：零基础新手请从 [📘 教程](tutorial/README.md) 开始；需要查阅概念原理或命令参数时，跳转到对应的解释/参考文档。

## 项目快速开始

```bash
# 编译工作空间
cd ros_demo && catkin_make

# 加载环境变量
source devel/setup.bash

# 一键启动 C++ 与 Python 双节点
roslaunch hello_world start_turtle.launch
```

## 项目结构

```
ros_demo/
├── src/hello_world/     # 功能包：C++/Python 节点 + launch 文件
├── docs/                # 本文档站
│   ├── README.md        # 文档导航（本文件）
│   ├── arch.md          # 架构详解
│   ├── comm.md          # 通信机制详解
│   ├── ros_cmd.md       # 命令速查
│   └── tutorial/        # ROS 1 完全教程
│       ├── README.md    # 教程总目录
│       └── chapter01/   # 第 1 章：ROS 概述与环境搭建
│           └── 01-introduction.md  # 1.1 ROS 简介
└── README.md            # 项目说明
```
