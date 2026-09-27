# 2.2 服务通信

> 本节学习目标：理解服务通信的请求/响应模型，掌握 C++ 和 Python 实现服务端与客户端节点的方法，以及如何定义和使用自定义服务类型。

---

## 2.2.1 服务通信理论模型

服务通信是 ROS 中另一种重要的通信方式，采用**请求/响应（Request/Response）**模型。

### 模型图解

```
┌─────────────────────────────────────────────────────────────────┐
│                     服务通信模型                                  │
│                                                                 │
│   ┌──────────────┐                      ┌──────────────┐        │
│   │   Client     │                      │   Server     │        │
│   │  (客户端)     │                      │  (服务端)     │        │
│   │              │      请求（Request）   │              │        │
│   │  "请帮我       │ ═══════════════════▶ │  "收到请求    │        │
│   │   算一下"     │      响应（Response）  │   正在处理"   │        │
│   │  "收到结果"   │ ◀═══════════════════ │  "这是结果"   │        │
│   └──────────────┘                      └──────────────┘        │
│                                                                 │
│   特点：同步、双向、一问一答、1:1                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 核心特点

| 特点 | 说明 |
|------|------|
| **同步** | 客户端发送请求后会阻塞等待，直到收到响应 |
| **双向** | 数据在客户端和服务端之间双向流动 |
| **一问一答** | 一次请求对应一次响应，没有持续数据流 |
| **1:1** | 一个服务只能有一个服务端，但可以有多个客户端 |

### 适用场景

服务通信适合以下场景：

- **一次性操作**：拍照、清除日志、重置状态
- **参数设置**：设置目标位置、切换模式、配置参数
- **状态查询**：获取当前地图、查询机器人位姿
- **计算请求**：路径规划请求、逆运动学求解

### 话题 vs 服务

| 对比项 | 话题（Topic） | 服务（Service） |
|--------|--------------|----------------|
| 通信模式 | 发布/订阅 | 请求/响应 |
| 同步性 | 异步 | 同步（阻塞等待） |
| 方向 | 单向 | 双向 |
| 数量关系 | 多对多 | 1:1（服务端唯一） |
| 生命周期 | 持续存在 | 调用时建立，用完释放 |
| 典型应用 | 传感器数据流 | 开关、查询、设置 |
| 底层协议 | TCPROS/UDPROS | TCPROS |

### 通信流程

一次完整的服务调用经历以下步骤：

```
Client                       Master                      Server
  │                            │                            │
  │                            │ ① registerService(/add)    │
  │                            │◀───────────────────────────│
  │                            │   登记：/add 的服务端地址    │
  │ ② lookupService(/add)      │                            │
  │ ──────────────────────────▶│                            │
  │ ◀── 返回 Server URI ───────│                            │
  │                            │                            │
  │ ③ XMLRPC 协商 → TCPROS 直连│                            │
  │ ═══════════════════════════════════════════════════════▶│
  │        ④ 请求 → ──────────────────────────────────────▶ │
  │        ◀───────────────────────────── 响应               │
  │        (客户端阻塞等待响应)                               │
```

1. **Server 注册**：服务端启动时向 Master 注册服务名和地址
2. **Client 查找**：客户端向 Master 查询服务地址
3. **建立连接**：客户端与服务端建立 TCPROS 连接
4. **请求/响应**：客户端发送请求，阻塞等待，服务端处理后返回响应

---

## 2.2.2 服务通信自定义 srv

与话题通信类似，实际项目中往往需要自定义服务类型。

### 步骤 1：定义 srv 文件

```bash
mkdir -p ~/ros_demo/src/hello_world/srv
cd ~/ros_demo/src/hello_world/srv
touch AddTwoInts.srv
```

编辑 `AddTwoInts.srv`：

```
int64 a        # ← 请求部分（Request）
int64 b
---            # ← 分隔线
int64 sum      # ← 响应部分（Response）
```

> 💡 **srv 文件的结构**：`---` 上方是请求参数，下方是响应参数。这与 msg 文件不同，msg 只有一个部分。

### 步骤 2：配置 package.xml

编辑 `hello_world/package.xml`，确保已添加（如果前面自定义 msg 时已添加则无需重复）：

```xml
<build_depend>message_generation</build_depend>
<exec_depend>message_runtime</exec_depend>
```

### 步骤 3：配置 CMakeLists.txt

**修改 1**：`find_package` 中包含 `message_generation`：

```cmake
find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
  message_generation
)
```

**修改 2**：添加服务文件声明：

```cmake
add_service_files(
  FILES
  AddTwoInts.srv
)
```

**修改 3**：生成消息（srv 也用这个函数）：

```cmake
generate_messages(
  DEPENDENCIES
  std_msgs
)
```

**修改 4**：`catkin_package` 中包含 `message_runtime`：

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

### 步骤 5：验证自定义服务

```bash
rossrv show hello_world/AddTwoInts
```

输出：

```
int64 a
int64 b
---
int64 sum
```

---

## 2.2.3 服务通信自定义 srv 调用 A：C++ 实现

### 编写服务端节点

在 `hello_world/src/` 下创建 `server.cpp`：

```cpp
#include "ros/ros.h"
#include "hello_world/AddTwoInts.h"   // 由 .srv 自动生成的头文件

// 服务回调函数
// 参数：请求对象引用、响应对象引用
// 返回值：true 表示处理成功，false 表示失败
bool add(hello_world::AddTwoInts::Request &req,
         hello_world::AddTwoInts::Response &res)
{
    res.sum = req.a + req.b;
    ROS_INFO("请求: %ld + %ld, 响应: %ld",
             (long)req.a, (long)req.b, (long)res.sum);
    return true;
}

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    ros::init(argc, argv, "add_server");
    ros::NodeHandle nh;

    // 注册服务：服务名 /add_two_ints，处理函数 add
    ros::ServiceServer server = nh.advertiseService("add_two_ints", add);

    ROS_INFO("加法服务已就绪...");

    ros::spin();   // 阻塞等待请求
    return 0;
}
```

**代码逐行解释：**

第 5-12 行定义了服务回调函数 `add`。每次客户端调用服务时，ROS 会自动调用这个函数。参数中 `Request &req` 是输入（客户端发来的请求），`Response &res` 是输出（要返回给客户端的响应）。函数返回 `true` 表示处理成功，客户端会收到响应；返回 `false` 表示处理失败。

第 22 行 `nh.advertiseService("add_two_ints", add)` 创建服务对象。第一个参数是服务名，第二个参数是回调函数指针。注册后，Master 会记录这个服务的地址，客户端可以通过服务名找到它。

第 27 行 `ros::spin()` 阻塞主线程，等待客户端请求。没有 spin，节点会立即退出，服务就无法响应了。

### 编写客户端节点

在 `hello_world/src/` 下创建 `client.cpp`：

```cpp
#include "ros/ros.h"
#include "hello_world/AddTwoInts.h"

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");

    // 参数校验：需要传入两个数字
    if (argc != 3)
    {
        ROS_ERROR("用法: rosrun hello_world client X Y");
        return 1;
    }

    ros::init(argc, argv, "add_client");
    ros::NodeHandle nh;

    // 创建客户端
    ros::ServiceClient client = nh.serviceClient<hello_world::AddTwoInts>("add_two_ints");

    // 等待服务上线（阻塞，最多等 3 秒）
    client.waitForExistence(ros::Duration(3.0));

    // 构造请求
    hello_world::AddTwoInts srv;
    srv.request.a = atoll(argv[1]);
    srv.request.b = atoll(argv[2]);

    // 发起同步调用，阻塞直到收到响应
    if (client.call(srv))
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

**代码逐行解释：**

第 9-13 行进行参数校验。`argc` 包含程序名本身，所以传两个数字时 `argc` 为 3。参数不足时打印用法提示并退出。

第 21 行 `nh.serviceClient<hello_world::AddTwoInts>("add_two_ints")` 创建客户端对象。模板参数指定服务类型，参数指定服务名。注意客户端和服务端使用**同一个服务名**才能匹配。

第 24 行 `client.waitForExistence(ros::Duration(3.0))` 等待服务上线。如果服务尚未启动，客户端会阻塞等待最多 3 秒。超时后继续执行，调用会失败。也可以用 `ros::service::waitForService("add_two_ints")` 实现相同功能。

第 27-28 行构造请求，从命令行参数读取两个数字并赋值给请求字段。`atoll()` 将字符串转换为 `long long` 类型。

第 31 行 `client.call(srv)` 发起同步调用。这个函数会阻塞，直到收到服务端的响应或调用失败。调用成功后，结果保存在 `srv.response.sum` 中。

### 配置编译并运行

在 `CMakeLists.txt` 中添加：

```cmake
add_executable(server src/server.cpp)
target_link_libraries(server ${catkin_LIBRARIES})

add_executable(client src/client.cpp)
target_link_libraries(client ${catkin_LIBRARIES})
```

编译运行：

```bash
cd ~/ros_demo
catkin_make
source devel/setup.bash

# 终端 1：启动 roscore
roscore

# 终端 2：启动服务端
rosrun hello_world server

# 终端 3：启动客户端（传入两个数字）
rosrun hello_world client 3 5
```

**运行效果：**

Server 终端输出：

```
[ INFO] [1695800000.000000000]: 加法服务已就绪...
[ INFO] [1695800001.000000000]: 请求: 3 + 5, 响应: 8
```

Client 终端输出：

```
[ INFO] [1695800001.000000000]: 响应: sum = 8
```

---

## 2.2.4 服务通信自定义 srv 调用 B：Python 实现

### 编写服务端节点

在 `hello_world/scripts/` 下创建 `server_p.py`：

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

import rospy
from hello_world.srv import AddTwoInts, AddTwoIntsResponse

def add(req):
    """服务回调函数"""
    res = AddTwoIntsResponse()
    res.sum = req.a + req.b
    rospy.loginfo("请求: %d + %d = %d" % (req.a, req.b, res.sum))
    return res

if __name__ == "__main__":
    rospy.init_node("add_server_p")

    # 注册服务
    server = rospy.Service("add_two_ints", AddTwoInts, add)

    rospy.loginfo("加法服务已就绪...")
    rospy.spin()
```

**代码逐行解释：**

第 5 行从 `hello_world.srv` 模块导入 `AddTwoInts` 服务类型和 `AddTwoIntsResponse` 响应类型。Python 中 srv 会生成三个类：服务基类、请求类和响应类。

第 7-12 行定义回调函数。与 C++ 不同，Python 的回调函数直接返回响应对象，不需要通过参数引用来填充。

第 19 行 `rospy.Service("add_two_ints", AddTwoInts, add)` 注册服务。三个参数分别是：服务名、服务类型、回调函数。

### 编写客户端节点

在 `hello_world/scripts/` 下创建 `client_p.py`：

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import rospy
from hello_world.srv import AddTwoInts

if __name__ == "__main__":
    # 参数校验
    if len(sys.argv) != 3:
        rospy.logerr("用法: rosrun hello_world client_p.py X Y")
        sys.exit(1)

    rospy.init_node("add_client_p")

    # 等待服务上线
    rospy.wait_for_service("add_two_ints", timeout=3.0)

    try:
        # 创建客户端并调用
        client = rospy.ServiceProxy("add_two_ints", AddTwoInts)
        resp = client(int(sys.argv[1]), int(sys.argv[2]))
        rospy.loginfo("响应: sum = %d" % resp.sum)
    except rospy.ServiceException as e:
        rospy.logerr("服务调用失败: %s" % e)
```

**代码逐行解释：**

第 15 行 `rospy.wait_for_service("add_two_ints", timeout=3.0)` 等待服务上线，对应 C++ 中的 `client.waitForExistence()`。

第 20 行 `rospy.ServiceProxy("add_two_ints", AddTwoInts)` 创建客户端代理。代理对象可以像普通函数一样直接调用。

第 21 行 `client(int(sys.argv[1]), int(sys.argv[2]))` 发起调用。Python 的 ServiceProxy 支持直接传参，比 C++ 更简洁。参数按 srv 定义中的顺序传入。

第 23-24 行捕获服务调用异常。如果服务不存在、连接失败或处理出错，会抛出 `rospy.ServiceException`。

### 添加权限并运行

```bash
chmod +x ~/ros_demo/src/hello_world/scripts/server_p.py
chmod +x ~/ros_demo/src/hello_world/scripts/client_p.py

# 不需要编译，直接运行
rosrun hello_world server_p.py
rosrun hello_world client_p.py 10 20
```

---

## 本节小结

| 概念 | 说明 |
|------|------|
| **请求/响应模型** | 同步、双向、一问一答、1:1 |
| **C++ 创建服务端** | `nh.advertiseService("服务名", 回调函数)` |
| **C++ 创建客户端** | `nh.serviceClient<类型>("服务名")` |
| **Python 创建服务端** | `rospy.Service("服务名", 类型, 回调函数)` |
| **Python 创建客户端** | `rospy.ServiceProxy("服务名", 类型)` |
| **自定义 srv 流程** | 定义 .srv 文件（--- 分隔请求和响应）→ 配置 package.xml → 配置 CMakeLists.txt → 编译 |
| **自定义服务依赖** | `message_generation`（编译时）+ `message_runtime`（运行时） |

---

## 理解检查

学完本节后，你应该能够：

- ✅ 描述服务通信的完整流程
- ✅ 区分话题通信和服务通信的适用场景
- ✅ 编写 C++ 版本的服务端和客户端节点
- ✅ 编写 Python 版本的服务端和客户端节点
- ✅ 定义自定义服务类型并在节点中使用
- ✅ 理解"同步阻塞等待响应"的含义

---

## 🔧 动手练习

1. **扩展服务功能**：修改 `AddTwoInts.srv`，增加减法、乘法、除法功能
2. **多客户端实验**：启动一个 Server，启动多个 Client 同时调用，观察服务端如何处理
3. **错误处理实验**：不启动 Server 直接运行 Client，观察超时和错误提示
4. **自定义 srv**：创建一个 `Person.srv`，请求为 `string name`，响应为 `string greeting`（返回"你好, xxx"）

---

## 下一节

服务通信适合一问一答的场景。接下来学习参数服务器，用于全局配置管理：

→ **[2.3 参数服务器](../../chapter02/08-parameter.md)**
