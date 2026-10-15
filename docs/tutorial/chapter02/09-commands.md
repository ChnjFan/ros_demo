# 2.4 常用命令

> 本节学习目标：掌握 rosnode、rostopic、rosmsg、rosservice、rossrv、rosparam 六大命令族的用法，并能用它们检查一个正在运行的 ROS 系统。

---

## 2.4.1 为什么命令行工具值得单独学一节

前三个小节里，我们写了发布者、订阅者、服务端、客户端，每次运行都要开好几个终端。代码能跑起来，但你对系统的了解全靠 `ros::loginfo` 打印出来的那几行字。

问题是：节点一旦跑起来，它就变成了一个黑盒。话题上到底有没有数据？谁订阅了谁？服务端卡住了还是在正常工作？参数被改成了什么？

命令行工具就是打开这个黑盒的接口。它们的价值不在于"能列出话题"，而在于构成一个可重复的排查流程：

```mermaid
flowchart LR
    A["rosnode list<br/>系统里有什么?"] --> B["rosnode info<br/>谁连着谁?"]
    B --> C["rostopic list / hz<br/>数据在流吗?"]
    C --> D["rostopic echo<br/>数据内容对吗?"]
    D --> E["rosmsg show<br/>字段结构对吗?"]
```

这套流程在真实项目里每天都会用到，而且顺序很重要。直接 `rostopic echo` 一个不存在的话题只会得到一句 `ERROR: Cannot load message class`，白跑一趟。按流程走，你能在四步之内定位到问题出在注册、连接、还是数据本身。

> 💡 **本节所有命令都可以在本项目的 `plumbing_pub_sub` 包上直接练习**。先按 2.5 节把节点跑起来，再回来执行这些命令，比单纯读输出示例有用得多。

---

## 2.4.2 rosnode：查看节点

节点是 ROS 计算图里的执行单元，`rosnode` 命令族负责回答"有哪些节点、它们长什么样"。

### rosnode list：列出所有节点

```bash
rosnode list
```

输出示例（跑起 2.1 节的发布者和订阅者之后）：

```
/listener
/rosout
/talker
```

`/rosout` 不是你的代码，它是 `roscore` 启动时自动创建的日志聚合节点。所有节点的 `ROS_INFO` 一类日志都会被它收集，`rqt_console` 读的就是它。**看到 `/rosout` 说明 `roscore` 活着**，这也是排查问题的第一个信号。

### rosnode info：查看节点详情

```bash
rosnode info /talker
```

部分输出：

```
Node [/talker]
Publications:
 * /chatter [std_msgs/String]
 * /rosout [rosgraph_msgs/Log]

Subscriptions:
 * /clock [rosgraph_msgs/Clock]

Services:
 * /talker/get_loggers
 * /talker/set_logger_level

contacting node http://localhost:41235/ ...
Pid: 12345
Connections:
 * topic: /rosout
    * to: /rosout
    * direction: outbound
 * topic: /chatter
    * to: /listener (http://localhost:41321/)
    * direction: outbound
```

这条命令信息量很大，值得逐块看：

| 区块 | 含义 | 排查用途 |
|------|------|---------|
| `Publications` | 本节点发布的话题 | 话题名、消息类型是否符合预期 |
| `Subscriptions` | 本节点订阅的话题 | 订阅没生效时看这里，通常说明代码里话题名写错了 |
| `Services` | 本节点提供的服务 | 除自定义服务外，每个节点自动带 `get_loggers` / `set_logger_level` |
| `Pid` | 操作系统进程号 | 需要 `kill` 或 `top` 时用得上 |
| `Connections` | 当前已建立的连接及方向 | **判断"发布者和订阅者有没有真正连上"** |

`Connections` 是这里最有价值的一段。`Publications` 只说明节点**声明**要发布 `/chatter`，而 `Connections` 说明**真的有人连上来了**。如果 `Publications` 里有 `/chatter`，但 `Connections` 里没有 `/listener`，那问题就出在订阅端，跟发布端无关。

### rosnode kill 与 rosnode cleanup

```bash
rosnode kill /talker
```

`rosnode kill` 会向目标节点发送关闭请求（等价于在该节点的终端按下 `Ctrl+C`），节点有机会执行清理逻辑后退出。它比 `kill -9 <pid>` 温和得多，**不要用 `kill -9`**：强杀会让节点来不及向 Master 注销。

来不及注销的后果就是"僵尸节点"：

```bash
# 节点进程已经没了，但注册信息还挂在 Master 上
rosnode list
# /talker      ← 假的
# /rosout

rosnode cleanup
# 交互确认后清理掉这些失效记录
```

`rosnode cleanup` 在节点崩溃、终端被强制关闭之后特别有用。它只清理注册表，不会去动任何活着的进程。

---

## 2.4.3 rostopic：查看话题

`rostopic` 是平时用得最多的一组命令。

### rosnode list 之外的第一件事：话题全貌

```bash
rostopic list
```

```
/chatter
/rosout
/rosout_agg
```

`/rosout_agg` 是 `/rosout` 的聚合版本，同样是系统自带。看到陌生话题名，先想想是不是某个节点的命名空间或重映射造成的，这一点在第 4 章会展开。

### rostopic type 与 rosmsg show：先查类型，再查结构

```bash
rostopic type /chatter
# std_msgs/String

rosmsg show std_msgs/String
# string data

rostopic type /turtle1/cmd_vel
# geometry_msgs/Twist

rosmsg show geometry_msgs/Twist
# geometry_msgs/Vector3 linear
#   float64 x
#   float64 y
#   float64 z
# geometry_msgs/Vector3 angular
#   float64 x
#   float64 y
#   float64 z
```

这两条命令要连着用。`rostopic type` 告诉你话题的消息类型，`rosmsg show` 告诉你这个类型里有哪些字段。**写发布代码或手动发消息之前，先用这两条命令确认字段名和类型**，能省掉大量编译期和运行期的报错。

`rostopic type` 加 `-v` 还能一次看到类型的完整定义来源：

```bash
rostopic type /chatter -v
```

### rostopic hz 与 rostopic bw：数据流健康度

```bash
rostopic hz /chatter
# subscribed to [/chatter]
# average rate: 9.998
#   min: 0.100s max: 0.101s std dev: 0.00025s window: 10
```

`hz` 统计的是**从你执行命令那一刻起**收到的消息频率。它有两个容易踩的坑：

- **它测的是你的终端，不是发布者**。如果你在发布者启动十秒后才执行 `hz`，前面那段数据是收不到的，但这不影响结果，频率统计只关心间隔。
- **Python 发布者常常测不到标称频率**。`rospy.Rate(10)` 配合 `rospy.sleep()` 的实际频率通常是 9.5 Hz 左右，误差来自 Python 的循环调度开销。C++ 的 `ros::Rate` 同样有偏差，只是更小。看到 9.99 还是 9.5，都属于正常范围。

```bash
rostopic bw /chatter
# subscribed to [/chatter]
# average: 20.00KB/s
#   mean: 0.20KB min: 0.20KB max: 0.20KB window: 100
```

`bw` 测带宽，在评估网络负载时才有意义。本地回环测试里它只是验证数据量级。

### rostopic echo：看数据内容

```bash
rostopic echo /chatter
# data: "hello world 12"
# ---
# data: "hello world 13"
# ---
```

只取一条然后退出：

```bash
rostopic echo -n 1 /chatter
```

`-n` 参数在脚本里很有用，配合 `timeout` 还能防止命令挂死：

```bash
timeout 3 rostopic echo -n 1 /chatter || echo "3 秒内没有收到数据"
```

**这条组合命令是判断"话题上到底有没有数据"的最快方式**，比开一个终端盯着看方便。`timeout` 的退出码非零会触发 `||` 分支，于是没数据的情况有了明确提示。

### rostopic info：连接关系

```bash
rostopic info /chatter
# Type: std_msgs/String
#
# Publishers:
#  * /talker (http://localhost:41235/)
#
# Subscribers:
#  * /listener (http://localhost:41321/)
```

`rostopic info` 和 `rosnode info` 的 `Connections` 区块回答的是同一个问题，只是视角不同：前者从话题看两端，后者从节点看连线。**排查"收不到数据"时两个都值得看一眼**：

- `Publishers` 为空 → 发布者没起来，或者话题名不一致
- `Subscribers` 为空 → 订阅者没起来，或者话题名不一致
- 两端都非空但收不到数据 → 看 `rostopic hz`，可能是发布频率极低或消息类型在两端不一致

### rostopic pub：不写代码发消息

这是命令行工具里最"能打"的一条：**你可以在不写一行代码的情况下测试任何订阅者**。

```bash
# 发一条
rostopic pub /chatter std_msgs/String "data: 'hello from cli'"

# 以 10 Hz 持续发（Ctrl+C 停止）
rostopic pub -r 10 /chatter std_msgs/String "data: 'hello from cli'"
```

消息内容用 YAML 语法书写。嵌套字段直接换行缩进，也支持流式写法：

```bash
# 缩进写法
rostopic pub -r 10 /turtle1/cmd_vel geometry_msgs/Twist "linear:
  x: 1.0
  y: 0.0
  z: 0.0
angular:
  x: 0.0
  y: 0.0
  z: 0.5"

# 流式写法，效果完全一样
rostopic pub -r 10 /turtle1/cmd_vel geometry_msgs/Twist "{linear: {x: 1.0}, angular: {z: 0.5}}"
```

两个细节值得记住：

- **`-1` 是默认值**，表示只发一条。`rostopic pub -1` 和 `rostopic pub` 等价。想持续发必须显式指定 `-r`。
- **自动补全**。输入 `rostopic pub /chatter std_msgs/String ` 后按两次 `Tab`，shell 会补全出一个包含所有字段的模板。自定义消息需要先 `source devel/setup.bash` 才能补全，因为模板是从生成的 Python 消息类里读的。

> ⚠️ **消息里含 `[]` 或 `{}` 时**，shell 的通配符展开可能吃掉它们。用单引号包住整个消息内容（上面的例子都加了引号），或者干脆用缩进写法。

### rostopic 其余子命令

| 子命令 | 用途 |
|--------|------|
| `rostopic find <类型>` | 反查使用某消息类型的所有话题 |
| `rostopic delay <话题>` | 显示消息头时间戳与当前时间的延迟，需要消息带 `Header` |
| `rostopic echo -p <话题>` | 以 CSV 格式输出，方便导入 Excel 或绘图 |

`rostopic delay` 在排查传感器时间戳异常时很有用，但它依赖消息里的 `Header` 字段。`std_msgs/String` 没有 `Header`，用了也没输出。

---

## 2.4.4 rosmsg：查看消息类型

`rosmsg` 专门跟 `.msg` 定义打交道。

```bash
# 列出系统中所有消息类型（几十上百个，建议配合 grep）
rosmsg list | grep Twist

# 查看某个消息的字段结构
rosmsg show geometry_msgs/Twist

# 查看某个包里的所有消息
rosmsg package plumbing_pub_sub
# plumbing_pub_sub/Person

# 反查：这个类型被哪些包定义
rosmsg packages std_msgs

# 查看消息的完整源文件路径
rosmsg show -r plumbing_pub_sub/Person
```

自定义消息编译完成后就能被 `rosmsg` 找到：

```bash
rosmsg show plumbing_pub_sub/Person
# string name
# uint8 age
# float64 height
```

如果这条命令报 `Cannot find message`，说明三件事之一出了问题：包没编译、`.msg` 没写进 `add_message_files`、或者当前终端没有 `source devel/setup.bash`。第三种最常见。

---

## 2.4.5 rosservice 与 rossrv：查看和调用服务

### rosservice list 与 rosservice type

```bash
rosservice list
# /add_int
# /rosout/get_loggers
# /rosout/set_logger_level
# /talker/get_loggers
# /talker/set_logger_level
```

`<节点名>/get_loggers` 和 `<节点名>/set_logger_level` 是每个节点自动提供的服务，不是你的代码。剩下的才是自定义服务。

```bash
rosservice type /add_int
# plumbing_server_client/AddInt

rosservice args /add_int
# num1 num2
```

`rosservice args` 只列出请求字段名，不看类型。要看完整定义得用 `rossrv`：

```bash
rossrv show plumbing_server_client/AddInt
# int32 num1
# int32 num2
# ---
# int32 sum
```

`---` 是请求与响应的分隔线，这是 `.srv` 文件的语法，也就解释了为什么 `rossrv show` 的输出和 `rosmsg show` 长得不一样。

### rosservice call：命令行调用服务

```bash
# 无参数的服务
rosservice call /clear

# 带参数（推荐写法：直接写 YAML）
rosservice call /add_int "{num1: 3, num2: 5}"
# sum: 8

# 等价的分行写法
rosservice call /add_int "num1: 3
num2: 5"

# 老版本的按位置传参（Noetic 仍支持，但不推荐）
rosservice call /add_int 3 5
```

`rosservice call` 会**阻塞直到服务端返回响应**。这正是服务通信与话题通信最直观的差别：命令行会停在那里等结果，而 `rostopic pub` 发完就返回。

调用一个不存在的服务会立刻报错：

```bash
rosservice call /add_int_not_exist "{num1: 1, num2: 1}"
# ERROR: Unable to load type [plumbing_server_client/AddInt_not_exist].
```

报错信息里带的是服务名而不是类型名，有点误导，但意思就是"这个服务找不到"。

### rosservice call 与 rosservice info

```bash
rosservice info /add_int
# Node: /add_int_server
# URI: rosrpc://localhost:45678
# Type: plumbing_server_client/AddInt
# Args: num1 num2
```

`Node` 字段告诉你谁在提供这个服务，服务端进程没起来时会直接报错。

---

## 2.4.6 rosparam：操作参数服务器

```bash
rosparam list
# /background_b
# /background_g
# /background_r
# /rosdistro
# /rosversion
# /run_id
```

`/rosdistro`、`/rosversion`、`/run_id` 是 `roscore` 写入的系统参数。`/run_id` 每次启动都不同，可以用来确认"我这个终端连的是哪个 Master"。

```bash
# 读参数
rosparam get /background_r
# 255

# 读全部（输出是 YAML）
rosparam get /

# 写参数
rosparam set /background_r 100

# 删除参数
rosparam delete /background_r

# 检查是否存在（配合 get 的返回码）
rosparam get /background_r 2>/dev/null && echo "存在" || echo "不存在"
```

小海龟背景色是个经典例子，但它有个容易忘的前提：**改完参数后要调用 `/clear` 服务刷新画面**。

```bash
rosparam set /background_r 255
rosparam set /background_g 0
rosparam set /background_b 0
rosservice call /clear     # ← 这一句不能少
```

参数服务器的读写是**即时生效**的，没有缓存也没有广播机制。节点在启动时读一次参数，之后你改了参数，节点是不知道的，除非它自己周期性去读，或者用第 10 章的动态参数机制。这一点在 2.6 节的选型讨论里会再强调一次。

### YAML 导入导出

```bash
# 导出所有参数
rosparam dump params.yaml

# 从文件加载
rosparam load params.yaml

# 加载到指定命名空间
rosparam load params.yaml /my_robot
```

`rosparam dump` 导出的是完整 YAML，可以手工编辑后再 `load` 回去。这是**保存一套调参结果**最省事的办法：调好了就 dump，下次 load 回来，不用把手敲的命令再执行一遍。

---

## 2.4.7 场景实战：三个排查流程

命令单独看都很简单，组合起来才能解决问题。下面是三个真实会遇到的现象。

### 场景一：订阅者收不到数据

```bash
# 1. 话题存在吗？
rostopic list | grep chatter

# 2. 两端都注册了吗？
rostopic info /chatter

# 3. 真的有数据在流吗？
timeout 3 rostopic hz /chatter

# 4. 数据内容对吗？
timeout 3 rostopic echo -n 1 /chatter

# 5. 消息类型匹配吗？
rostopic type /chatter
```

五步走完，问题必然落在某一环上：

| 现象 | 定位 |
|------|------|
| 第 1 步就没有话题 | 发布者没起来，或者话题名写错 |
| 有话题但 `Publishers` 为空 | 发布者崩了，看它的终端输出 |
| 有发布者但 `Subscribers` 为空 | 订阅者没起来，或话题名不匹配 |
| 两端都有但 `hz` 无输出 | 发布者逻辑里没走到 `publish()`，检查循环条件 |
| 有数据但订阅者没打印 | 回调函数有问题，或消息类型两端不一致 |

### 场景二：单独测试一个订阅者

不需要写发布者，直接用手动发布把订阅者喂饱：

```bash
# 终端 1：启动被测节点
rosrun plumbing_pub_sub sub_person_node

# 终端 2：手动发一条 Person 消息
rostopic pub -1 /person_info plumbing_pub_sub/Person "{name: 'zhangsan', age: 20, height: 1.75}"
```

这是**开发订阅者节点时的标准做法**。先把 `rostopic pub` 调通，再写发布者，能明确区分"订阅逻辑错"和"发布逻辑错"。

### 场景三：确认自己连的是哪个 Master

多机或者同时开了多个 `roscore` 时，你的终端连的可能不是你以为的那个。

```bash
echo $ROS_MASTER_URI        # Master 地址
echo $ROS_IP / $ROS_HOSTNAME # 本机在 ROS 网络中的标识
rosparam get /run_id        # 当前 Master 的本次运行 ID
```

在 A 终端执行 `rosparam get /run_id`，在 B 终端也执行一次。**两个 ID 不一样，说明它们连的不是同一个 Master**，此时在 A 里发布的消息，B 永远收不到。

---

## 本节小结

| 命令族 | 核心子命令 | 解决的问题 |
|--------|-----------|-----------|
| `rosnode` | `list` / `info` / `kill` / `cleanup` | 有哪些节点，谁连着谁 |
| `rostopic` | `list` / `info` / `echo` / `hz` / `pub` / `type` | 话题是否存在，数据是否在流 |
| `rosmsg` | `list` / `show` / `package` | 消息类型的字段结构 |
| `rosservice` | `list` / `info` / `type` / `args` / `call` | 服务是否可用，如何手动调用 |
| `rossrv` | `show` / `list` / `package` | 服务请求与响应的完整结构 |
| `rosparam` | `get` / `set` / `list` / `delete` / `dump` / `load` | 参数的读写与持久化 |

三条最值得记住的组合：

```bash
timeout 3 rostopic echo -n 1 /话题    # 有没有数据
rostopic pub -1 /话题 类型 "字段: 值"  # 不写代码测试订阅者
rosparam dump params.yaml             # 保存调参结果
```

📖 需要按功能分类速查全部命令参数，见 [ROS 常用命令速查手册](../../ros_cmd.md)。

---

**下一节**：[2.5 通信机制实操](10-communication-practice.md) — 把三种通信机制放进同一个系统里跑起来。
