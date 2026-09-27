# ROS 架构详解

## 一、整体架构概览

ROS 本质是一个**分布式计算框架**。它的设计哲学是：

> **将机器人的复杂功能拆分成大量独立的"节点"，节点之间通过标准化的"通信接口"进行协作。**

```
┌─────────────────────────────────────────────────────────────────┐
│                        ROS 架构全景图                            │
│                                                                 │
│  ┌──────────┐     ┌──────────┐     ┌──────────┐                │
│  │ 感知层    │     │ 决策层    │     │ 执行层    │                │
│  │ 摄像头节点 │────▶│ SLAM节点  │────▶│ 电机节点  │                │
│  │ 雷达节点   │     │ 规划节点   │     │ 舵机节点  │                │
│  └────┬─────┘     └────┬─────┘     └────┬─────┘                │
│       │                │                │                       │
│       └────────────────┼────────────────┘                       │
│                        │                                        │
│              ┌─────────▼─────────┐                              │
│              │   通信中间件层      │                              │
│              │  Topic/Service/    │                              │
│              │  Action/Parameter  │                              │
│              └─────────┬─────────┘                              │
│                        │                                        │
│              ┌─────────▼─────────┐                              │
│              │   Master（ROS 1）  │                              │
│              │   DDS（ROS 2）     │                              │
│              └───────────────────┘                              │
└─────────────────────────────────────────────────────────────────┘
```

---

## 二、核心概念逐层详解

### 2.1 节点（Node）

**节点是 ROS 中最基本的计算单元**，每个节点是一个独立的进程，负责做一件事。

以 `hello_world` 包为例，`start_turtle.launch` 启动了 **两个节点**：

```xml
<launch>
    <!-- 节点 1：C++ 编写的 hello_node -->
    <node pkg="hello_world" type="hello_node" name="hello_cpp" output="screen"/>
    <!-- 节点 2：Python 编写的 helloworld_p.py -->
    <node pkg="hello_world" type="helloworld_p.py" name="hello_python" output="screen"/>
</launch>
```

| 属性 | 含义 |
|------|------|
| `pkg="hello_world"` | 节点所属的功能包 |
| `type="hello_node"` | 可执行文件名（CMake 中 `add_executable` 定义的目标名） |
| `name="hello_cpp"` | 节点在 ROS 中的运行时名称（唯一标识） |
| `output="screen"` | 日志输出到终端 |

**节点内部代码对比：**

**C++ 节点** (`helloworld_c.cpp`)：
```cpp
ros::init(argc, argv, "hello");   // 1. 初始化节点，注册名称
ros::NodeHandle n;                 // 2. 创建句柄（管理通信接口）
ROS_INFO("hello world!测试");      // 3. 日志输出（替代 printf）
```

**Python 节点** (`helloworld_p.py`)：
```python
rospy.init_node("hello_p")                          # 初始化节点
rospy.loginfo("hello world! by python")             # 日志输出
```

> **关键设计原则**：节点之间**零耦合**，它们不知道对方的存在，只通过通信机制交互。

---

### 2.2 通信机制（ROS 的灵魂）

ROS 提供了 **四种通信方式**，覆盖所有机器人应用场景：

#### (1) Topic（话题）— 发布/订阅模型

```
  Publisher ──────/scan────────▶  Subscriber1
      │           /scan          Subscriber2
      └──────────/scan────────▶  Subscriber3
```

- **异步、单向、流式数据**
- 一个话题可以有**多个发布者和订阅者**
- 典型应用：传感器数据流（激光雷达、摄像头、里程计）

**工作流程：**
```
Publisher 注册话题 ──▶ Master 记录 ──▶ Subscriber 查询话题
                                        │
                                        ▼
                              Publisher 与 Subscriber 建立直连
                                        │
                                        ▼
                              TCPROS / UDPROS 传输数据
```

#### (2) Service（服务）— 请求/响应模型

```
  Client ────请求────▶  Service Server
         ◀──响应────
```

- **同步、双向、一问一答**
- 一个服务只能有一个服务器
- 典型应用：设置参数、切换模式、一次性查询

#### (3) Action（动作）— 带反馈的长任务模型

```
  Client ────Goal────▶  Action Server
         ◀──Feedback───  (持续反馈)
         ◀──Result────  (最终结果)
         ───Cancel───▶  (可取消)
```

- **异步、带进度反馈、可取消**
- 底层由 Topic + Service 组合实现
- 典型应用：导航到目标点、机械臂抓取、建图

#### (4) Parameter（参数）— 全局键值存储

```
  Node A ──写入──▶  Parameter Server  ◀──读取── Node B
```

- 集中管理配置参数（如机器人尺寸、PID 系数）
- 支持动态更新（dynamic_reconfigure）

#### 四种通信方式对比

| 特性 | Topic | Service | Action | Parameter |
|------|-------|---------|--------|-----------|
| **模式** | 发布/订阅 | 请求/响应 | 目标/反馈/结果 | 键值存储 |
| **同步性** | 异步 | 同步 | 异步 | 同步 |
| **方向** | 单向 | 双向 | 双向（持续） | 读写 |
| **多对多** | ✅ 1:N, N:M | ❌ 1:1 | ❌ 1:1 | N:M |
| **典型场景** | 传感器流 | 开关/查询 | 导航/操作 | 配置参数 |
| **底层协议** | TCPROS/UDPROS | TCPROS | Topic+Service | XMLRPC |

---

### 2.3 ROS Master（主控）— 节点发现中心

```
┌─────────────────────────────────────────────┐
│                ROS Master                    │
│  ┌─────────────────────────────────────┐    │
│  │  注册表                             │    │
│  │  ├── 话题列表：/scan → [pub_addr]   │    │
│  │  ├── 服务列表：/set_map → [srv_addr]│    │
│  │  ├── 节点列表：/camera_node         │    │
│  │  └── 参数列表：/max_speed = 1.0     │    │
│  └─────────────────────────────────────┘    │
│                                             │
│  Publisher 说："我在 /scan 上发数据"          │
│  Subscriber 问："谁在发 /scan？"              │
│  Master 回答："Publisher 在 192.168.1.2:3344" │
│  → 两者建立直连（P2P），Master 退出           │
└─────────────────────────────────────────────┘
```

**重要理解**：Master 只负责**初始发现**，实际数据传输是**点对点直连**，不经过 Master。

> **注意**：ROS 2 已**移除 Master**，改用 DDS（Data Distribution Service）实现去中心化发现。

---

### 2.4 功能包（Package）— 代码组织单元

`hello_world` 包结构：

```
hello_world/                    ← 功能包
├── package.xml                 ← 包元信息（依赖、版本、描述）
├── CMakeLists.txt              ← 编译规则
├── src/
│   └── helloworld_c.cpp        ← C++ 节点源码
├── scripts/
│   └── helloworld_p.py         ← Python 节点源码
├── launch/
│   └── start_turtle.launch     ← 启动配置文件
├── msg/                        ← 自定义消息（本例未使用）
├── srv/                        ← 自定义服务（本例未使用）
└── action/                     ← 自定义动作（本例未使用）
```

**`package.xml`** 关键内容：

```xml
<buildtool_depend>catkin</buildtool_depend>   <!-- 构建工具 -->
<build_depend>roscpp</build_depend>            <!-- C++ 客户端库 -->
<build_depend>rospy</build_depend>             <!-- Python 客户端库 -->
<build_depend>std_msgs</build_depend>          <!-- 标准消息类型 -->
```

**`CMakeLists.txt`** 关键内容：

```cmake
find_package(catkin REQUIRED COMPONENTS roscpp rospy std_msgs)
add_executable(hello_node src/helloworld_c.cpp)        # 编译 C++ 节点
target_link_libraries(hello_node ${catkin_LIBRARIES})
catkin_install_python(PROGRAMS scripts/helloworld_p.py) # 安装 Python 节点
```

---

### 2.5 Launch 文件 — 系统编排器

**`start_turtle.launch`** 是系统的**启动入口**：

```xml
<launch>
    <node pkg="hello_world" type="hello_node" name="hello_cpp" output="screen"/>
    <node pkg="hello_world" type="helloworld_p.py" name="hello_python" output="screen"/>
</launch>
```

**启动命令：**
```bash
roslaunch hello_world start_turtle.launch
```

**Launch 支持的高级功能：**

```xml
<launch>
    <!-- 嵌套其他 launch 文件 -->
    <include file="$(find other_pkg)/launch/other.launch"/>

    <!-- 设置参数 -->
    <param name="max_speed" value="1.0"/>

    <!-- 启动命名空间（防止重名冲突） -->
    <group ns="robot1">
        <node pkg="hello_world" type="hello_node" name="talker"/>
    </group>
    <group ns="robot2">
        <node pkg="hello_world" type="hello_node" name="talker"/>
    </group>

    <!-- 条件启动 -->
    <arg name="use_sim" default="true"/>
    <node if="$(arg use_sim)" pkg="turtlesim" type="turtlesim_node" name="sim"/>
</launch>
```

---

## 三、文件系统级视图（Workspace）

```
ros_demo/                           ← catkin 工作空间
├── src/                            ← 源代码目录
│   ├── CMakeLists.txt              ← 顶层 CMake（catkin 生成）
│   └── hello_world/                ← 功能包
│       ├── package.xml
│       ├── CMakeLists.txt
│       ├── src/                    ← C++ 源码
│       ├── scripts/                ← Python 脚本
│       ├── launch/                 ← launch 文件
│       ├── msg/                    ← 自定义消息定义
│       ├── srv/                    ← 自定义服务定义
│       └── action/                 ← 自定义动作定义
├── build/                          ← 编译中间产物
└── devel/                          ← 开发环境
    ├── setup.bash                  ← 环境变量配置
    └── share/hello_world/cmake/    ← 包配置文件
```

**构建流程：**
```bash
catkin_make                    # 编译所有包
source devel/setup.bash        # 加载环境变量
roslaunch hello_world start_turtle.launch   # 启动系统
```

---

## 四、ROS 1 vs ROS 2 架构对比

| 维度 | ROS 1 | ROS 2 |
|------|-------|-------|
| **节点发现** | 中心化（Master） | 去中心化（DDS 组播发现） |
| **通信中间件** | 自研 TCPROS/UDPROS | 标准 DDS |
| **实时性** | ❌ 不支持 | ✅ 支持（实时操作系统） |
| **多机器人** | 困难（多 Master 复杂） | 原生支持 |
| **生命周期管理** | 无 | 有（Active/Inactive/Shutdown） |
| **QoS 策略** | 无 | 可靠/尽力/截止期等 |
| **构建系统** | catkin / catkin_make | ament / colcon |
| **Python 支持** | Python 2 | Python 3 |
| **维护状态** | 仅维护到 2025 (Noetic) | 持续发展 |

---

## 五、ROS 在机器人系统中的典型架构

一个完整机器人系统的 ROS 节点图：

```
┌─────────────────────────────────────────────────────────────────────┐
│                        自主移动机器人 ROS 架构                        │
│                                                                     │
│  ┌──────────┐    /scan    ┌──────────┐   /map    ┌──────────┐      │
│  │ 激光雷达  │────────────▶│  SLAM    │──────────▶│  地图    │      │
│  │ 驱动节点  │             │  节点    │           │  节点    │      │
│  └──────────┘             └──────────┘           └────┬─────┘      │
│                                                        │            │
│  ┌──────────┐   /odom    ┌──────────┐                 │            │
│  │ IMU/里程计│───────────▶│  定位    │                 │            │
│  │ 驱动节点  │            │  节点    │                 │            │
│  └──────────┘            └──────────┘                 │            │
│       │                                              │            │
│       │         ┌──────────┐    /plan                │            │
│       └────────▶│  路径规划 │◀───────────────────────┘            │
│                 │  节点    │                                       │
│       ┌────────▶│          │───▶ /cmd_vel                         │
│       │         └──────────┘      │                                │
│       │                           ▼                                │
│  ┌────┴─────┐              ┌──────────┐                           │
│  │  避障    │              │  运动控制 │                           │
│  │  节点    │              │  节点    │                           │
│  └──────────┘              └────┬─────┘                           │
│                                  │                                  │
│                                  ▼                                  │
│                            ┌──────────┐                             │
│                            │  电机    │                             │
│                            │  驱动    │                             │
│                            └──────────┘                             │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 六、总结

| 层级 | 组件 | 作用 |
|------|------|------|
| **应用层** | Node（节点） | 单一职责的计算进程 |
| **通信层** | Topic/Service/Action/Parameter | 节点间数据交换 |
| **发现层** | Master (ROS 1) / DDS (ROS 2) | 节点互相发现 |
| **组织层** | Package（功能包） | 代码组织单元 |
| **编排层** | Launch 文件 | 批量启动与配置 |
| **构建层** | catkin / ament | 编译与安装 |
| **工作空间** | Workspace | 整体工程容器 |

ROS 的核心价值在于：**标准化 + 模块化 + 分布式**。任何人都可以发布一个 ROS 节点（如相机驱动），其他人无需修改代码即可通过标准话题/服务使用它，这就是 ROS 生态强大的原因。
