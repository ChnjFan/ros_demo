# 2.3 参数服务器

> 本节学习目标：理解参数服务器的全局键值存储模型，掌握 C++ 和 Python 中读写参数的方法，以及如何在 launch 文件中操作参数。

---

## 2.3.1 参数服务器理论模型

参数服务器（Parameter Server）是 ROS 中用于**全局配置管理**的机制，本质上是一个**共享的字典**（键值对存储）。

### 模型图解

```
┌─────────────────────────────────────────────────────────────────┐
│                       参数服务器                                 │
│                                                                 │
│   ┌──────────┐                                  ┌──────────┐   │
│   │  Node A  │──── set("/max_speed", 1.0) ────▶│          │   │
│   └──────────┘                                  │          │   │
│                                                 │ Parameter│   │
│   ┌──────────┐                                  │  Server  │   │
│   │  Node B  │──── get("/max_speed") ─────────▶│          │   │
│   └──────────┘                                  │          │   │
│                                                 │ /max_speed│   │
│   ┌──────────┐                                  │ /pid_p   │   │
│   │  Node C  │──── set("/pid_p", 0.5) ────────▶│ /robot   │   │
│   └──────────┘                                  │  ...     │   │
│                                                 └──────────┘   │
│                                                                 │
│   特点：全局共享、键值存储、低频读写、非实时                      │
└─────────────────────────────────────────────────────────────────┘
```

### 核心特点

| 特点 | 说明 |
|------|------|
| **全局共享** | 任何节点都可以读写，没有归属限制 |
| **键值存储** | 参数以键值对形式存储，键是字符串，值是多种类型 |
| **低频读写** | 走 XMLRPC 协议，速度慢，不适合高频数据 |
| **非实时** | 不适合实时控制循环中的数据交换 |

### 支持的数据类型

| 类型 | C++ 类型 | Python 类型 | 示例 |
|------|---------|------------|------|
| 整数 | `int` | `int` | `42` |
| 浮点数 | `double` | `float` | `3.14` |
| 布尔值 | `bool` | `bool` | `true` / `false` |
| 字符串 | `std::string` | `str` | `"hello"` |
| 列表 | `std::vector` | `list` | `[1, 2, 3]` |
| 字典 | `XmlRpcValue` | `dict` | `{"x": 1, "y": 2}` |
| 二进制 | `std::vector<uint8_t>` | `bytes` | 二进制数据 |

### 适用场景

参数服务器适合存储以下数据：

- **机器人参数**：轮距、半径、最大速度
- **控制参数**：PID 系数、阈值
- **配置参数**：文件路径、话题名称
- **环境参数**：地图名称、仿真场景

> ⚠️ **不适合用参数服务器的场景**：高频传感器数据（用话题）、实时控制指令（用话题）、大数据量传输（用话题）

### 参数命名规则

```
/global_param       ← 全局参数（以 / 开头）
~private_param      ← 私有参数（以 ~ 开头，自动加节点名前缀）
relative_param      ← 相对参数（相对于节点命名空间）
```

- **全局参数**：以 `/` 开头，全局唯一，所有节点可见
- **私有参数**：以 `~` 开头，自动加上节点名前缀，如 `~speed` 实际存储为 `/node_name/speed`
- **相对参数**：相对于节点的命名空间

---

## 2.3.2 参数操作 A：C++ 实现

### 编写参数操作节点

在 `hello_world/src/` 下创建 `param_demo.cpp`：

```cpp
#include "ros/ros.h"

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    ros::init(argc, argv, "param_demo");
    ros::NodeHandle nh;

    // ============ 写参数 ============
    nh.setParam("type", "turtlesim");       // string
    nh.setParam("radius", 0.5);             // double
    nh.setParam("wheels", 4);               // int
    nh.setParam("use_sim", true);           // bool

    ROS_INFO("参数已设置");

    // ============ 读参数（带默认值）============
    std::string type;
    double radius;
    int wheels;

    // param() 模板函数：如果参数存在则读取到变量，不存在则使用默认值
    nh.param<std::string>("type", type, "default_type");
    nh.param("radius", radius, 1.0);    // 模板参数自动推断
    nh.param("wheels", wheels, 2);

    ROS_INFO("读取参数: type=%s, radius=%.2f, wheels=%d",
             type.c_str(), radius, wheels);

    // ============ 读参数（不带默认值）============
    // getParam() 返回 bool 表示参数是否存在
    double radius_get;
    if (nh.getParam("radius", radius_get))
    {
        ROS_INFO("getParam 读取 radius = %.2f", radius_get);
    }
    else
    {
        ROS_WARN("参数 radius 不存在");
    }

    // ============ 检查参数是否存在 ============
    if (nh.hasParam("type"))
    {
        ROS_INFO("参数 type 存在");
    }

    // ============ 删除参数 ============
    nh.deleteParam("radius");
    ROS_INFO("参数 radius 已删除");

    // 删除后再次检查
    if (!nh.hasParam("radius"))
    {
        ROS_INFO("确认 radius 已不存在");
    }

    return 0;
}
```

**代码逐行解释：**

第 13-16 行使用 `setParam()` 写入参数。这个函数是模板函数，支持多种类型：整数、浮点数、字符串、布尔值、列表等。参数会存储在参数服务器中，任何节点都可以读取。

第 22-25 行使用 `param()` 模板函数读取参数。这个函数接受三个参数：参数名、接收变量、默认值。如果参数存在，读取到变量中并返回 true；如果不存在，变量保持默认值并返回 false。这种方式**推荐使用**，因为不会因为参数不存在而崩溃。

第 32-40 行使用 `getParam()` 读取参数。与 `param()` 不同，它没有默认值参数，返回值表示参数是否存在。适合需要判断参数是否存在的场景。

第 43-46 行使用 `hasParam()` 检查参数是否存在，返回布尔值。

第 49 行使用 `deleteParam()` 删除参数。删除后其他节点无法再读取到这个参数。

### 配置编译并运行

在 `CMakeLists.txt` 中添加：

```cmake
add_executable(param_demo src/param_demo.cpp)
target_link_libraries(param_demo ${catkin_LIBRARIES})
```

编译运行：

```bash
cd ~/ros_demo
catkin_make
source devel/setup.bash

roscore
rosrun hello_world param_demo
```

**运行效果：**

```
[ INFO] [1695800000.000000000]: 参数已设置
[ INFO] [1695800000.000000000]: 读取参数: type=turtlesim, radius=0.50, wheels=4
[ INFO] [1695800000.000000000]: getParam 读取 radius = 0.50
[ INFO] [1695800000.000000000]: 参数 type 存在
[ INFO] [1695800000.000000000]: 参数 radius 已删除
[ INFO] [1695800000.000000000]: 确认 radius 已不存在
```

### 进阶：列表和字典参数

```cpp
#include "ros/ros.h"
#include <vector>
#include <map>

int main(int argc, char *argv[])
{
    ros::init(argc, argv, "param_advanced");
    ros::NodeHandle nh;

    // 设置列表参数
    std::vector<std::string> names;
    names.push_back("Alice");
    names.push_back("Bob");
    names.push_back("Charlie");
    nh.setParam("names", names);

    // 设置字典参数（YAML 格式）
    std::map<std::string, double> pid;
    pid["p"] = 1.0;
    pid["i"] = 0.1;
    pid["d"] = 0.01;
    nh.setParam("pid", pid);

    // 读取列表
    std::vector<std::string> get_names;
    nh.getParam("names", get_names);
    for (size_t i = 0; i < get_names.size(); i++)
    {
        ROS_INFO("names[%zu] = %s", i, get_names[i].c_str());
    }

    // 读取字典
    std::map<std::string, double> get_pid;
    nh.getParam("pid", get_pid);
    ROS_INFO("PID: p=%.2f, i=%.2f, d=%.2f",
             get_pid["p"], get_pid["i"], get_pid["d"]);

    return 0;
}
```

---

## 2.3.3 参数操作 B：Python 实现

### 编写参数操作节点

在 `hello_world/scripts/` 下创建 `param_demo_p.py`：

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

import rospy

if __name__ == "__main__":
    rospy.init_node("param_demo_p")

    # ============ 写参数 ============
    rospy.set_param("type", "turtlesim")         # string
    rospy.set_param("radius", 0.5)               # float
    rospy.set_param("wheels", 4)                 # int
    rospy.set_param("use_sim", True)             # bool

    rospy.loginfo("参数已设置")

    # ============ 读参数（带默认值）============
    type_val = rospy.get_param("type", "default_type")
    radius_val = rospy.get_param("radius", 1.0)
    wheels_val = rospy.get_param("wheels", 2)

    rospy.loginfo("读取参数: type=%s, radius=%.2f, wheels=%d",
                  type_val, radius_val, wheels_val)

    # ============ 检查参数是否存在 ============
    if rospy.has_param("type"):
        rospy.loginfo("参数 type 存在")

    # ============ 删除参数 ============
    rospy.delete_param("radius")
    rospy.loginfo("参数 radius 已删除")

    # ============ 读取所有参数 ============
    params = rospy.get_param_names()
    rospy.loginfo("当前参数数量: %d" % len(params))
```

**代码逐行解释：**

第 11-14 行使用 `rospy.set_param()` 写入参数。Python 版本会自动推断类型，无需模板参数。

第 18-20 行使用 `rospy.get_param()` 读取参数。第二个参数是默认值，如果参数不存在则返回默认值，不会抛出异常。

第 24-25 行使用 `rospy.has_param()` 检查参数是否存在。

第 28 行使用 `rospy.delete_param()` 删除参数。

第 31 行使用 `rospy.get_param_names()` 获取所有参数名的列表。

### 进阶：列表和字典参数

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

import rospy

if __name__ == "__main__":
    rospy.init_node("param_advanced_p")

    # 设置列表参数
    rospy.set_param("names", ["Alice", "Bob", "Charlie"])

    # 设置字典参数
    rospy.set_param("pid", {"p": 1.0, "i": 0.1, "d": 0.01})

    # 读取列表
    names = rospy.get_param("names")
    for i, name in enumerate(names):
        rospy.loginfo("names[%d] = %s" % (i, name))

    # 读取字典
    pid = rospy.get_param("pid")
    rospy.loginfo("PID: p=%.2f, i=%.2f, d=%.2f" %
                  (pid["p"], pid["i"], pid["d"]))
```

### 添加权限并运行

```bash
chmod +x ~/ros_demo/src/hello_world/scripts/param_demo_p.py

rosrun hello_world param_demo_p.py
```

---

## 2.3.4 命令行操作参数

除了代码操作，ROS 还提供了 `rosparam` 命令行工具：

```bash
# 设置参数
rosparam set /type "turtlesim"
rosparam set /radius 0.5

# 读取参数
rosparam get /type
rosparam get /          # 读取所有参数

# 检查参数是否存在
rosparam list

# 删除参数
rosparam delete /radius

# 从 YAML 文件批量加载
rosparam load params.yaml

# 导出所有参数到 YAML 文件
rosparam dump params.yaml
```

**YAML 文件示例**（`params.yaml`）：

```yaml
type: turtlesim
radius: 0.5
wheels: 4
use_sim: true
pid:
  p: 1.0
  i: 0.1
  d: 0.01
names:
  - Alice
  - Bob
  - Charlie
```

---

## 2.3.5 Launch 文件中操作参数

launch 文件提供了两种设置参数的方式：

### `<param>` 标签

设置单个参数，作用域由位置决定：

```xml
<launch>
    <!-- 全局参数（在所有节点外） -->
    <param name="robot_name" value="turtlesim"/>
    <param name="max_speed" value="1.0"/>

    <!-- 私有参数（在 node 标签内，自动加节点名前缀） -->
    <node pkg="hello_world" type="my_node" name="my_node">
        <!-- 实际参数名为 /my_node/speed -->
        <param name="speed" value="0.5"/>
        <param name="use_sim" value="true"/>
    </node>
</launch>
```

### `<rosparam>` 标签

批量加载 YAML 文件中的参数：

```xml
<launch>
    <!-- 从 YAML 文件批量加载 -->
    <rosparam file="$(find hello_world)/config/params.yaml" command="load"/>

    <!-- 设置字典参数 -->
    <rosparam>
        pid:
            p: 1.0
            i: 0.1
            d: 0.01
        names: ["Alice", "Bob"]
    </rosparam>

    <!-- 删除参数 -->
    <rosparam param="old_param" command="delete"/>
</launch>
```

> 💡 **`<param>` vs `<rosparam>`**：
> - `<param>`：设置单个参数，value 属性指定值
> - `<rosparam>`：批量操作，可以从 YAML 文件加载，也可以直接写 YAML 格式

---

## 本节小结

| 操作 | C++ | Python | 命令行 |
|------|-----|--------|--------|
| 写参数 | `nh.setParam("key", value)` | `rospy.set_param("key", value)` | `rosparam set /key value` |
| 读参数 | `nh.param<T>("key", var, default)` | `rospy.get_param("key", default)` | `rosparam get /key` |
| 检查存在 | `nh.hasParam("key")` | `rospy.has_param("key")` | — |
| 删除参数 | `nh.deleteParam("key")` | `rospy.delete_param("key")` | `rosparam delete /key` |
| 批量加载 | — | — | `rosparam load file.yaml` |
| 导出参数 | — | — | `rosparam dump file.yaml` |

---

## 理解检查

学完本节后，你应该能够：

- ✅ 描述参数服务器的全局共享特性
- ✅ 区分参数服务器和话题通信的适用场景
- ✅ 用 C++ 读写参数
- ✅ 用 Python 读写参数
- ✅ 使用 rosparam 命令行工具
- ✅ 在 launch 文件中设置参数
- ✅ 理解全局参数、私有参数、相对参数的区别

---

## 🔧 动手练习

1. **参数共享实验**：启动两个节点，一个设置参数，另一个读取参数
2. **YAML 加载实验**：创建一个 YAML 文件，包含机器人的各种参数，用 launch 文件加载
3. **私有参数实验**：在同一个 launch 文件中启动两个相同节点，用私有参数设置不同的速度
4. **动态修改实验**：运行节点过程中，用 `rosparam set` 修改参数，观察节点行为变化

---

## 下一节

参数服务器适合全局配置管理。接下来学习 ROS 常用命令，用于调试和查看系统状态：

→ **[2.4 常用命令](../../chapter02/09-commands.md)**
