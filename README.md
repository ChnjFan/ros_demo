# ros_demo

ROS（Robot Operating System）入门学习工程 —— 基于 catkin 工作空间的 ROS 1 示例项目。

## 项目简介

`ros_demo` 是一个可以边跑边学的 ROS 1 学习工程：**catkin 工作空间里的示例代码，同时是一套中文教程中每一节的配套实现**。

教程每讲一个概念，工作空间里就有一个能编译、能运行的节点与之对应。阅读文档时看到的命令，都可以在本项目里直接执行。

### 项目包含什么

| 组成 | 内容 |
|------|------|
| **示例代码** | 4 个功能包，覆盖节点编写、话题通信、自定义消息、服务通信、自定义服务、参数操作 |
| **教程文档** | [ROS 1 完全教程](docs/tutorial/index.md)：11 章 48 课时，完整学习路径；前 2 章已发布 |
| **参考文档** | 架构详解、通信机制详解、命令速查手册、Docker 开发环境 4 份 |
| **文档站点** | MkDocs Material + GitHub Actions，推送 `main` 自动部署 |
| **容器环境** | Docker Compose 双服务（`ros-master` + `ros-dev`），macOS 上无需本机安装 ROS |

### 示例功能包

`src/` 下的每个包对应教程中的一个阶段，可以按顺序逐个运行：

| 功能包 | 对应教程 | 可执行节点 | 演示内容 |
|--------|---------|-----------|---------|
| [`hello_world`](src/hello_world) | 1.3 快速体验 | `hello_node` (C++)<br/>`helloworld_p.py` (Python) | 最小节点、`rosrun` / `roslaunch` |
| [`plumbing_pub_sub`](src/plumbing_pub_sub) | 2.1 话题通信 | `pub_node` / `sub_node`<br/>`pub_person_node` / `sub_person_node` | 发布订阅、自定义消息 [`Person.msg`](src/plumbing_pub_sub/msg/Person.msg) |
| [`plumbing_server_client`](src/plumbing_server_client) | 2.2 服务通信 | `add_int_server` / `add_int_client` | 服务端客户端、自定义服务 [`AddInt.srv`](src/plumbing_server_client/srv/AddInt.srv) |
| [`plumbing_param_server`](src/plumbing_param_server) | 2.3 参数服务器 | `param_server` | 参数的增删改查、缓存读取、`param()` 默认值 |

> 这些包是**教学示例的集合**，不是一个完整应用。它们之间没有依赖关系，每个包都能单独运行。

### 使用方式

📖 **在线文档**：<https://chnjfan.github.io/ros_demo/>

| 你的情况 | 建议路径 |
|---------|---------|
| 有 Ubuntu 20.04 / ROS Noetic | 按"快速开始"编译运行，对照教程逐节实践 |
| macOS，不想装 ROS | 走 [Docker Compose 快速开始](#docker-compose-快速开始)，无需本机环境 |
| 只想读教程 | 直接访问在线文档，不需要编译本项目 |

本地预览文档站点：

```bash
pip install -r requirements.txt
mkdocs serve                      # http://127.0.0.1:8000
```

## 项目结构

```
ros_demo/                                    # catkin 工作空间
├── src/                                     # 功能包源码
│   ├── hello_world/                         # 最小节点示例（C++ / Python + launch）
│   ├── plumbing_pub_sub/                    # 话题通信
│   │   ├── src/  demo_pub.cpp / demo_sub.cpp
│   │   │         demo_pub_person.cpp / demo_sub_person.cpp
│   │   └── msg/  Person.msg                 # 自定义消息
│   ├── plumbing_server_client/              # 服务通信
│   │   ├── src/  add_int_server.cpp / add_int_client.cpp
│   │   └── srv/  AddInt.srv                 # 自定义服务
│   └── plumbing_param_server/               # 参数服务器
│       └── src/  param_server.cpp
├── build/                                   # 编译中间产物（catkin_make 生成）
├── devel/                                   # 开发环境（setup.bash、可执行文件）
├── docker/                                  # 容器入口脚本
├── docs/                                    # 文档站（MkDocs Material + GitHub Actions）
│   ├── index.md                             # 站点首页
│   ├── arch.md                              # ROS 架构详解
│   ├── comm.md                              # ROS 1 通信机制详解
│   ├── ros_cmd.md                           # ROS 常用命令速查手册
│   ├── docker.md                            # Docker Compose 开发环境
│   └── tutorial/                            # ROS 1 完全教程
│       ├── index.md                         # 教程总目录（章节与课时状态）
│       ├── chapter01/                       # 第 1 章：01 ~ 05
│       └── chapter02/                       # 第 2 章：06 ~ 11
├── mkdocs.yml                               # 站点与导航配置
├── compose.yaml                             # Docker Compose 双服务定义
└── README.md
```

## 环境要求

| 依赖 | 版本 |
|------|------|
| Ubuntu | 18.04 / 20.04 |
| ROS 1 | Melodic / Noetic |
| CMake | ≥ 3.0.2 |
| Python | 2.7 (Melodic) / 3.x (Noetic) |

> 本项目使用 `catkin` 构建系统（ROS 1 标准构建工具）。

## 快速开始

### 1. 编译工作空间

```bash
cd ros_demo
catkin_make
```

### 2. 加载环境变量

```bash
source devel/setup.bash
```

> 建议将该命令追加到 `~/.bashrc`，避免每次手动执行：
> ```bash
> echo "source ~/ros_demo/devel/setup.bash" >> ~/.bashrc
> ```

### 3. 启动节点

**方式一：使用 launch 文件（推荐）**

```bash
roslaunch hello_world start_turtle.launch
```

该命令会同时启动两个节点：

| 节点名 | 语言 | 源文件 | 功能 |
|--------|------|--------|------|
| `hello_cpp` | C++ | `src/helloworld_c.cpp` | 输出 `hello world!测试` |
| `hello_python` | Python | `scripts/helloworld_p.py` | 输出 `hello world! by python` |

**方式二：使用 rosrun 单独启动**

```bash
# 需要先在另一个终端启动 ROS Master
roscore

# 启动 C++ 节点
rosrun hello_world hello_node

# 启动 Python 节点
rosrun hello_world helloworld_p.py
```

### 4. 验证节点运行

```bash
# 查看当前运行的节点列表
rosnode list

# 查看节点日志
rosnode info /hello_cpp
```

## Docker Compose 快速开始

启动 Docker 后，在项目根目录执行：

```bash
docker compose up -d --build
docker compose exec ros-dev catkin_make
docker compose exec ros-dev roslaunch hello_world start_turtle.launch
```

直接启动 `plumbing_pub_sub` 的 Topic 发布节点：

```bash
docker compose exec ros-dev rosrun plumbing_pub_sub pub_node
```

另开一个终端查看发布到 `/chatter` 的消息：

```bash
docker compose exec ros-dev rostopic echo /chatter
```

也可以在另一个终端启动订阅节点：

```bash
docker compose exec ros-dev rosrun plumbing_pub_sub sub_node
```

停止环境：

```bash
docker compose down
```

项目源码会实时挂载到容器；Linux 的 `build`、`devel` 产物保存在 Docker 中。详细说明见 [Docker Compose 开发环境](docs/docker.md)。

## 代码说明

### C++ 节点 (`src/helloworld_c.cpp`)

```cpp
#include <ros/ros.h>

int main(int argc, char *argv[])
{
    ros::init(argc, argv, "hello");   // 节点初始化，注册节点名
    ros::NodeHandle n;                 // 创建节点句柄
    setlocale(LC_ALL, "");             // 支持中文输出
    ROS_INFO("hello world!测试");      // 日志输出
    return 0;
}
```

### Python 节点 (`scripts/helloworld_p.py`)

```python
import rospy

if __name__ == "__main__":
    rospy.init_node("hello_p")               # 节点初始化
    rospy.loginfo("hello world! by python")  # 日志输出
```

### Launch 文件 (`launch/start_turtle.launch`)

```xml
<launch>
    <node pkg="hello_world" type="hello_node" name="hello_cpp" output="screen"/>
    <node pkg="hello_world" type="helloworld_p.py" name="hello_python" output="screen"/>
</launch>
```

- `pkg`：功能包名称
- `type`：可执行文件名
- `name`：节点运行时名称
- `output="screen"`：日志输出到终端

## 常用命令速查

> 完整命令说明与使用示例见 [ROS 常用命令速查手册](docs/ros_cmd.md)

| 命令 | 作用 |
|------|------|
| `catkin_make` | 编译工作空间内所有功能包 |
| `source devel/setup.bash` | 加载工作空间环境变量 |
| `roscore` | 启动 ROS Master |
| `roslaunch <pkg> <file.launch>` | 通过 launch 文件启动节点 |
| `rosrun <pkg> <node>` | 运行单个节点 |
| `rosnode list` | 列出当前所有节点 |
| `rostopic list` | 列出当前所有话题 |
| `rostopic echo <topic>` | 实时打印话题数据 |
| `rosmsg show <msg_type>` | 查看消息类型定义 |
| `rqt_graph` | 图形化查看节点连接关系 |


## 实现进度

示例代码与教程文档同步推进。教程共 11 章 48 课时，已完成的部分如下：

| 章节 | 教程 | 配套代码 | 状态 |
|------|:----:|:-------:|:----:|
| 第 1 章 ROS 概述与环境搭建 | 5 课时 | `hello_world` | ✅ |
| 第 2 章 ROS 通信机制 | 6 课时 | `plumbing_pub_sub`<br/>`plumbing_server_client`<br/>`plumbing_param_server` | ✅ |
| 第 3 ~ 11 章 | 37 课时 | — | ⬜ |

后续章节将按教程规划继续补充：通信机制进阶（常用 API、头文件与模块）、运行管理（launch、命名空间、分布式通信）、常用组件（TF、rosbag、rqt）、机器人仿真（URDF、Gazebo）、导航（SLAM、路径规划）、机器人平台设计、Action 通信与动态参数。

大纲与课时状态见[教程总目录](docs/tutorial/index.md)。
