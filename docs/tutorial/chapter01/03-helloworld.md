# 1.3 ROS 快速体验

> 本节学习目标：创建第一个 ROS 工作空间，编写并运行 C++ 和 Python 版本的 HelloWorld 节点，获得第一次成功的体验。

---

## 1.3.1 HelloWorld 实现简介

我们将实现一个最简单的 ROS 节点——启动后输出一行"Hello World"日志。虽然简单，但它涵盖了 ROS 开发的核心流程：

```
创建工作空间 → 创建功能包 → 编写节点代码 → 编译 → 运行
```

### 最终效果

运行后终端输出：

```
[ INFO] [1695800000.000000000]: Hello World! by C++
[ INFO] [1695800000.000000000]: Hello World! by Python
```

---

## 1.3.2 创建工作空间

ROS 使用 **catkin** 构建系统，所有代码必须放在 **catkin 工作空间** 中。

### 步骤 1：创建目录结构

```bash
# 创建工作空间根目录
mkdir -p ~/ros_demo/src

# 进入工作空间
cd ~/ros_demo
```

### 步骤 2：初始化工作空间

```bash
# 初始化 catkin 工作空间（在 src/ 下生成 CMakeLists.txt）
catkin_init_workspace src
```

此时目录结构：

```
ros_demo/
└── src/
    └── CMakeLists.txt    ← catkin 生成的顶层 CMake
```

### 步骤 3：创建功能包

```bash
# 进入 src 目录
cd src

# 创建功能包：包名 hello_world，依赖 roscpp、rospy、std_msgs
catkin_create_pkg hello_world roscpp rospy std_msgs
```

> 💡 **`catkin_create_pkg` 做了什么？**
> - 创建了 `hello_world/` 目录
> - 生成了 `package.xml`（包元信息）
> - 生成了 `CMakeLists.txt`（编译规则）
> - 创建了 `src/` 和 `include/` 目录

此时目录结构：

```
ros_demo/
└── src/
    ├── CMakeLists.txt
    └── hello_world/
        ├── package.xml          ← 包元信息（依赖、版本、描述）
        ├── CMakeLists.txt       ← 编译规则
        ├── src/                 ← C++ 源码放这里
        └── include/             ← C++ 头文件放这里
```

---

## 1.3.3 HelloWorld 实现 A：C++ 版本

### 步骤 1：编写节点代码

```bash
# 进入功能包的 src 目录
cd ~/ros_demo/src/hello_world/src/

# 创建并编辑 C++ 节点
touch helloworld_c.cpp
code helloworld_c.cpp    # 或用 vim/nano
```

**`helloworld_c.cpp` 完整代码：**

```cpp
#include "ros/ros.h"
#include "std_msgs/String.h"

int main(int argc, char *argv[])
{
    // 1. 初始化 ROS 节点，注册节点名称为 "hello_c"
    //    注意：节点名是 ROS 中该节点的唯一标识
    ros::init(argc, argv, "hello_c");

    // 2. 创建节点句柄（NodeHandle）
    //    它是节点与 ROS 系统通信的入口，管理发布/订阅/服务/参数等
    ros::NodeHandle nh;

    // 3. 输出日志
    //    ROS_INFO 类似 printf，但带时间戳、日志级别、自动输出到 rosout
    ROS_INFO("Hello World! by C++");

    // 4. spin 一次（因为本节点只输出一行就结束，不需要持续运行）
    ros::spinOnce();

    return 0;
}
```

**代码逐行解释：**

第 1 行 `#include "ros/ros.h"` 引入 ROS C++ 核心库，这是编写 ROS C++ 节点必须的头文件，包含了初始化、日志、节点句柄等核心功能。

第 2 行 `#include "std_msgs/String.h"` 引入标准字符串消息类型。虽然本例只是输出日志没有发布消息，但加上它是良好的编程习惯，后续扩展时可以直接使用。

第 7 行 `ros::init(argc, argv, "hello_c")` 初始化 ROS 节点，将节点注册到 Master，名称为 `"hello_c"`。这个函数必须是节点 main 函数中第一个调用的 ROS 函数，它负责解析命令行参数、建立与 Master 的连接。

第 11 行 `ros::NodeHandle nh` 创建节点句柄（NodeHandle）。它是节点与 ROS 系统通信的入口，后续创建发布者、订阅者、客户端、参数操作等都要通过它。可以把 NodeHandle 想象成节点的"手"，通过这只手来抓取 ROS 的各种服务。

第 15 行 `ROS_INFO("Hello World! by C++")` 输出一条 INFO 级别的日志。`ROS_INFO` 是一个宏，用法类似 `printf`，支持格式化输出（如 `ROS_INFO("count = %d", count)`）。相比标准输出，ROS_INFO 会自动添加时间戳、节点名、日志级别，并同步发布到 `/rosout` 话题供远程查看。

第 18 行 `ros::spinOnce()` 处理一次当前积压的回调。本例中没有订阅任何话题，所以实际上没有回调需要处理，但加上它是良好的编程习惯。在带有 while 循环的节点中，`ros::spinOnce()` 放在循环体内可以持续处理回调，而不阻塞主循环。

### 步骤 2：配置 CMakeLists.txt

编辑 `~/ros_demo/src/hello_world/CMakeLists.txt`，找到以下行并取消注释/添加：

```cmake
# 在文件末尾添加：
add_executable(hello_node src/helloworld_c.cpp)
target_link_libraries(hello_node ${catkin_LIBRARIES})
```

> 💡 **这两行的含义**：
> - `add_executable(hello_node src/helloworld_c.cpp)`：将 `helloworld_c.cpp` 编译成名为 `hello_node` 的可执行文件
> - `target_link_libraries(hello_node ${catkin_LIBRARIES})`：链接 ROS 库

### 步骤 3：编译

```bash
# 回到工作空间根目录
cd ~/ros_demo

# 编译整个工作空间
catkin_make
```

编译成功后输出：

```
-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-- ~~  traversing packages in topological order:
-- ~~  - hello_world
-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-- +++ processing catkin package: 'hello_world'
-- ==> add_subdirectory(hello_world)
...
[100%] Built target hello_node
```

> ⚠️ **注意**：`catkin_make` 必须在工作空间根目录（即 `src/` 的上一级）执行。

### 步骤 4：运行

```bash
# 加载环境变量（让系统能找到你编译出的包）
source devel/setup.bash

# 启动 ROS Master（新终端必须执行）
roscore

# 在另一个终端运行节点
rosrun hello_world hello_node
```

**输出：**

```
[ INFO] [1695800000.000000000]: Hello World! by C++
```

> 💡 **rosrun 的用法**：`rosrun <包名> <可执行文件名>`
> - 包名：`hello_world`（`package.xml` 中定义的名字）
> - 可执行文件名：`hello_node`（`CMakeLists.txt` 中 `add_executable` 定义的目标名）

> ⚠️ **常见错误**：如果提示 `Couldn't find executable named hello_node`，说明：
> 1. 编译失败了 → 检查 `catkin_make` 输出
> 2. 没有 source → 执行 `source devel/setup.bash`
> 3. CMakeLists.txt 配置错了 → 检查目标名是否一致

---

## 1.3.4 HelloWorld 实现 B：Python 版本

Python 节点**不需要编译**，更加快捷。

### 步骤 1：创建 scripts 目录

```bash
mkdir -p ~/ros_demo/src/hello_world/scripts
```

### 步骤 2：编写节点代码

```bash
cd ~/ros_demo/src/hello_world/scripts/
touch helloworld_p.py
code helloworld_p.py
```

**`helloworld_p.py` 完整代码：**

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

import rospy                    # 引入 ROS Python 核心库
from std_msgs.msg import String  # 引入标准字符串消息类型（本例未使用）

if __name__ == "__main__":
    # 1. 初始化 ROS 节点，注册节点名称为 "hello_p"
    rospy.init_node("hello_p")

    # 2. 输出日志
    rospy.loginfo("Hello World! by Python")

    # 3. spin 防止节点退出
    rospy.spin()
```

**代码解释：**

`#!/usr/bin/env python` 是 Shebang，告诉系统用环境中的 Python 解释器来执行这个脚本，这样在终端可以直接 `./helloworld_p.py` 运行。

`import rospy` 引入 ROS Python 核心库。和 C++ 中的 `#include "ros/ros.h"` 类似，这是使用 ROS 功能的入口。

`rospy.init_node("hello_p")` 初始化 ROS 节点，并将节点注册到 Master，名称为 `"hello_p"`。这个名称在 ROS 网络中唯一标识该节点，其他节点通过它来发现彼此。对应 C++ 中的 `ros::init(argc, argv, "hello_c")`。

`rospy.spin()` 阻塞主线程，防止节点退出。因为本节点除了输出日志外没有其他持续工作，如果不 spin，节点执行完就会退出。spin 会让节点保持运行状态，直到收到 Ctrl+C 或 `rosnode kill`。

### 步骤 3：添加可执行权限

```bash
chmod +x ~/ros_demo/src/hello_world/scripts/helloworld_p.py
```

> ⚠️ **Python 节点必须添加可执行权限**，否则 `rosrun` 会报 `Permission denied`。

### 步骤 4：运行

Python 节点**不需要编译**，直接运行：

```bash
# 确保已加载环境变量
source ~/ros_demo/devel/setup.bash

# 启动 roscore（如果还没启动）
roscore

# 在另一个终端运行 Python 节点
rosrun hello_world helloworld_p.py
```

**输出：**

```
[ INFO] [1695800000.500000000]: Hello World! by Python
```

---

## 1.3.5 使用 launch 文件同时启动两个节点

每次都要开两个终端分别运行很麻烦，用 **launch 文件** 可以一键启动多个节点。

### 步骤 1：创建 launch 目录和启动文件

```bash
mkdir -p ~/ros_demo/src/hello_world/launch
cd ~/ros_demo/src/hello_world/launch
touch start_hello.launch
```

### 步骤 2：编写 launch 文件

```xml
<launch>
    <!-- 启动 C++ 节点 -->
    <node pkg="hello_world" type="hello_node" name="hello_cpp" output="screen"/>

    <!-- 启动 Python 节点 -->
    <node pkg="hello_world" type="helloworld_p.py" name="hello_python" output="screen"/>
</launch>
```

**launch 文件标签说明：**

| 属性 | 含义 |
|------|------|
| `pkg="hello_world"` | 节点所属的功能包 |
| `type="hello_node"` | 可执行文件名（C++ 是 CMake 目标名，Python 是脚本名） |
| `name="hello_cpp"` | 节点在 ROS 中的运行时名称（唯一标识） |
| `output="screen"` | 日志输出到终端 |

### 步骤 3：运行 launch 文件

```bash
# roslaunch 会自动启动 roscore（如果还没启动）
roslaunch hello_world start_hello.launch
```

**输出：**

```
... logging to /home/username/.ros/log/xxx.log
Checking log disk usage for logging purposes.
...
[ INFO] [1695800000.000000000]: Hello World! by C++
[ INFO] [1695800000.500000000]: Hello World! by Python
```

---

## 1.3.6 验证节点运行状态

### 查看运行中的节点

```bash
# 在另一个终端执行
rosnode list
```

**输出：**

```
/hello_cpp
/hello_python
/rosout
```

> 💡 `/rosout` 是 ROS 自动启动的日志节点，不需要手动启动。

### 查看节点详细信息

```bash
rosnode info /hello_cpp
```

**输出：**

```
--------------------------------------------------------------------------------
Node [/hello_cpp]
Publications: None
Subscriptions: None
Services: None

contacting node http://ubuntu:43351/ ...
Pid: 12345
Connections: none
```

### 终止节点

```bash
rosnode kill /hello_cpp
```

---

## 1.3.7 最终项目结构

```
ros_demo/                           ← catkin 工作空间
├── src/                            ← 源代码目录
│   ├── CMakeLists.txt              ← 顶层 CMake（catkin 生成）
│   └── hello_world/                ← 功能包
│       ├── package.xml             ← 包元信息
│       ├── CMakeLists.txt          ← 编译规则
│       ├── src/
│       │   └── helloworld_c.cpp    ← C++ 节点源码
│       ├── scripts/
│       │   └── helloworld_p.py     ← Python 节点源码
│       └── launch/
│           └── start_hello.launch  ← 启动文件
├── build/                          ← 编译中间产物
└── devel/                          ← 开发环境
    ├── setup.bash                  ← 环境变量配置
    └── share/hello_world/cmake/    ← 包配置文件
```

---

## 本节小结

| 步骤 | 命令 | 作用 |
|------|------|------|
| 创建工作空间 | `mkdir -p ~/ros_demo/src && catkin_init_workspace src` | 初始化 catkin 工作空间 |
| 创建功能包 | `catkin_create_pkg hello_world roscpp rospy std_msgs` | 生成包模板 |
| 编写 C++ 节点 | 编辑 `src/helloworld_c.cpp` | 实现节点逻辑 |
| 配置编译规则 | 编辑 `CMakeLists.txt`，添加 `add_executable` | 告诉 catkin 编译哪个文件 |
| 编译 | `catkin_make` | 编译整个工作空间 |
| 加载环境 | `source devel/setup.bash` | 让系统找到你的包 |
| 运行节点 | `rosrun hello_world hello_node` | 启动单个节点 |
| 批量启动 | `roslaunch hello_world start_hello.launch` | 启动多个节点 |

---

## 理解检查

学完本节后，你应该能够：

- ✅ 解释 catkin 工作空间的目录结构
- ✅ 区分 `rosrun` 和 `roslaunch` 的用途
- ✅ 编写一个最简单的 C++ 和 Python 节点
- ✅ 理解 `ros::init` / `rospy.init_node` 的作用
- ✅ 使用 `rosnode list` 查看运行中的节点

---

## 🔧 动手练习

1. **修改 HelloWorld 消息**：将输出内容改成"你好，ROS！"，重新编译运行。
2. **添加第三个节点**：创建一个 Python 节点，输出当前时间（使用 `rospy.get_time()`）。
3. **修改 launch 文件**：将新节点加入 `start_hello.launch`，一键启动三个节点。

---

## 下一节

我们已经成功运行了第一个 ROS 程序。接下来深入了解 ROS 的架构设计：

→ **[1.5 ROS 架构](05-architecture.md)**
