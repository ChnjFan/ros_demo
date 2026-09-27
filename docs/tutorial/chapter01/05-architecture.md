# 1.5 ROS 架构

> 本节学习目标：理解 ROS 的文件系统组织方式、计算图模型，以及操作文件系统的常用命令，建立对 ROS 系统架构的完整认知。

---

## 1.5.1 ROS 文件系统

ROS 的文件系统分为两个层级：**文件系统级**（磁盘上的目录结构）和**计算图级**（运行时的逻辑连接）。

### 文件系统级的核心概念

ROS 文件系统有四个核心概念，它们层层包含：

```
Workspace（工作空间）
  └── Package（功能包）
        └── Node（节点）
              └── Source Code（源码）
```

| 概念 | 比喻 | 作用 |
|------|------|------|
| **Workspace（工作空间）** | 一个项目工程 | 存放多个功能包的容器 |
| **Package（功能包）** | 一个功能模块 | ROS 代码组织的基本单元 |
| **Node（节点）** | 一个独立进程 | 实际执行计算的程序 |
| **Source Code（源码）** | 具体实现代码 | C++ / Python 源文件 |

### 功能包（Package）详解

功能包是 ROS 中最核心的代码组织单位，一个典型的功能包结构如下：

```
my_package/                         ← 功能包根目录
├── package.xml                     ← 包的"身份证"（元信息）
├── CMakeLists.txt                  ← 包的"配方"（编译规则）
├── src/                            ← C++ 源码目录
│   ├── my_node.cpp
│   └── my_class.cpp
├── include/                        ← C++ 头文件目录
│   └── my_package/
│       └── my_class.h
├── scripts/                        ← Python 脚本目录
│   └── my_node.py
├── msg/                            ← 自定义消息定义
│   └── MyMessage.msg
├── srv/                            ← 自定义服务定义
│   └── MyService.srv
├── action/                         ← 自定义动作定义
│   └── MyAction.action
├── launch/                         ← launch 启动文件
│   └── start_all.launch
├── config/                         ← 配置文件
│   └── params.yaml
└── urdf/                           ← 机器人模型文件
    └── my_robot.urdf
```

**关键文件说明：**

**`package.xml`** —— 包的元信息文件，描述了包的"身份"：

```xml
<?xml version="1.0"?>
<package format="2">
  <name>my_package</name>           <!-- 包名 -->
  <version>1.0.0</version>          <!-- 版本号 -->
  <description>我的ROS功能包</description>  <!-- 描述 -->

  <!-- 维护者信息 -->
  <maintainer email="user@example.com">用户名</maintainer>

  <!-- 许可证 -->
  <license>BSD</license>

  <!-- 依赖声明 -->
  <buildtool_depend>catkin</buildtool_depend>    <!-- 构建工具 -->
  <build_depend>roscpp</build_depend>            <!-- 编译依赖 -->
  <build_depend>rospy</build_depend>
  <build_depend>std_msgs</build_depend>
  <exec_depend>roscpp</exec_depend>              <!-- 运行依赖 -->
  <exec_depend>rospy</exec_depend>
  <exec_depend>std_msgs</exec_depend>
</package>
```

> 💡 **package.xml 中的依赖类型**：
> - `buildtool_depend`：构建工具（catkin）
> - `build_depend`：编译时需要的库
> - `exec_depend`：运行时需要的库
> - `depend`：同时是编译和运行依赖（简写形式）

**`CMakeLists.txt`** —— 编译规则文件，告诉 catkin 如何构建这个包：

```cmake
cmake_minimum_required(VERSION 3.0.2)
project(my_package)

# 1. 查找依赖包
find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
)

# 2. 声明 catkin 包
catkin_package(
  CATKIN_DEPENDS roscpp rospy std_msgs
)

# 3. 指定头文件搜索路径
include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

# 4. 编译 C++ 可执行文件
add_executable(my_node src/my_node.cpp)
target_link_libraries(my_node ${catkin_LIBRARIES})

# 5. 安装 Python 脚本
catkin_install_python(PROGRAMS scripts/my_node.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
```

### 工作空间（Workspace）结构

```
ros_demo/                           ← catkin 工作空间
├── src/                            ← 源代码空间（Source Space）
│   ├── CMakeLists.txt              ← 顶层 CMake（catkin 自动生成）
│   ├── hello_world/                ← 功能包 1
│   │   ├── package.xml
│   │   ├── CMakeLists.txt
│   │   └── ...
│   └── my_navigation/              ← 功能包 2
│       └── ...
├── build/                          ← 编译空间（Build Space）
│   └── ...                         ← CMake 编译中间产物
├── devel/                          ← 开发空间（Development Space）
│   ├── setup.bash                  ← 环境变量配置脚本
│   ├── setup.sh
│   └── share/                      ← 包配置文件
└── install/                        ← 安装空间（Installation Space）
    └── ...                         ← 安装后的文件（可选）
```

| 目录 | 名称 | 内容 |
|------|------|------|
| `src/` | Source Space | 功能包源码 |
| `build/` | Build Space | CMake 缓存、编译中间产物 |
| `devel/` | Devel Space | 编译后的可执行文件、环境变量脚本 |
| `install/` | Install Space | `catkin_make install` 后的安装目录 |

> 💡 **日常开发主要关注 `src/` 和 `devel/`**：`src/` 是你的代码，`devel/setup.bash` 是每次新终端必须 source 的环境变量。

### 消息、服务、动作定义

ROS 提供三种自定义接口的方式：

| 类型 | 文件后缀 | 存放目录 | 用途 |
|------|---------|---------|------|
| **消息（Message）** | `.msg` | `msg/` | 定义话题通信的数据结构 |
| **服务（Service）** | `.srv` | `srv/` | 定义服务通信的请求和响应 |
| **动作（Action）** | `.action` | `action/` | 定义动作通信的目标、反馈、结果 |

**.msg 示例**（`msg/Person.msg`）：

```
string name
uint8 age
float64 height
```

**.srv 示例**（`srv/AddTwoInts.srv`）：

```
int64 a        # ← 请求（Request）
int64 b
---            # ← 分隔线
int64 sum      # ← 响应（Response）
```

**.action 示例**（`action/Navigate.action`）：

```
# 目标（Goal）
float64 target_x
float64 target_y
---
# 反馈（Feedback）
float64 current_distance
---
# 结果（Result）
bool success
float64 final_distance
```

> 📖 这些自定义接口的编写和使用方法，将在第 2 章通信机制中详细讲解。

---

## 1.5.2 ROS 文件系统相关命令

ROS 提供了一系列以 `ros` 为前缀的快捷命令，用于操作文件系统，避免记忆复杂的路径。

### 快速跳转命令

| 命令 | 作用 | 示例 |
|------|------|------|
| `roscd` | 跳转到功能包目录 | `roscd hello_world` |
| `roscd` | 跳转到子目录 | `roscd hello_world/launch` |
| `rosls` | 列出功能包内容 | `rosls hello_world` |
| `rosed` | 编辑功能包内文件 | `rosed hello_world package.xml` |

```bash
# 直接跳转到 hello_world 包目录（不用记完整路径）
roscd hello_world

# 列出 hello_world 包的所有文件和目录
rosls hello_world

# 用默认编辑器打开 package.xml
rosed hello_world package.xml

# 用指定编辑器打开
EDITOR=code rosed hello_world package.xml
```

### 包查询命令

| 命令 | 作用 | 示例 |
|------|------|------|
| `rospack find` | 查找包的完整路径 | `rospack find hello_world` |
| `rospack list` | 列出所有已注册的包 | `rospack list` |
| `rospack profile` | 刷新包索引 | `rospack profile` |

```bash
# 查找 hello_world 包的路径
rospack find hello_world
# 输出：/home/user/ros_demo/src/hello_world

# 列出系统中所有 ROS 包
rospack list

# 查找包含关键词的包
rospack list | grep turtle
```

### 消息 / 服务查询命令

| 命令 | 作用 | 示例 |
|------|------|------|
| `rosmsg list` | 列出所有消息类型 | `rosmsg list` |
| `rosmsg show` | 查看消息结构 | `rosmsg show std_msgs/String` |
| `rosmsg package` | 查看包中所有消息 | `rosmsg package std_msgs` |
| `rossrv list` | 列出所有服务类型 | `rossrv list` |
| `rossrv show` | 查看服务结构 | `rossrv show turtlesim/Spawn` |

```bash
# 查看标准字符串消息的结构
rosmsg show std_msgs/String
# 输出：string data

# 查看海龟仿真器的 Spawn 服务定义
rossrv show turtlesim/Spawn
# 输出：
# float32 x
# float32 y
# float32 theta
# string name
# ---
# string name
```

### Tab 补全

ROS 命令大多支持 **Tab 键自动补全**，善用补全可以大幅提高效率：

```bash
rosrun hello_world [Tab][Tab]    # 列出该包所有可执行节点
rostopic [Tab][Tab]              # 列出所有子命令
rosservice list [Tab][Tab]       # 列出所有服务
```

> 💡 **如果 Tab 补全不工作**，可能需要安装 bash 补全：
> ```bash
> sudo apt install bash-completion
> ```

---

## 1.5.3 ROS 计算图（Computation Graph）

计算图是 ROS 的**运行时视图**——当多个节点启动后，它们之间的通信关系构成了一张图。

### 计算图的核心概念

```
                    ┌──────────────┐
                    │  ROS Master  │  ← 节点注册与发现中心
                    └──────┬───────┘
                           │
         ┌─────────────────┼─────────────────┐
         │                 │                 │
    ┌────▼────┐       ┌────▼────┐       ┌────▼────┐
    │ 节点 A   │       │ 节点 B   │       │ 节点 C   │
    │(Publisher)│       │(Subscriber)│    │(Server)  │
    └────┬────┘       └────┬────┘       └────┬────┘
         │                 │                 │
         └──── /chatter ───┘                 │
                                           │
                               ┌───────────▼───────────┐
                               │        Client          │
                               └───────────────────────┘
```

| 计算图概念 | 说明 | 类比 |
|-----------|------|------|
| **Node（节点）** | 一个独立的计算进程 | 一个"人" |
| **Master（主控）** | 节点注册与发现中心 | "电话总机" |
| **Topic（话题）** | 异步发布/订阅的数据流 | "广播频道" |
| **Message（消息）** | 话题上传输的数据单元 | "广播内容" |
| **Service（服务）** | 同步请求/响应 | "打电话" |
| **Parameter（参数）** | 全局键值存储 | "公告板" |
| **Bag（数据包）** | 话题数据的录制与回放 | "录像带" |

### 节点（Node）

节点是 ROS 中最基本的计算单元。一个机器人系统通常由大量节点组成：

```
┌─────────────────────────────────────────────────────┐
│                  机器人系统节点图                      │
│                                                     │
│  ┌──────────┐   /scan    ┌──────────┐              │
│  │ 雷达驱动  │──────────▶│  SLAM    │─── /map      │
│  └──────────┘            └──────────┘              │
│                                                     │
│  ┌──────────┐   /odom   ┌──────────┐              │
│  │ 里程计    │──────────▶│  定位    │              │
│  └──────────┘           └──────────┘              │
│                               │                     │
│                               ▼                     │
│                        ┌──────────┐                │
│                        │ 路径规划  │── /cmd_vel     │
│                        └─────┬────┘                │
│                              │                      │
│                              ▼                      │
│                        ┌──────────┐                │
│                        │ 运动控制  │                │
│                        └──────────┘                │
└─────────────────────────────────────────────────────┘
```

**节点的特点：**

- 每个节点是一个独立的进程
- 节点之间**松耦合**，不知道彼此的存在
- 节点通过话题、服务进行通信
- 一个节点可以是发布者、订阅者、或服务端/客户端
- 节点可以用不同语言编写（C++、Python 等）

### 话题与消息（Topic & Message）

**话题（Topic）**是节点之间传输数据的**通道名称**，**消息（Message）**是传输的**数据本身**。

```
Publisher ───── /scan (sensor_msgs/LaserScan) ─────▶ Subscriber
Publisher ───── /cmd_vel (geometry_msgs/Twist) ───▶ Subscriber
Publisher ───── /image_raw (sensor_msgs/Image) ───▶ Subscriber
```

- 话题名是一个字符串（如 `/scan`、`/cmd_vel`）
- 消息类型定义了数据的结构（如 `sensor_msgs/LaserScan`）
- 一个话题可以有多个发布者和多个订阅者
- 发布者和订阅者之间是**异步**的：发布者发完就走，不等订阅者

### 服务（Service）

服务用于**同步请求/响应**场景：

```
Client ─────── /spawn (turtlesim/Spawn) ──────▶ Server
Client ◀────────────────────────────────────── Server
```

- 服务名是一个字符串（如 `/spawn`）
- 服务类型定义了请求和响应的结构（如 `turtlesim/Spawn`）
- 一个服务只能有一个服务端
- 客户端调用服务时会**阻塞等待**响应

### 主控（ROS Master）

Master 是 ROS 1 中**节点发现**的中心：

1. 节点启动时向 Master 注册自己
2. 节点通过 Master 查找其他节点
3. 节点建立连接后，Master 退出数据通路

```
┌──────────────────────────────────────────────┐
│                ROS Master                     │
│                                              │
│  Publisher 说："我在 /scan 上发布数据"         │
│  Subscriber 问："谁在发布 /scan？"             │
│  Master 回答："Publisher 在 192.168.1.2:3344" │
│                                              │
│  → Publisher 和 Subscriber 建立直连           │
│  → Master 退出数据通路（P2P 直传）             │
└──────────────────────────────────────────────┘
```

> ⚠️ **重要理解**：Master 只负责"牵线搭桥"，不参与实际数据传输。所以 Master 挂掉后，已建立的连接仍能继续传数据，但新节点无法加入。

### 参数服务器（Parameter Server）

参数服务器是 Master 内置的**全局字典**，用于存储配置参数：

```
Node A ─── set("/max_speed", 1.0) ──▶ Parameter Server ◀── Node B
                                         │
                                         ▼
                              get("/max_speed") → 1.0
```

- 支持的数据类型：`int`、`float`、`bool`、`string`、`list`、`dict`
- 任何节点都可以读写
- 适合存储低频变化的配置参数
- 不适合高频数据（那是话题的用途）

### 可视化计算图

使用 `rqt_graph` 可以实时查看计算图：

```bash
# 启动 roscore 和若干节点后
rqt_graph
```

> 📖 `rqt_graph` 的详细使用将在第 5 章常用组件中讲解。

---

## 本节小结

| 层级 | 概念 | 作用 |
|------|------|------|
| **文件系统级** | Workspace | 项目工程容器 |
| | Package | 代码组织单元 |
| | Node | 独立计算进程 |
| | msg/srv/action | 自定义接口定义 |
| **计算图级** | Master | 节点注册与发现 |
| | Topic | 异步发布/订阅通道 |
| | Service | 同步请求/响应 |
| | Parameter | 全局键值存储 |
| **常用命令** | `roscd` / `rosls` / `rosed` | 快速跳转 / 列出 / 编辑 |
| | `rospack find` / `rospack list` | 查找包 / 列出所有包 |
| | `rosmsg show` / `rossrv show` | 查看消息 / 服务结构 |

---

## 理解检查

学完本节后，你应该能够：

- ✅ 描述 catkin 工作空间的四个目录（src / build / devel / install）各自的作用
- ✅ 解释 package.xml 和 CMakeLists.txt 的用途
- ✅ 区分文件系统级和计算图级的概念
- ✅ 使用 `roscd`、`rosls`、`rosed` 快速操作功能包
- ✅ 使用 `rosmsg show` 查看消息结构
- ✅ 理解 Master 在计算图中的角色（只负责发现，不参与数据传输）

---

## 🔧 动手练习

1. **探索 std_msgs 包**：
   ```bash
   roscd std_msgs
   rosls std_msgs
   rosmsg show std_msgs/Header
   rosmsg show std_msgs/String
   ```

2. **探索 turtlesim 包**：
   ```bash
   rospack find turtlesim
   rosls turtlesim
   rosrvs list | grep turtle
   rossrv show turtlesim/Spawn
   ```

3. **查看当前计算图**：启动 `roscore` 后运行 `rqt_graph`，观察有哪些节点。

---

## 下一节

第 1 章到这里就完成了。接下来进入 ROS 最核心的章节——通信机制：

→ **[第 2 章：ROS 通信机制](../chapter02/06-topic.md)**
