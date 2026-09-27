# ROS 常用命令速查手册

> 适用于 ROS 1 (Melodic / Noetic)。按功能分类整理，含命令解释和使用示例。

---

## 一、系统启动类

### 1.1 roscore — 启动 ROS Master

```bash
roscore
```

**解释**：启动 ROS Master（节点注册与发现中心）+ 参数服务器 + `rosout` 日志节点。任何节点通信前必须先启动 Master（`roslaunch` 会自动启动，无需手动执行）。

### 1.2 rosrun — 运行单个节点

```bash
rosrun <package_name> <node_name>

# 示例：运行 hello_world 包的 C++ 节点
rosrun hello_world hello_node
```

**解释**：从指定功能包中查找并运行一个可执行节点。**缺点**：一次只能启动一个节点，多节点系统请用 `roslaunch`。

### 1.3 roslaunch — 通过 launch 文件批量启动

```bash
roslaunch <package_name> <file.launch>

# 示例：同时启动 hello_world 包中的 C++ 和 Python 节点
roslaunch hello_world start_turtle.launch

# 直接运行包内 launch 文件（无需先 source）
roslaunch hello_world start.launch

# 传递参数
roslaunch hello_world start_turtle.launch use_sim:=true
```

**解释**：解析 `.launch` XML 文件，批量启动多个节点，**自动启动 roscore**（若未运行），支持参数传递、命名空间、条件启动、节点重启等高级特性。

---

## 二、节点（Node）管理类

### 2.1 rosnode list — 列出所有运行中的节点

```bash
rosnode list
```

**输出示例**：
```
/hello_cpp
/hello_python
/rosout
```

**解释**：显示当前所有已注册节点的名称。`/rosout` 是系统日志节点，由 roscore 自动启动。

### 2.2 rosnode info — 查看节点详细信息

```bash
rosnode info <node_name>

# 示例
rosnode info /hello_cpp
```

**解释**：显示节点的发布话题、订阅话题、提供的服务、连接的节点等详细信息。**调试节点连接关系的首选命令**。

### 2.3 rosnode kill — 终止指定节点

```bash
rosnode kill <node_name>

# 示例
rosnode kill /hello_cpp
```

**解释**：向节点发送关闭信号（优雅退出），区别于 `kill -9` 强杀进程。

### 2.4 rosnode cleanup — 清理失效节点

```bash
rosnode cleanup
```

**解释**：删除注册表中已断连但未注销的"僵尸节点"记录。当节点异常退出（如强杀进程）后，`rosnode list` 仍显示它时使用。

---

## 三、话题（Topic）类

### 3.1 rostopic list — 列出所有活跃话题

```bash
rostopic list

# 只显示包含指定关键词的话题
rostopic list | grep cmd
```

**解释**：列出当前系统中所有话题名称。

### 3.2 rostopic echo — 打印话题数据（最常用）

```bash
rostopic echo <topic_name>

# 示例：查看小海龟位姿
rostopic echo /turtle1/pose

# 只显示一次消息
rostopic echo -n 1 /turtle1/pose
```

**解释**：实时订阅并打印话题上的消息内容，**调试数据流的第一选择**（相当于机器人世界的 `tail -f`）。

### 3.3 rostopic hz — 查看话题发布频率

```bash
rostopic hz <topic_name>

# 示例：查看激光雷达数据频率（通常应为 10 Hz）
rostopic hz /scan
```

**解释**：统计消息平均发布频率。用于验证传感器驱动是否正常工作。

### 3.4 rostopic bw — 查看话题带宽

```bash
rostopic bw <topic_name>
```

**解释**：统计话题数据传输带宽（字节/秒），评估网络负载。

### 3.5 rostopic type — 查看话题消息类型

```bash
rostopic type <topic_name>

# 示例
rostopic type /turtle1/cmd_vel
# 输出：geometry_msgs/Twist
```

**解释**：查询话题使用的消息类型，配合 `rosmsg show` 查看消息结构。

### 3.6 rostopic pub — 手动向话题发布消息

```bash
rostopic pub <topic_name> <msg_type> <args>

# 示例：让小海龟以线速度 2.0、角速度 1.8 运动（只发一次）
rostopic pub /turtle1/cmd_vel geometry_msgs/Twist "linear:
  x: 2.0
angular:
  z: 1.8"

# 以 10 Hz 持续发布（-r 频率）
rostopic pub -r 10 /turtle1/cmd_vel geometry_msgs/Twist "linear:
  x: 1.0
angular:
  z: 0.5"
```

**解释**：命令行直接发布消息，**无需写代码即可测试订阅者节点**。`-1`（默认）发一次，`-r <hz>` 按频率持续发。

### 3.7 rostopic info — 查看话题连接信息

```bash
rostopic info <topic_name>

# 示例
rostopic info /turtle1/pose
```

**解释**：显示话题的消息类型、发布者列表、订阅者列表。快速排查"为什么收不到数据"。

---

## 四、消息（Message）类

### 4.1 rosmsg list — 列出所有消息类型

```bash
rosmsg list

# 过滤查找
rosmsg list | grep Twist
```

### 4.2 rosmsg show — 查看消息结构定义

```bash
rosmsg show <msg_type>

# 示例：查看速度消息结构
rosmsg show geometry_msgs/Twist
# 输出：
# geometry_msgs/Vector3 linear
#   float64 x
#   float64 y
#   float64 z
# geometry_msgs/Vector3 angular
#   float64 x
#   float64 y
#   float64 z
```

**解释**：显示消息的字段组成，**编写发布代码前必查**。配合 `rostopic type` 使用：先查类型，再看结构。

### 4.3 rosmsg package — 查看包中所有消息

```bash
rosmsg package <package_name>

# 示例
rosmsg package std_msgs
```

---

## 五、服务（Service）类

### 5.1 rosservice list — 列出所有服务

```bash
rosservice list

# 示例输出
# /clear
# /reset
# /spawn
# /turtle1/set_pen
```

### 5.2 rosservice type — 查看服务类型

```bash
rosservice type <service_name>

# 示例
rosservice type /spawn
# 输出：turtlesim/Spawn
```

### 5.3 rosservice call — 调用服务

```bash
rosservice call <service_name> <args>

# 示例 1：清除小海龟轨迹
rosservice call /clear

# 示例 2：在指定坐标生成新海龟
rosservice call /spawn "x: 5.0
y: 5.0
theta: 0.0
name: 'turtle2'"

# 示例 3：重置仿真
rosservice call /reset
```

**解释**：命令行直接发起服务请求，测试服务端节点。

### 5.4 rosservice args — 查看服务请求参数

```bash
rosservice args <service_name>

# 示例
rosservice args /spawn
# 输出：x y theta name
```

---

## 六、参数（Parameter）类

### 6.1 rosparam list — 列出所有参数

```bash
rosparam list
```

### 6.2 rosparam get — 读取参数

```bash
# 读取单个参数
rosparam get <param_name>
rosparam get /background_r

# 读取所有参数
rosparam get /
```

### 6.3 rosparam set — 设置参数

```bash
rosparam set <param_name> <value>

# 示例：修改小海龟背景色
rosparam set /background_r 255
rosservice call /clear   # 需调用 /clear 使参数生效
```

### 6.4 rosparam load / dump — 参数文件导入/导出

```bash
# 从 YAML 文件加载参数
rosparam load <file.yaml>

# 将参数保存到 YAML 文件
rosparam dump <file.yaml>

# 示例
rosparam dump params.yaml
rosparam load params.yaml
```

---

## 七、功能包（Package）管理类

### 7.1 rospack find — 查找功能包路径

```bash
rospack find <package_name>

# 示例
rospack find hello_world
# 输出：/home/user/ros_demo/src/hello_world
```

### 7.2 rospack list — 列出所有功能包

```bash
rospack list
```

### 7.3 roscd — 快速跳转到包目录

```bash
roscd <package_name>

# 示例
roscd hello_world
roscd hello_world/launch   # 直接跳到子目录
```

### 7.4 rosls — 列出包目录内容

```bash
rosls <package_name>

# 示例
rosls hello_world
# 输出：CMakeLists.txt  launch  package.xml  scripts  src
```

### 7.5 rosed — 直接编辑包内文件

```bash
rosed <package_name> <filename>

# 示例：编辑 hello_world 包的 package.xml（无需知道完整路径）
rosed hello_world package.xml

# 文件名支持 Tab 补全
rosed hello_world helloworld_c.cpp
```

**解释**：在不知道文件完整路径的情况下，直接用编辑器打开功能包内的文件。默认使用 `vim`，可通过环境变量修改默认编辑器：

```bash
# 临时修改（当前终端有效）
export EDITOR=nano

# 永久生效
echo "export EDITOR=nano" >> ~/.bashrc
```

> `roscd`、`rosls`、`rosed` 同属 **rosbash** 工具集，是操作功能包的三大快捷命令。

### 7.6 catkin_create_pkg — 创建新功能包

```bash
catkin_create_pkg <package_name> [dependencies]

# 示例：创建依赖 roscpp、rospy、std_msgs 的新包
catkin_create_pkg my_pkg roscpp rospy std_msgs
```

**解释**：在 `src/` 目录下执行，自动生成 `package.xml` 和 `CMakeLists.txt` 模板。

---

## 八、工作空间与构建类

### 8.1 catkin_make — 编译工作空间

```bash
cd ~/ros_demo
catkin_make            # 编译所有包

# 只编译指定包
catkin_make --only-pkg-with-deps hello_world

# 强制重新编译
catkin_make --force-cmake
```

### 8.2 source — 加载环境变量

```bash
source devel/setup.bash

# 追加到 ~/.bashrc 永久生效
echo "source ~/ros_demo/devel/setup.bash" >> ~/.bashrc
```

**解释**：将工作空间的路径注册到 `ROS_PACKAGE_PATH` 等环境变量，使系统能找到你的包。**每次编译后新终端必须重新 source**（或写入 bashrc）。

### 8.3 环境变量检查

```bash
echo $ROS_PACKAGE_PATH     # 查看包搜索路径
echo $ROS_MASTER_URI       # 查看 Master 地址（默认 http://localhost:11311）
echo $ROS_HOSTNAME         # 查看本机在 ROS 网络中的名称
```

---

## 九、数据录制与回放类（rosbag）

### 9.1 rosbag record — 录制话题数据

```bash
# 录制指定话题
rosbag record /turtle1/cmd_vel /turtle1/pose

# 录制所有话题
rosbag record -a

# 指定输出文件名
rosbag record -o my_data /turtle1/cmd_vel
```

### 9.2 rosbag play — 回放数据

```bash
rosbag play <bag_file>

# 示例
rosbag play my_data_2026-09-26-15-30-00.bag

# 以 2 倍速回放
rosbag play -r 2 my_data.bag

# 循环回放
rosbag play -l my_data.bag
```

### 9.3 rosbag info — 查看录制文件信息

```bash
rosbag info <bag_file>
```

**解释**：显示录制时长、消息数量、包含的话题及频率，回放前先检查。

---

## 十、调试与可视化工具

### 10.1 rqt_graph — 节点关系图

```bash
rosrun rqt_graph rqt_graph
# 或直接
rqt_graph
```

**解释**：图形化显示节点—话题连接关系图，**排查通信拓扑的神器**。

### 10.2 rviz — 3D 可视化

```bash
rviz
```

**解释**：3D 可视化平台，显示传感器数据（点云、图像、激光）、机器人模型（URDF）、TF 坐标系、规划路径等。

### 10.3 rqt 常用插件

```bash
rqt_console          # 日志查看器（分级过滤）
rqt_plot             # 实时曲线绘制
rqt_image_view       # 图像查看器
rqt_tf_tree          # TF 坐标树查看
rqt                  # 启动 rqt 集成环境
```

### 10.4 rosrun tf — 坐标变换工具

```bash
# 查看任意两个坐标系间的变换关系
rosrun tf tf_echo /map /base_link

# 查看坐标树结构
rosrun tf view_frames          # 生成 frames.pdf
rosrun rqt_tf_tree rqt_tf_tree # 图形化实时查看
```

---

## 十一、命令组合使用场景

### 场景 1：调试"订阅者收不到数据"

```bash
rostopic list                    # 1. 话题存在吗？
rostopic info /my_topic          # 2. 发布者和订阅者都连上了吗？
rostopic hz /my_topic            # 3. 数据在正常发送吗？
rostopic echo /my_topic          # 4. 数据内容对吗？
rostopic type /my_topic          # 5. 消息类型匹配吗？
rosmsg show <查到的类型>          # 6. 字段结构对吗？
rqt_graph                        # 7. 用图形确认整体连接
```

### 场景 2：测试一个新写的订阅者节点

```bash
# 终端 1：启动被测节点
rosrun my_pkg my_subscriber

# 终端 2：手动发布测试数据
rostopic pub -r 10 /my_topic std_msgs/String "data: 'test message'"
```

### 场景 3：复现实验现场

```bash
# 实验时录制
rosbag record -a

# 事后回放分析
rosbag play recorded.bag
rviz    # 配合可视化分析
```

---

## 十二、命令速查总表

| 命令 | 作用 | 使用频率 |
|------|------|:---:|
| `roscore` | 启动 Master | ⭐⭐⭐ |
| `rosrun <pkg> <node>` | 运行单节点 | ⭐⭐⭐ |
| `roslaunch <pkg> <file>` | 批量启动节点 | ⭐⭐⭐ |
| `rostopic list` | 列出话题 | ⭐⭐⭐ |
| `rostopic echo <topic>` | 打印话题数据 | ⭐⭐⭐ |
| `rostopic pub ...` | 发布测试消息 | ⭐⭐⭐ |
| `rostopic hz <topic>` | 查看话题频率 | ⭐⭐ |
| `rostopic type <topic>` | 查看消息类型 | ⭐⭐ |
| `rosmsg show <type>` | 查看消息结构 | ⭐⭐⭐ |
| `rosnode list` | 列出节点 | ⭐⭐⭐ |
| `rosnode info <node>` | 查看节点详情 | ⭐⭐ |
| `rosservice list` | 列出服务 | ⭐⭐ |
| `rosservice call ...` | 调用服务 | ⭐⭐ |
| `rosparam get/set` | 读写参数 | ⭐⭐ |
| `roscd <pkg>` | 跳转包目录 | ⭐⭐⭐ |
| `rosls <pkg>` | 列出包内容 | ⭐⭐ |
| `rosed <pkg> <file>` | 编辑包内文件 | ⭐⭐ |
| `rospack find <pkg>` | 查找包路径 | ⭐⭐ |
| `catkin_make` | 编译工作空间 | ⭐⭐⭐ |
| `source devel/setup.bash` | 加载环境 | ⭐⭐⭐ |
| `rosbag record/play` | 录制/回放数据 | ⭐⭐ |
| `rqt_graph` | 节点关系图 | ⭐⭐⭐ |
| `rviz` | 3D 可视化 | ⭐⭐⭐ |
