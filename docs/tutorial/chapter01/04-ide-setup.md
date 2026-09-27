# 1.4 ROS 集成开发环境搭建

> 本节学习目标：搭建高效的 ROS 开发环境，包括终端工具和 VS Code IDE，并演示 launch 文件的使用。

---

## 1.4.1 安装终端

Ubuntu 默认的终端功能有限，推荐安装 **Terminator** 终端，支持分屏、标签页等功能，非常适合 ROS 开发（经常需要开多个终端）。

### 安装 Terminator

```bash
sudo apt install -y terminator
```

### 基本使用

```bash
# 启动 Terminator
terminator
```

**常用快捷键：**

| 快捷键 | 功能 |
|--------|------|
| `Ctrl + Shift + O` | 水平分屏（上下两半） |
| `Ctrl + Shift + E` | 垂直分屏（左右两半） |
| `Ctrl + Shift + W` | 关闭当前窗格 |
| `Ctrl + Shift + T` | 新建标签页 |
| `Alt + 方向键` | 切换窗格 |
| `Ctrl + Shift + S` | 滚动条切换 |

> 💡 **ROS 开发典型布局**：一个窗格运行 `roscore`，一个运行发布者，一个运行订阅者，一个运行调试命令。Terminator 的分屏功能正好满足这个需求。

### 可选：安装 tmux（终端复用器）

如果你更喜欢在 SSH 远程开发时使用终端复用器：

```bash
sudo apt install -y tmux
```

```bash
# 启动 tmux
tmux

# 水平分屏：Ctrl+B 然后按 "
# 垂直分屏：Ctrl+B 然后按 %
# 切换窗格：Ctrl+B 然后按方向键
# 分离会话：Ctrl+B 然后按 d
# 重新连接：tmux attach
```

---

## 1.4.2 安装 VS Code

VS Code 是目前最流行的 ROS 开发 IDE，轻量、插件丰富、支持调试。

### 安装方式

**方式 1：Snap 安装（推荐）**

```bash
sudo snap install code --classic
```

**方式 2：deb 包安装**

```bash
# 下载 deb 包
wget -qO- https://packages.microsoft.com/keys/microsoft.asc | gpg --dearmor > packages.microsoft.gpg
sudo install -o root -g root -m 644 packages.microsoft.gpg /etc/apt/trusted.gpg.d/
sudo sh -c 'echo "deb [arch=amd64] https://packages.microsoft.com/repos/code stable main" > /etc/apt/sources.list.d/vscode.list'
sudo apt update
sudo apt install -y code
```

**方式 3：Ubuntu Software 图形安装**

在 Ubuntu 应用商店搜索 "Visual Studio Code"，点击安装。

### 安装 ROS 相关插件

启动 VS Code，按 `Ctrl + Shift + X` 打开扩展面板，搜索并安装以下插件：

| 插件名称 | 作用 |
|---------|------|
| **ROS** | 微软官方 ROS 插件，支持语法高亮、launch 文件调试、节点管理等 |
| **C/C++** | C++ 智能提示、调试 |
| **Python** | Python 智能提示、调试 |
| **CMake** | CMakeLists.txt 语法高亮和补全 |
| **XML Tools** | launch 文件是 XML 格式，此插件提供格式化、校验功能 |
| **vscode-icons** | 文件图标美化，方便区分文件类型 |

### 配置 ROS 环境

VS Code 需要知道 ROS 的环境变量才能正常工作：

```bash
# 在 VS Code 的 settings.json 中添加
{
    "terminal.integrated.env.linux": {
        "PATH": "/opt/ros/noetic/bin:${env:PATH}",
        "PYTHONPATH": "/opt/ros/noetic/lib/python3/dist-packages"
    }
}
```

或者更简单的方式：**在已经 source 过 ROS 环境的终端中启动 VS Code**：

```bash
source /opt/ros/noetic/setup.bash
code
```

### 打开 ROS 项目

```bash
# 在终端中打开工作空间
cd ~/ros_demo
code .
```

此时 VS Code 左侧会显示工作空间目录结构，你可以直接编辑代码。

### 配置 C++ 智能提示

在项目根目录创建 `.vscode/c_cpp_properties.json`：

```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "/opt/ros/noetic/include",
                "${workspaceFolder}/**"
            ],
            "defines": [],
            "compilerPath": "/usr/bin/g++",
            "cStandard": "c11",
            "cppStandard": "c++14"
        }
    ],
    "version": 4
}
```

这样在编写 C++ 节点时，`ros/ros.h`、`std_msgs/String.h` 等头文件就能正确找到，智能提示也能正常工作。

### 配置 Python 智能提示

在 `.vscode/settings.json` 中添加：

```json
{
    "python.pythonPath": "/usr/bin/python3",
    "python.autoComplete.extraPaths": [
        "/opt/ros/noetic/lib/python3/dist-packages"
    ]
}
```

---

## 1.4.3 launch 文件演示

launch 文件是 ROS 中用于**批量启动节点**的 XML 配置文件。在第 1.3 节我们已经简单使用过，这里进行更详细的演示。

### 创建 launch 文件

```bash
mkdir -p ~/ros_demo/src/hello_world/launch
cd ~/ros_demo/src/hello_world/launch
touch start_demo.launch
```

### 编写 launch 文件

```xml
<launch>
    <!-- 启动 roscore（可选，roslaunch 会自动启动） -->
    <!-- <node pkg="roscore" type="roscore" name="roscore"/> -->

    <!-- 启动 C++ 发布者 -->
    <node pkg="hello_world" type="talker" name="talker" output="screen"/>

    <!-- 启动 C++ 订阅者 -->
    <node pkg="hello_world" type="listener" name="listener" output="screen"/>

    <!-- 启动 Python 发布者 -->
    <node pkg="hello_world" type="talker_p.py" name="talker_p" output="screen"/>

    <!-- 启动 Python 订阅者 -->
    <node pkg="hello_world" type="listener_p.py" name="listener_p" output="screen"/>
</launch>
```

### launch 文件标签详解

| 标签 | 作用 | 属性 |
|------|------|------|
| `<launch>` | 根标签，所有节点必须放在里面 | — |
| `<node>` | 声明一个节点 | `pkg`（包名）、`type`（可执行文件名）、`name`（节点名）、`output`（日志输出方式） |

**`<node>` 常用属性：**

| 属性 | 说明 | 默认值 |
|------|------|--------|
| `pkg` | 节点所属的功能包 | 必填 |
| `type` | 可执行文件名（C++ 是 CMake 目标名，Python 是脚本名） | 必填 |
| `name` | 节点在 ROS 中的运行时名称（唯一标识） | 与 type 相同 |
| `output` | 日志输出方式：`screen`（终端）或 `log`（日志文件） | `log` |
| `args` | 传递给节点的命令行参数 | 无 |
| `respawn` | 节点崩溃后是否自动重启 | `false` |
| `required` | 节点退出时是否终止整个 launch | `false` |
| `ns` | 命名空间 | 无 |
| `launch-prefix` | 节点启动前的前缀命令（如 gdb、valgrind） | 无 |

### 运行 launch 文件

```bash
# roslaunch 会自动启动 roscore（如果还没启动）
roslaunch hello_world start_demo.launch
```

**运行效果：**

```
... logging to /home/user/.ros/log/xxx.log
Checking log disk usage for logging purposes.
...
[ INFO] [1695800000.000000000]: 发布: hello world 0
[ INFO] [1695800000.000000000]: 收到: [hello world 0]
[ INFO] [1695800000.100000000]: 发布: hello world 1
[ INFO] [1695800000.100000000]: 收到: [hello world 1]
...
```

### launch 文件高级功能

**设置参数：**

```xml
<launch>
    <!-- 全局参数 -->
    <param name="max_speed" value="1.0"/>

    <!-- 私有参数（属于节点） -->
    <node pkg="hello_world" type="talker" name="talker">
        <param name="rate" value="5"/>
    </node>
</launch>
```

**命名空间：**

```xml
<launch>
    <!-- 两个同名节点在不同命名空间下，不会冲突 -->
    <group ns="robot1">
        <node pkg="hello_world" type="talker" name="talker"/>
    </group>

    <group ns="robot2">
        <node pkg="hello_world" type="talker" name="talker"/>
    </group>
</launch>
```

**重映射：**

```xml
<launch>
    <!-- 将节点的 /chatter 话题重映射到 /my_chatter -->
    <node pkg="hello_world" type="listener" name="listener">
        <remap from="chatter" to="my_chatter"/>
    </node>
</launch>
```

**条件启动：**

```xml
<launch>
    <arg name="use_sim" default="true"/>

    <!-- 只有当 use_sim 为 true 时才启动 -->
    <node if="$(arg use_sim)" pkg="turtlesim" type="turtlesim_node" name="sim"/>
</launch>
```

> 📖 launch 文件的详细语法将在第 4 章运行管理中深入讲解。

### 在 VS Code 中调试 launch 文件

安装 ROS 插件后，可以在 VS Code 中直接调试 launch 文件：

1. 按 `Ctrl + Shift + D` 打开调试面板
2. 点击 "创建 launch.json 文件"，选择 "ROS"
3. 选择 "ROS: Launch" 模板
4. 在配置中指定 launch 文件路径
5. 按 `F5` 启动调试

---

## 本节小结

| 工具 | 安装命令 | 作用 |
|------|---------|------|
| Terminator | `sudo apt install terminator` | 分屏终端，方便多节点开发 |
| VS Code | `sudo snap install code --classic` | 代码编辑和调试 |
| ROS 插件 | VS Code 扩展面板安装 | ROS 语法高亮、launch 调试 |
| C/C++ 插件 | VS Code 扩展面板安装 | C++ 智能提示和调试 |
| Python 插件 | VS Code 扩展面板安装 | Python 智能提示和调试 |

---

## 理解检查

学完本节后，你应该能够：

- ✅ 安装并使用 Terminator 分屏终端
- ✅ 安装 VS Code 和 ROS 相关插件
- ✅ 配置 VS Code 的 C++ 和 Python 智能提示
- ✅ 编写 launch 文件启动多个节点
- ✅ 理解 launch 文件中 `<node>` 标签的常用属性

---

## 🔧 动手练习

1. **Terminator 分屏**：用 Terminator 分屏，分别运行 roscore、talker、listener
2. **VS Code 打开项目**：用 VS Code 打开 `~/ros_demo`，编辑一个节点代码
3. **launch 文件练习**：编写一个 launch 文件，同时启动 talker 和 listener，并设置不同的命名空间

---

## 下一节

开发环境已就绪，接下来深入了解 ROS 的架构设计：

→ **[1.5 ROS 架构](05-architecture.md)**
