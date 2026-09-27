# ros_demo

ROS（Robot Operating System）入门学习工程 —— 基于 catkin 工作空间的 ROS 1 示例项目。

## 项目简介

本项目是一个 ROS 1 catkin 工作空间，包含 `hello_world` 功能包，用于演示：

- C++ 节点的编写与编译 (`roscpp`)
- Python 节点的编写 (`rospy`)
- Launch 文件批量启动多个节点

## 项目结构

```
ros_demo/                            # catkin 工作空间
├── src/                             # 源代码目录
│   ├── CMakeLists.txt               # 顶层 CMake 配置（catkin 自动生成）
│   └── hello_world/                 # hello_world 功能包
│       ├── package.xml              # 包元信息（名称、版本、依赖）
│       ├── CMakeLists.txt           # 包编译规则
│       ├── src/
│       │   └── helloworld_c.cpp     # C++ 节点源码
│       ├── scripts/
│       │   └── helloworld_p.py      # Python 节点源码
│       └── launch/
│           └── start_turtle.launch  # 启动文件（同时启动 C++/Python 节点）
├── build/                           # 编译中间产物（catkin_make 生成）
├── devel/                           # 开发环境（setup.bash、可执行文件）
├── docs/                           # 在线文档（GitHub Pages 零配置部署）
│   ├── README.md                   # 站点首页
│   ├── arch.md                     # ROS 架构详解
│   ├── comm.md                     # ROS 1 通信机制详解
│   └── ros_cmd.md                  # ROS 常用命令速查手册
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

## 文档

- [ROS 架构详解](docs/arch.md) — 计算图模型、通信机制（Topic/Service/Action/Parameter）、功能包结构、ROS 1 vs ROS 2 对比等
- [ROS 1 通信机制详解](docs/comm.md) — 协议栈、连接建立流程、C++/Python 代码实战、回调机制、问题排查
- [ROS 常用命令速查手册](docs/ros_cmd.md) — 节点/话题/服务/参数/rosbag 等命令详解，含调试场景与使用示例

## 在线文档 (GitHub Pages)

本项目的 `docs/` 目录通过 GitHub Pages 发布，采用**零配置方案**（参考 [google/tcmalloc](https://github.com/google/tcmalloc) 的做法）—— 无需 `_config.yml`、Gemfile 或 Actions 工作流：

1. 仓库页面进入 **Settings → Pages**
2. **Source** 选择 **Deploy from a branch** → 分支 `main` → 目录 **`/docs`** → Save
3. 推送后等待 1~2 分钟，访问 `https://<用户名>.github.io/ros_demo/`

GitHub Pages 内置的 Jekyll 管线会自动完成：`README.md` 作为首页、文档间相对链接 (`xxx.md`) 自动转为 `xxx.html`、套用默认主题样式。以后往 `docs/` 加新 Markdown 文件直接 push 即可发布。

### 本地预览（可选）

```bash
gem install jekyll          # 需先安装 Ruby
cd docs
jekyll serve --baseurl ""
# 访问 http://127.0.0.1:4000（本地样式与线上略有差异，以线上为准）
```

## 后续计划

- [ ] 添加 Topic 发布/订阅示例（Publisher / Subscriber）
- [ ] 添加 Service 请求/响应示例（Server / Client）
- [ ] 添加自定义消息类型 (`.msg`)
- [ ] 添加 turtlesim 小海龟控制示例
- [ ] 添加参数服务器使用示例
