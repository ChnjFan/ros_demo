---
title: ROS 1 通信机制详解
nav_order: 3
---

# ROS 1 通信机制详解

> 本文深入剖析 ROS 1 的通信原理、协议栈、四种通信方式的实现细节，并给出 C++ / Python 双语言代码示例。

---

## 一、通信架构总览

ROS 1 的通信体系由三大角色和三层协议构成：

```
┌────────────────────────────────────────────────────────────────┐
│                      ROS 1 通信架构                            │
│                                                                │
│   ┌──────────┐   XMLRPC(注册/发现)    ┌──────────┐             │
│   │Publisher │◀───────────────────▶ │  Master  │             │
│   └────┬─────┘                        └────▲─────┘             │
│        │                                   │ XMLRPC            │
│        │  ① XMLRPC 协商连接                │                   │
│        ├──────────────────────────────▶  ┌┴─────────┐         │
│   ┌────▼─────┐   ② XMLRPC 协商连接        │Subscriber│         │
│   │Publisher │◀──────────────────────────┴──────────┘         │
│   │ (直连方) │                                                 │
│   └────┬─────┘                                                 │
│        │  ③ TCPROS 点对点数据传输（不再经过 Master）            │
│        └────────────────────────────────────▶ Subscriber      │
└────────────────────────────────────────────────────────────────┘
```

| 角色 | 职责 |
|------|------|
| **Master** | 中心注册/查找服务，负责节点发现与连接撮合，**不参与实际数据传输** |
| **Publisher / Server** | 数据/服务的提供方 |
| **Subscriber / Client** | 数据/服务的消费方 |

### 三层协议栈

| 协议 | 用途 | 特点 |
|------|------|------|
| **XMLRPC** | 节点 ↔ Master 的注册/查询；节点间的连接协商 | 基于 HTTP + XML，跨语言 |
| **TCPROS** | 话题与服务的数据传输 | 基于 TCP，可靠有序，ROS 1 默认 |
| **UDPROS** | 话题数据传输（可选） | 基于 UDP，低延迟但可能丢包，适合视频流 |

> **关键理解**：Master 只在"建立连接"阶段起作用，连接建立后数据完全点对点（P2P）直传。这就是为什么 Master 挂掉后**已建立的连接仍能继续传数据**，但新节点无法加入。

---

## 二、话题通信（Topic）— 发布/订阅模型

### 2.1 适用场景

**异步、单向、连续流式数据**：传感器数据（激光 `/scan`、图像 `/image_raw`）、控制指令（`/cmd_vel`）、里程计（`/odom`）。

### 2.2 连接建立完整流程（面试高频考点）

以 Publisher 发布 `/scan`、Subscriber 订阅 `/scan` 为例：

```
Publisher                     Master                      Subscriber
   │                            │                             │
   │ ①registerPublisher(/scan)  │                             │
   │ ─────────────────────────▶│ 记录：/scan 的发布者地址     │
   │                            │                             │
   │                            │ ②publisherUpdate(/scan)     │
   │                            │   "有人发布了 /scan"        │
   │                            │ ─────────────────────────▶ │
   │                            │                             │
   │ ③requestTopic(/scan)       │                             │
   │ ◀─────────────────────────│──────────────────────────── │
   │   (Subscriber 向 Publisher 请求建立连接，协商协议)        │
   │                            │                             │
   │ ④建立 TCPROS Socket 直连   │                             │
   │ ══════════════════════════════════════════════════════▶ │
   │        ⑤数据持续点对点传输（Master 不参与）               │
```

**详细步骤说明**：

1. **Publisher 注册**：Publisher 启动时通过 XMLRPC 调用 `registerPublisher`，向 Master 登记话题名、消息类型和自己 XMLRPC 服务器的 URI。
2. **Subscriber 注册 + Master 推送**：Subscriber 启动时调用 `registerSubscriber`；Master 发现该话题已有发布者，通过 `publisherUpdate` 把**所有发布者的 URI 列表**推送给 Subscriber。
3. **连接协商**：Subscriber 直接向 Publisher 的 XMLRPC 服务器发起 `requestTopic` 请求，双方协商传输协议（默认 TCPROS）。
4. **建立直连**：Publisher 监听一个 TCP 端口，把端口号告诉 Subscriber，双方建立 Socket 连接。
5. **数据传输**：此后消息通过 TCPROS 序列化传输，**Master 完全退出数据通路**。

### 2.3 消息序列化（TCPROS 数据格式）

每条消息在 TCPROS 上的传输格式：

```
┌────────────────────┬──────────────────────────────┐
│ 4 字节 Header 长度  │ Header 字段（caller_id 等）  │
├────────────────────┼──────────────────────────────┤
│ 4 字节 Body 长度    │ 序列化后的消息体（紧凑二进制）│
└────────────────────┴──────────────────────────────┘
```

- 消息按 `.msg` 定义的字段顺序**紧凑二进制序列化**（无字段名，只有数据），效率高
- 数组类型额外带 4 字节长度前缀

### 2.4 消息队列机制

roscpp 中 Publisher 和 Subscriber 各有一个缓冲队列，这是**丢消息问题的根源**：

```
Publisher                        Subscriber
┌──────────────┐   TCPROS   ┌─────────────────┐
│ publish()    │ ────────▶ │ 接收队列(默认∞) │ ──▶ 回调函数
│ 发布队列     │            │ 订阅队列        │    (逐条处理)
│ (默认0,直发) │            └─────────────────┘
└──────────────┘
```

| 队列 | 作用 | 溢出行为 |
|------|------|---------|
| 发布队列 `queue_size`（publish） | 缓存待发送消息 | 队满时**丢弃最旧**的消息 |
| 订阅队列 `queue_size`（subscribe） | 缓存收到但未处理的消息 | 队满时**丢弃最旧**的消息 |

> **最佳实践**：`publish` 的 `queue_size` 设为实际需求（如 10），**不要设为 0**（0 表示无限队列，可能导致内存暴涨）；`subscribe` 的队列大小决定回调来不及处理时丢多少数据。

### 2.5 C++ 实现：Publisher / Subscriber

**Publisher（`talker.cpp`）**：

```cpp
#include "ros/ros.h"
#include "std_msgs/String.h"

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    ros::init(argc, argv, "talker");                    // 1. 初始化节点
    ros::NodeHandle nh;                                  // 2. 创建句柄

    // 3. 创建发布者：话题 /chatter，消息类型 std_msgs/String，队列长度 10
    ros::Publisher pub = nh.advertise<std_msgs::String>("chatter", 10);

    ros::Rate rate(10);                                  // 发布频率 10Hz
    int count = 0;
    while (ros::ok())                                    // Ctrl+C 或 rosnode kill 时退出
    {
        std_msgs::String msg;
        msg.data = "hello count: " + std::to_string(count);
        pub.publish(msg);                                // 4. 发布消息
        ROS_INFO("发布: %s", msg.data.c_str());
        rate.sleep();                                    // 按频率休眠
        count++;
    }
    return 0;
}
```

**Subscriber（`listener.cpp`）**：

```cpp
#include "ros/ros.h"
#include "std_msgs/String.h"

// 回调函数：收到消息时自动触发
void chatterCallback(const std_msgs::String::ConstPtr &msg)
{
    ROS_INFO("收到: %s", msg->data.c_str());
}

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    ros::init(argc, argv, "listener");
    ros::NodeHandle nh;

    // 订阅 /chatter，队列长度 10，绑定回调函数
    ros::Subscriber sub = nh.subscribe("chatter", 10, chatterCallback);

    ros::spin();   // 阻塞式循环触发回调（详见 5.1 回调机制）
    return 0;
}
```

### 2.6 Python 实现：Publisher / Subscriber

**Publisher（`talker_p.py`）**：

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-
import rospy
from std_msgs.msg import String

if __name__ == "__main__":
    rospy.init_node("talker_p")

    pub = rospy.Publisher("chatter", String, queue_size=10)

    rate = rospy.Rate(10)   # 10Hz
    count = 0
    while not rospy.is_shutdown():
        msg = String()
        msg.data = "hello count: %d" % count
        pub.publish(msg)
        rospy.loginfo("发布: %s" % msg.data)
        rate.sleep()
        count += 1
```

**Subscriber（`listener_p.py`）**：

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-
import rospy
from std_msgs.msg import String

def chatterCallback(msg):
    rospy.loginfo("收到: %s" % msg.data)

if __name__ == "__main__":
    rospy.init_node("listener_p")
    sub = rospy.Subscriber("chatter", String, chatterCallback, queue_size=10)
    rospy.spin()   # 阻塞处理回调
```

---

## 三、服务通信（Service）— 请求/响应模型

### 3.1 适用场景

**同步、一问一答、低频调用**：触发一次性操作（拍照、清除日志）、查询状态（获取地图）、设置参数。

### 3.2 与话题的本质区别

| 维度 | Topic | Service |
|------|-------|---------|
| 通信方向 | 单向流 | 双向（请求→响应） |
| 同步性 | 异步（发完即走） | **同步**（客户端阻塞等待响应） |
| 数量关系 | N 发布者 : M 订阅者 | **1 服务端 : N 客户端** |
| 生命周期 | 持续存在 | 调用时建立，用完释放 |
| 典型例子 | `/scan` 雷达数据 | `/turtle1/teleport_absolute` 传送海龟 |

### 3.3 通信流程

```
Client                       Master                      Server
  │                            │                            │
  │                            │ ①registerService(/add)     │
  │                            │◀───────────────────────────│
  │                            │   记录：/add 的服务端 URI    │
  │ ②lookupService(/add)       │                            │
  │ ──────────────────────────▶│                            │
  │ ◀── 返回 Server URI ───────│                            │
  │                            │                            │
  │ ③XMLRPC 协商 → TCPROS 直连  │                            │
  │ ═══════════════════════════════════════════════════════▶│
  │        ④请求 → ──────────────────────────────────────▶  │
  │        ◀───────────────────────────── 响应               │
  │        (客户端阻塞等待响应，超时可配置)                     │
```

### 3.4 `.srv` 服务定义文件

服务由**请求（request）+ 响应（response）**两部分定义，用 `---` 分隔：

```
# 文件：srv/AddTwoInts.srv
int64 a        # ← 请求部分
int64 b
---            # ← 分隔线
int64 sum      # ← 响应部分
```

### 3.5 C++ 实现：Service Server / Client

**Server（`server.cpp`）**：

```cpp
#include "ros/ros.h"
#include "beginner_tutorials/AddTwoInts.h"   // 由 .srv 自动生成的头文件

// 处理函数：req 为请求，res 为响应
bool add(beginner_tutorials::AddTwoInts::Request &req,
         beginner_tutorials::AddTwoInts::Response &res)
{
    res.sum = req.a + req.b;
    ROS_INFO("请求: %ld + %ld = %ld", (long)req.a, (long)req.b, (long)res.sum);
    return true;   // 返回 true 表示处理成功
}

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    ros::init(argc, argv, "add_server");
    ros::NodeHandle nh;

    // 注册服务：服务名 /add_two_ints，处理函数 add
    ros::ServiceServer server = nh.advertiseService("add_two_ints", add);
    ROS_INFO("服务已就绪...");

    ros::spin();   // 阻塞等待请求
    return 0;
}
```

**Client（`client.cpp`）**：

```cpp
#include "ros/ros.h"
#include "beginner_tutorials/AddTwoInts.h"

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    ros::init(argc, argv, "add_client");
    if (argc != 3) {
        ROS_ERROR("用法: rosrun beginner_tutorials add_client X Y");
        return 1;
    }

    ros::NodeHandle nh;
    // 创建客户端：服务名 /add_two_ints
    ros::ServiceClient client = nh.serviceClient<beginner_tutorials::AddTwoInts>("add_two_ints");

    // 等待服务上线（阻塞，最多等 3 秒）
    client.waitForExistence(ros::Duration(3.0));

    beginner_tutorials::AddTwoInts srv;
    srv.request.a = atoll(argv[1]);
    srv.request.b = atoll(argv[2]);

    if (client.call(srv))   // 发起同步调用，阻塞直到响应
    {
        ROS_INFO("响应: sum = %ld", (long)srv.response.sum);
    }
    else
    {
        ROS_ERROR("服务调用失败");
        return 1;
    }
    return 0;
}
```

### 3.6 Python 实现

**Server（`server_p.py`）**：

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-
import rospy
from beginner_tutorials.srv import AddTwoInts, AddTwoIntsResponse

def add(req):
    res = AddTwoIntsResponse()
    res.sum = req.a + req.b
    rospy.loginfo("请求: %d + %d = %d" % (req.a, req.b, res.sum))
    return res

if __name__ == "__main__":
    rospy.init_node("add_server_p")
    server = rospy.Service("add_two_ints", AddTwoInts, add)
    rospy.loginfo("服务已就绪...")
    rospy.spin()
```

**Client（`client_p.py`）**：

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import rospy
from beginner_tutorials.srv import AddTwoInts

if __name__ == "__main__":
    if len(sys.argv) != 3:
        rospy.logerr("用法: rosrun beginner_tutorials client_p.py X Y")
        sys.exit(1)

    rospy.init_node("add_client_p")
    rospy.wait_for_service("add_two_ints", timeout=3.0)   # 等待服务上线

    try:
        client = rospy.ServiceProxy("add_two_ints", AddTwoInts)
        resp = client(int(sys.argv[1]), int(sys.argv[2]))   # 同步调用
        rospy.loginfo("响应: sum = %d" % resp.sum)
    except rospy.ServiceException as e:
        rospy.logerr("服务调用失败: %s" % e)
```

---

## 四、参数服务器（Parameter Server）

### 4.1 原理

参数服务器是 **Master 内置的全局字典**（键值对存储），通过 XMLRPC 访问：

```
Node A ──set("robot_name", "turtlesim")──▶ Master 内置字典 ──get──▶ Node B
```

- 数据类型支持：`int`、`float`、`bool`、`string`、`list`、`dict`（YAML 格式）
- **全局共享**：任何节点都可读写，无归属限制
- **非实时**：走 XMLRPC，速度慢，不适合高频数据

### 4.2 C++ 操作参数

```cpp
#include "ros/ros.h"

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    ros::init(argc, argv, "param_demo");
    ros::NodeHandle nh;

    // ── 写参数 ──
    nh.setParam("type", "小海龟");        // string
    nh.setParam("radius", 0.5);           // double
    nh.setParam("wheels", 4);             // int

    // ── 读参数（带默认值，推荐）──
    std::string type;
    double radius;
    nh.param<std::string>("type", type, "默认类型");
    nh.param("radius", radius, 1.0);

    // ── 读参数（不带默认值，参数不存在则返回 false）──
    int wheels;
    if (nh.getParam("wheels", wheels))
        ROS_INFO("wheels = %d", wheels);

    // ── 删除参数 ──
    nh.deleteParam("radius");

    return 0;
}
```

### 4.3 Python 操作参数

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-
import rospy

if __name__ == "__main__":
    rospy.init_node("param_demo_p")
    rospy.set_param("type_p", "小海龟")        # 写
    rospy.set_param("list_p", [1, 2, 3])      # 写列表

    type_p = rospy.get_param("type_p", "默认")  # 读（带默认值）
    rospy.loginfo("type_p = %s" % type_p)

    rospy.delete_param("type_p")                # 删除
```

### 4.4 命令行 / launch 文件操作参数

```bash
rosparam set /background_r 255     # 设置
rosparam get /type                 # 读取
rosparam load params.yaml          # 从 YAML 批量加载
rosparam dump params.yaml          # 导出全部参数
```

```xml
<!-- launch 文件中设置参数 -->
<launch>
    <!-- 全局参数 -->
    <param name="robot_name" value="turtlesim"/>

    <!-- 私有参数（属于节点，自动加节点名前缀）-->
    <node pkg="my_pkg" type="my_node" name="demo">
        <param name="speed" value="1.5"/>   <!-- 实际是 /demo/speed -->
    </node>

    <!-- 从 YAML 文件批量加载 -->
    <rosparam file="$(find my_pkg)/config/params.yaml" command="load"/>
</launch>
```

---

## 五、回调与消息处理机制（roscpp 核心）

### 5.1 spin 家族 — 驱动回调执行

**订阅回调不会自动执行**，必须由 spin 系列函数驱动：

| 函数 | 行为 | 适用场景 |
|------|------|---------|
| `ros::spin()` | **阻塞**循环处理回调，直到节点关闭 | 纯订阅节点（如 listener） |
| `ros::spinOnce()` | 处理一次积压的回调后立即返回 | 与 `while` 循环配合，节点既要订阅又要做别的事 |
| `ros::AsyncSpinner` | 后台线程持续处理回调，**不阻塞主线程** | 主线程有其他阻塞任务（如 UI） |
| `ros::MultiThreadedSpinner` | 多线程处理回调 | 回调耗时长，需要并发 |

```cpp
// 方式1：spinOnce + 循环（注意与 ros::Rate 配合）
ros::Rate rate(10);
while (ros::ok())
{
    ros::spinOnce();   // 处理完当前积压回调
    // ... 其他逻辑（如发布、计算）
    rate.sleep();
}

// 方式2：异步 Spinner（不阻塞主线程）
ros::AsyncSpinner spinner(2);   // 2 个线程
spinner.start();
// ... 主线程自由执行其他任务
ros::waitForShutdown();
```

> **常见坑**：`while` 循环里忘记调用 `spinOnce()`，导致回调永远不触发；`spin()` 和 `spinOnce()` 混用在同一循环导致逻辑混乱。

### 5.2 rospy 的回调处理

rospy 的回调在**独立线程池**中自动执行，`rospy.spin()` 只是防止主线程退出：

```python
rospy.spin()   # rospy 中回调已自动处理，spin 仅阻塞主线程
```

---

## 六、Action 通信 — 带反馈的长任务

### 6.1 为什么需要 Action

服务是同步阻塞的——导航到 10 米外的目标点需要 30 秒，期间客户端只能干等，且无法获取进度、无法取消。Action 弥补了这些缺陷：

```
Action Client                          Action Server
     │────── goal（目标）──────────────▶│
     │◀───── feedback（进度反馈，持续）───│   ← 底层：一对 Topic
     │◀───── result（最终结果）──────────│
     │────── cancel（取消）────────────▶│   ← 底层：取消 Topic
     │◀──────── status（状态）──────────│   ← 底层：状态 Topic
```

### 6.2 `.action` 定义文件

```
# 文件：action/MoveDistance.action
float32 target_distance       # ← 目标（goal）
---                           # ← 分隔线 1
float32 current_distance      # ← 反馈（feedback）
---                           # ← 分隔线 2
bool success                  # ← 结果（result）
float32 final_distance
```

### 6.3 实现方式

Action 基于 `actionlib` 库，底层由 **5 个 Topic** 组合实现：

| Topic | 方向 | 作用 |
|-------|------|------|
| `/goal` | Client → Server | 下发目标 |
| `/feedback` | Server → Client | 持续进度反馈 |
| `/result` | Server → Client | 最终结果 |
| `/status` | Server → Client | 状态机（PENDING/ACTIVE/SUCCEEDED...） |
| `/cancel` | Client → Server | 取消请求 |

> 由于底层是 Topic，Action 是**异步**的：客户端下发目标后可继续做其他事，通过回调接收反馈与结果。

---

## 七、自定义消息与服务（工程实战）

### 7.1 自定义 `.msg`

```
# 文件：msg/Person.msg
string name
uint8  age
float64 height
```

### 7.2 配置 `package.xml`

```xml
<build_depend>message_generation</build_depend>
<exec_depend>message_runtime</exec_depend>
```

### 7.3 配置 `CMakeLists.txt`

```cmake
# ① find_package 添加 message_generation
find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
  message_generation
)

# ② 添加消息文件
add_message_files(
  FILES
  Person.msg
)

# ③ 声明消息依赖
generate_messages(
  DEPENDENCIES
  std_msgs
)

# ④ catkin_package 添加 message_runtime
catkin_package(
  CATKIN_DEPENDS roscpp rospy std_msgs message_runtime
)
```

### 7.4 编译后使用

```bash
catkin_make
# 生成的头文件位于 devel/include/<pkg>/Person.h

# C++ 使用
#include "hello_world/Person.h"
hello_world::Person p;
p.name = "张三"; p.age = 20; p.height = 1.75;

# Python 使用
from hello_world.msg import Person
p = Person()
p.name = "张三"

# 命令行查看
rosmsg show hello_world/Person
```

> 自定义 `.srv` / `.action` 的流程完全相同，只是文件放 `srv/` / `action/` 目录，CMake 用 `add_service_files` / `add_action_files`。

---

## 八、命名空间与重映射（通信寻址机制）

### 8.1 全局名、相对名、私有名

```
/namespace/topic     ← 全局名（以 / 开头，绝对路径）
topic                ← 相对名（相对于节点当前命名空间）
~param               ← 私有名（自动加 /node_name/ 前缀）
```

### 8.2 重映射（remap）— 不改代码换话题

```bash
# 把节点的 /chatter 重映射到 /talker_data，代码零修改
rosrun hello_world listener /chatter:=/talker_data
```

```xml
<!-- launch 文件中重映射 -->
<node pkg="hello_world" type="listener" name="listener">
    <remap from="chatter" to="talker_data"/>
</node>
```

**应用场景**：同一套代码控制多台机器人、更换传感器话题名、模块化复用。

### 8.3 命名空间（namespace）— 隔离同名资源

```xml
<group ns="robot1">
    <node pkg="hello_world" type="talker" name="pub"/>   <!-- /robot1/pub -->
</group>
<group ns="robot2">
    <node pkg="hello_world" type="talker" name="pub"/>   <!-- /robot2/pub -->
</group>
```

---

## 九、四种通信机制选型总结

```
                     需要通信
                        │
            ┌───────────┴───────────┐
            │                       │
        数据是持续流？            数据是请求-响应？
            │                       │
            ▼                       ▼
        Topic                  需要进度反馈/取消？
     （传感器、控制指令）           │
                          ┌───────┴────────┐
                          │ 否              │ 是
                          ▼                ▼
                      Service           Action
                    （查询、设置）     （导航、抓取）

    配置数据（低频读写）→ Parameter Server
```

| 判断依据 | 选择 |
|---------|------|
| 持续单向流数据 | **Topic** |
| 一问一答、同步等待 | **Service** |
| 长任务、需进度反馈、可取消 | **Action** |
| 全局配置项、低频读写 | **Parameter** |

---

## 十、常见通信问题排查

| 现象 | 可能原因 | 排查命令 |
|------|---------|---------|
| 订阅者收不到数据 | 话题名不一致（重映射/命名空间） | `rostopic list` 对比两边 |
| 回调不触发 | 忘记 `spin()` / `spinOnce()` | 检查代码 |
| 收到的数据有延迟堆积 | 订阅队列太大、回调处理慢 | `rostopic hz` 查频率 |
| 消息类型不匹配 | 两端 `.msg` 定义不同 | `rostopic type` + `rosmsg show` |
| Master 连接失败 | `ROS_MASTER_URI` 配置错误 | `echo $ROS_MASTER_URI` |
| 多机通信失败 | `ROS_HOSTNAME`/`ROS_IP` 不对 | `echo $ROS_HOSTNAME` |
| 发布频率上不去 | 发布队列满、`rate.sleep()` 位置错 | `rostopic hz`、检查代码 |

---

## 参考

- [ROS 1 官方文档 — Concepts](http://wiki.ros.org/ROS/Concepts)
- [roscpp 发布/订阅教程](http://wiki.ros.org/ROS/Tutorials/WritingPublisherSubscriber%28c%2B%2B%29)
- [actionlib 官方文档](http://wiki.ros.org/actionlib)
- 相关文档：[ROS 架构详解](arch.md) | [ROS 常用命令速查](ros_cmd.md)
