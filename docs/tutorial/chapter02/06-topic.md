# 2.1 话题通信

> 本节学习目标：深入理解话题通信的发布/订阅模型，掌握 C++ 和 Python 实现发布者与订阅者节点的方法，以及如何定义和使用自定义消息类型。

---

## 2.1.1 话题通信理论模型

话题通信是 ROS 中**最常用、最基础**的通信方式，采用**发布/订阅（Publish/Subscribe）**模型。

### 模型图解

```
┌─────────────────────────────────────────────────────────────────┐
│                     话题通信模型                                  │
│                                                                 │
│   ┌──────────────┐                      ┌──────────────┐        │
│   │  Publisher   │                      │  Subscriber  │        │
│   │  (发布者)     │                      │  (订阅者)     │        │
│   │              │                      │              │        │
│   │  "我发布数据  │      /chatter       │  "我要接收    │        │
│   │   到/chatter"│ ═══════════════════▶ │   /chatter"  │        │
│   └──────┬───────┘                      └──────┬───────┘        │
│          │                                     │                │
│          ▼                                     ▼                │
│   ┌──────────────┐                      ┌──────────────┐        │
│   │  发布队列      │      TCPROS        │  订阅队列       │        │
│   │  (queue_size)│ ═══════════════════▶│  (queue_size) │        │
│   └──────────────┘                      └──────────────┘        │
│                                                                 │
│   特点：异步、单向、流式、多对多                                   │
└─────────────────────────────────────────────────────────────────┘
```

### 核心特点

| 特点 | 说明 |
|------|------|
| **异步** | 发布者发完消息就继续执行，不等订阅者接收完毕 |
| **单向** | 数据只从发布者流向订阅者，没有反向通道 |
| **流式** | 消息是一条一条连续发送的，适合传感器数据流 |
| **多对多** | 一个话题可以有多个发布者和多个订阅者 |

### 适用场景

话题通信适合以下场景：

- **传感器数据**：激光雷达 `/scan`、摄像头 `/image_raw`、里程计 `/odom`
- **控制指令**：速度指令 `/cmd_vel`
- **状态信息**：机器人位姿、电池电量
- **连续流数据**：任何需要持续单向传输的数据

### 通信流程

一次完整的话题通信经历 5 个步骤：

```
Publisher                     Master                      Subscriber
   │                            │                             │
   │ ① registerPublisher(/chatter)                             │
   │ ─────────────────────────▶│ 登记：/chatter 的发布者地址   │
   │                            │                             │
   │                            ② publisherUpdate(/chatter)   │
   │                            │ "有人发布了 /chatter"        │
   │                            │ ─────────────────────────▶ │
   │                            │                             │
   │ ③ requestTopic(/chatter)   │                             │
   │ ◀─────────────────────────│─────────────────────────────│
   │   (Subscriber 请求建立连接)  │                             │
   │                            │                             │
   │ ④ 建立 TCPROS Socket 直连   │                             │
   │ ═══════════════════════════════════════════════════════▶ │
   │                            │                             │
   │ ⑤ 数据持续点对点传输（Master 不参与）                       │
   │ ═══════════════════════════════════════════════════════▶ │
```

1. **Publisher 注册**：发布者启动时向 Master 注册话题名、消息类型和自己的地址
2. **Subscriber 注册 + 推送**：订阅者注册后，Master 将发布者地址推送给它
3. **连接协商**：订阅者直接向发布者请求建立连接，协商传输协议
4. **建立直连**：双方建立 TCPROS Socket 连接
5. **数据传输**：消息通过 TCPROS 点对点传输，Master 完全退出数据通路

> 💡 **关键理解**：Master 只在建立连接时起作用，实际数据传输是发布者与订阅者之间的**点对点直连**。这就是为什么 Master 挂掉后，已建立的连接仍能继续工作。

---

## 2.1.2 话题通信基本操作 A：C++ 实现

我们将实现一个经典的"说话者-监听者"（Talker-Listener）示例：Talker 不断发布字符串消息，Listener 接收并打印。

### 创建功能包

```bash
cd ~/ros_demo/src
catkin_create_pkg hello_world roscpp rospy std_msgs
cd ~/ros_demo
```

### 编写发布者节点（Talker）

在 `hello_world/src/` 下创建 `talker.cpp`：

```cpp
#include "ros/ros.h"
#include "std_msgs/String.h"
#include <sstream>

int main(int argc, char *argv[])
{
    // 1. 初始化节点，注册名称为 "talker"
    ros::init(argc, argv, "talker");

    // 2. 创建节点句柄
    ros::NodeHandle nh;

    // 3. 创建发布者
    //    参数：话题名 "/chatter"，消息类型 std_msgs::String，队列长度 10
    ros::Publisher pub = nh.advertise<std_msgs::String>("/chatter", 10);

    // 4. 设置发布频率为 10Hz
    ros::Rate rate(10);

    int count = 0;
    while (ros::ok())  // 节点正常运行时循环
    {
        // 5. 组装消息
        std_msgs::String msg;
        std::stringstream ss;
        ss << "hello world " << count;
        msg.data = ss.str();

        // 6. 发布消息
        pub.publish(msg);

        // 7. 打印日志
        ROS_INFO("发布: %s", msg.data.c_str());

        // 8. 按频率休眠
        rate.sleep();
        count++;
    }

    return 0;
;
}
```

**代码逐行解释：**

第 10 行 `ros::init(argc, argv, "talker")` 初始化 ROS 节点，注册名称为 `"talker"`。注意这个名称会出现在 `rosnode list` 的输出中。

第 13 行 `ros::NodeHandle nh` 创建节点句柄，它是节点与 ROS 系统交互的接口。后续创建发布者、订阅者、调用服务都要通过它。

第 17 行 `nh.advertise<std_msgs::String>("/chatter", 10)` 创建一个发布者对象。模板参数 `std_msgs::String` 指定消息类型，第一个参数 `"/chatter"` 是话题名，第二个参数 `10` 是发布队列长度。队列长度决定了在消息发送来不及的情况下最多缓存多少条消息，超过则丢弃最旧的消息。

第 20 行 `ros::Rate rate(10)` 创建一个频率控制对象，设定为 10Hz（每秒 10 次）。配合 `rate.sleep()` 使用，可以精确控制循环频率。

第 23 行 `while (ros::ok())` 是 ROS 节点的标准主循环条件。当用户按下 Ctrl+C、调用 `rosnode kill`、或 `ros::shutdown()` 被调用时，`ros::ok()` 返回 false，循环退出。

第 34 行 `pub.publish(msg)` 将消息发布到 `/chatter` 话题。所有订阅了该话题的订阅者都会收到这条消息。

第 37 行 `ROS_INFO("发布: %s", msg.data.c_str())` 输出日志，方便调试。

第 40 行 `rate.sleep()` 按设定的频率休眠，确保循环以 10Hz 运行。它会自动计算本次循环已消耗的时间，只休眠剩余时间。

### 编写订阅者节点（Listener）

在 `hello_world/src/` 下创建 `listener.cpp`：

```cpp
#include "ros/ros.h"
#include "std_msgs/String.h"

// 回调函数：收到消息时自动触发
void chatterCallback(const std_msgs::String::ConstPtr& msg)
{
    ROS_INFO("收到: [%s]", msg->data.c_str());
}

int main(int argc, char *argv[])
{
    // 1. 初始化节点，注册名称为 "listener"
    ros::init(argc, argv, "listener");

    // 2. 创建节点句柄
    ros::NodeHandle nh;

    // 3. 创建订阅者
    //    参数：话题名 "/chatter"，队列长度 10，回调函数 chatterCallback
    ros::Subscriber sub = nh.subscribe("/chatter", 10, chatterCallback);

    // 4. 阻塞式循环，等待并处理回调
    ros::spin();

    return 0;
}
```

**代码逐行解释：**

第 4-8 行定义了回调函数 `chatterCallback`，每当订阅者收到一条消息，ROS 就会自动调用这个函数。参数 `const std_msgs::String::ConstPtr& msg` 是指向常量消息的智能指针，使用引用传递避免拷贝，ConstPtr 保证不会修改消息内容。

第 18 行 `nh.subscribe("/chatter", 10, chatterCallback)` 创建订阅者。模板参数自动推断消息类型，第一个参数是话题名，第二个参数是订阅队列长度，第三个参数是回调函数的指针。

第 21 行 `ros::spin()` 进入阻塞循环，不断检查是否有新消息到达，有则调用回调函数。这个函数不会返回，直到节点被关闭。

> ⚠️ **常见错误**：忘记调用 `ros::spin()` 或 `ros::spinOnce()`，导致回调函数永远不会被触发，订阅者形同虚设。

### 配置编译规则

编辑 `hello_world/CMakeLists.txt`，在文件末尾添加：

```cmake
add_executable(talker src/talker.cpp)
target_link_libraries(talker ${catkin_LIBRARIES})

add_executable(listener src/listener.cpp)
target_link_libraries(listener ${catkin_LIBRARIES})
```

### 编译与运行

```bash
# 编译
cd ~/ros_demo
catkin_make

# 加载环境变量
source devel/setup.bash

# 终端 1：启动 roscore
roscore

# 终端 2：运行发布者
rosrun hello_world talker

# 终端 3：运行订阅者
rosrun hello_world listener
```

**运行效果：**

Talker 终端输出：

```
[ INFO] [1695800000.000000000]: 发布: hello world 0
[ INFO] [1695800000.100000000]: 发布: hello world 1
[ INFO] [1695800000.200000000]: 发布: hello world 2
...
```

Listener 终端输出：

```
[ INFO] [1695800000.000000000]: 收到: [hello world 0]
[ INFO] [1695800000.100000000]: 收到: [hello world 1]
[ INFO] [1695800000.200000000]: 收到: [hello world 2]
...
```

---

## 2.1.3 话题通信基本操作 B：Python 实现

Python 实现与 C++ 逻辑完全相同，只是语法不同。

### 创建 Python 脚本

```bash
mkdir -p ~/ros_demo/src/hello_world/scripts
```

在 `hello_world/scripts/` 下创建 `talker_p.py`：

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

import rospy
from std_msgs.msg import String

if __name__ == "__main__":
    # 1. 初始化节点
    rospy.init_node("talker_p")

    # 2. 创建发布者
    pub = rospy.Publisher("/chatter", String, queue_size=10)

    # 3. 设置发布频率
    rate = rospy.Rate(10)  # 10Hz

    count = 0
    while not rospy.is_shutdown():
        # 4. 组装消息
        msg = String()
        msg.data = "hello world %d" % count

        # 5. 发布消息
        pub.publish(msg)

        # 6. 打印日志
        rospy.loginfo("发布: %s" % msg.data)

        # 7. 按频率休眠
        rate.sleep()
        count += 1
```

在 `hello_world/scripts/` 下创建 `listener_p.py`：

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

import rospy
from std_msgs.msg import String

def chatterCallback(msg):
    rospy.loginfo("收到: [%s]" % msg.data)

if __name__ == "__main__":
    # 1. 初始化节点
    rospy.init_node("listener_p")

    # 2. 创建订阅者
    sub = rospy.Subscriber("/chatter", String, chatterCallback, queue_size=10)

    # 3. 阻塞等待回调
    rospy.spin()
```

### 添加可执行权限并运行

```bash
# 添加可执行权限（Python 必须！）
chmod +x ~/ros_demo/src/hello_world/scripts/talker_p.py
chmod +x ~/ros_demo/src/hello_world/scripts/listener_p.py

# Python 节点不需要编译，直接运行
rosrun hello_world talker_p.py
rosrun hello_world listener_p.py
```

> 💡 **C++ vs Python 对比**：
> - C++ 需要编译（`catkin_make`），运行效率高，适合性能敏感的节点
> - Python 无需编译，开发效率高，适合快速原型和算法验证
> - 两者可以混用：C++ 写驱动，Python 写 AI 算法，通过话题无缝通信

---

## 2.1.4 话题通信自定义 msg

在实际项目中，标准消息类型往往不够用，需要自定义消息。

### 步骤 1：定义 msg 文件

```bash
mkdir -p ~/ros_demo/src/hello_world/msg
cd ~/ros_demo/src/hello_world/msg
touch Person.msg
```

编辑 `Person.msg`：

```
string name
uint8 age
float64 height
```

> 💡 **msg 支持的数据类型**：
> - 基础类型：`int8/16/32/64`、`uint8/16/32/64`、`float32/64`、`string`、`bool`
> - 其他包的 msg：`geometry_msgs/Pose`、`std_msgs/Header`
> - 数组：`string[] name_list`、`int32[] data`

### 步骤 2：配置 package.xml

编辑 `hello_world/package.xml`，添加消息生成和运行时依赖：

```xml
<build_depend>message_generation</build_depend>
<exec_depend>message_runtime</exec_depend>
```

> 💡 **为什么需要这两个依赖？**
> - `message_generation`：编译时用来将 `.msg` 文件转换成 C++ 头文件和 Python 模块
> - `message_runtime`：运行时用来支持自定义消息的序列化和反序列化

### 步骤 3：配置 CMakeLists.txt

编辑 `hello_world/CMakeLists.txt`，做三处修改：

**修改 1**：在 `find_package` 中添加 `message_generation`：

```cmake
find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
  message_generation    # ← 添加
)
```

**修改 2**：添加消息文件声明：

```cmake
add_message_files(
  FILES
  Person.msg
)
```

**修改 3**：声明消息依赖并生成消息：

```cmake
generate_messages(
  DEPENDENCIES
  std_msgs
)
```

**修改 4**：在 `catkin_package` 中添加 `message_runtime`：

```cmake
catkin_package(
  CATKIN_DEPENDS roscpp rospy std_msgs message_runtime
)
```

### 步骤 4：编译

```bash
cd ~/ros_demo
catkin_make
```

编译成功后，会自动生成：

- C++ 头文件：`devel/include/hello_world/Person.h`
- Python 模块：`devel/lib/python3/dist-packages/hello_world/msg/`

### 步骤 5：验证自定义消息

```bash
rosmsg show hello_world/Person
```

输出：

```
[string name
uint8 age
float64 height
```

---

## 2.1.5 话题通信自定义 msg 调用 A：C++ 实现

### 编写发布者

在 `hello_world/src/` 下创建 `person_talker.cpp`：

```cpp
#include "ros/ros.h"
#include "hello_world/Person.h"  // 自定义消息头文件

int main(int argc, char *argv[])
{
    ros::init(argc, argv, "person_talker");
    ros::NodeHandle nh;

    // 使用自定义消息类型创建发布者
    ros::Publisher pub = nh.advertise<hello_world::Person>("/person_info", 10);

    ros::Rate rate(1);

    int count = 0;
    while (ros::ok())
    {
        hello_world::Person msg;
        msg.name = "张三";
        msg.age = 20 + count;
        msg.height = 1.75;

        pub.publish(msg);
        ROS_INFO("发布: 姓名=%s, 年龄=%d, 身高=%.2f",
                 msg.name.c_str(), msg.age, msg.height);

        rate.sleep();
        count++;
    }

    return 0;
}
```

### 编写订阅者

在 `hello_world/src/` 下创建 `person_listener.cpp`：

```cpp
#include "ros/ros.h"
#include "hello_world/Person.h"

void personCallback(const hello_world::Person::ConstPtr& msg)
{
    ROS_INFO("收到: 姓名=%s, 年龄=%d, 身高=%.2f",
             msg->name.c_str(), msg->age, msg->height);
}

int main(int argc, char *argv[])
{
    ros::init(argc, argv, "person_listener");
    ros::NodeHandle nh;

    ros::Subscriber sub = nh.subscribe("/person_info", 10, personCallback);

    ros::spin();
    return 0;
}
```

### 配置编译并运行

在 `CMakeLists.txt` 中添加：

```cmake
add_executable(person_talker src/person_talker.cpp)
target_link_libraries(person_talker ${catkin_LIBRARIES})

add_executable(person_listener src/person_listener.cpp)
target_link_libraries(person_listener ${catkin_LIBRARIES})
```

编译运行：

```bash
cd ~/ros_demo
catkin_make
source devel/setup.bash

roscore
rosrun hello_world person_talker
rosrun hello_world person_listener
```

---

## 2.1.6 话题通信自定义 msg 调用 B：Python 实现

### 编写发布者

在 `hello_world/scripts/` 下创建 `person_talker_p.py`：

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

import rospy
from hello_world.msg import Person  # 导入自定义消息

if __name__ == "__main__":
    rospy.init_node("person_talker_p")

    pub = rospy.Publisher("/person_info", Person, queue_size=10)
    rate = rospy.Rate(1)

    count = 0
    while not rospy.is_shutdown():
        msg = Person()
        msg.name = "张三"
        msg.age = 20 + count
        msg.height = 1.75

        pub.publish(msg)
        rospy.loginfo("发布: 姓名=%s, 年龄=%d, 身高=%.2f",
                      msg.name, msg.age, msg.height)

        rate.sleep()
        count += 1
```

### 编写订阅者

在 `hello_world/scripts/` 下创建 `person_listener_p.py`：

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

import rospy
from hello_world.msg import Person

def personCallback(msg):
    rospy.loginfo("收到: 姓名=%s, 年龄=%d, 身高=%.2f",
                  msg.name, msg.age, msg.height)

if __name__ == "__main__":
    rospy.init_node("person_listener_p")
    rospy.Subscriber("/person_info", Person, personCallback, queue_size=10)
    rospy.spin()
```

### 添加权限并运行

```bash
chmod +x ~/ros_demo/src/hello_world/scripts/person_talker_p.py
chmod +x ~/ros_demo/src/hello_world/scripts/person_listener_p.py

# 不需要编译，直接运行
rosrun hello_world person_talker_p.py
rosrun hello_world person_listener_p.py
```

---

## 本节小结

| 概念 | 说明 |
|------|------|
| **发布/订阅模型** | 异步、单向、流式、多对多 |
| **C++ 创建发布者** | `nh.advertise<类型>("话题名", 队列长度)` |
| **C++ 创建订阅者** | `nh.subscribe("话题名", 队列长度, 回调函数)` |
| **Python 创建发布者** | `rospy.Publisher("话题名", 类型, queue_size=)` |
| **Python 创建订阅者** | `rospy.Subscriber("话题名", 类型, 回调函数)` |
| **自定义 msg 流程** | 定义 .msg 文件 → 配置 package.xml → 配置 CMakeLists.txt → 编译 |
| **自定义消息依赖** | `message_generation`（编译时）+ `message_runtime`（运行时） |

---

## 理解检查

学完本节后，你应该能够：

- ✅ 描述话题通信的 5 步连接建立流程
- ✅ 解释为什么 Master 挂掉后已建立的连接仍能工作
- ✅ 编写 C++ 版本的发布者和订阅者节点
- ✅ 编写 Python 版本的发布者和订阅者节点
- ✅ 定义自定义消息类型并在节点中使用
- ✅ 理解 `ros::spin()` 和回调函数的关系

---

## 🔧 动手练习

1. **修改发布频率**：将 Talker 的频率改为 1Hz 和 50Hz，观察 Subscriber 的输出变化
2. **多订阅者实验**：启动一个 Talker，启动两个 Listener，观察是否都能收到消息
3. **多发布者实验**：启动两个 Talker（话题名相同），启动一个 Listener，观察输出
4. **自定义消息扩展**：在 `Person.msg` 中添加 `string email` 和 `bool is_student` 字段，重新编译并使用

---

## 下一节

话题通信是 ROS 最常用的通信方式。接下来学习另一种重要的通信机制——服务通信：

→ **[2.2 服务通信](../../chapter02/07-service.md)**
