# 1.2 ROS 安装

> 本节学习目标：完成 ROS Noetic 开发环境的搭建，并成功运行 `roscore` 验证安装。

---

## 1.2.1 安装方案概览

根据你的宿主机不同，有三种安装方案：

| 方案 | 适用场景 | 难度 | 推荐度 |
|------|---------|:----:|:------:|
| **方案 A：物理机安装** | 有一台专用 Ubuntu 电脑 | ⭐ | ⭐⭐⭐ |
| **方案 B：虚拟机安装** | Windows 用户使用 VirtualBox / VMware | ⭐⭐ | ⭐⭐⭐ |
| **方案 C：Docker 安装** | 不想装 Ubuntu，想快速体验 | ⭐ | ⭐⭐ |

> 💡 **本教程推荐方案 B（虚拟机）**，因为大多数 Windows 用户不需要双系统，且虚拟机可以随时快照回滚，不怕搞坏环境。

---

## 1.2.2 方案 B：虚拟机安装（推荐）

### 步骤 1：安装虚拟机软件

下载并安装 **VirtualBox**（免费）或 **VMware Workstation**（付费）：

| 软件 | 下载链接 | 费用 |
|------|---------|------|
| VirtualBox | <https://www.virtualbox.org/wiki/Downloads> | 免费 |
| VMware Workstation Pro | <https://www.vmware.com/products/workstation-pro.html> | 免费（个人使用） |

安装完成后，新建一台虚拟机，推荐配置：

| 资源 | 最低配置 | 推荐配置 |
|------|---------|---------|
| CPU | 2 核 | 4 核 |
| 内存 | 4 GB | 8 GB |
| 硬盘 | 40 GB | 80 GB |
| 网络 | NAT | 桥接（多机通信时需要） |

### 步骤 2：安装 Ubuntu 20.04

1. 下载 Ubuntu 20.04 LTS 镜像：<https://releases.ubuntu.com/20.04/>
2. 在虚拟机中挂载 ISO 镜像，启动安装
3. 安装过程中勾选 **"安装 OpenSSH 服务器"**（方便后续远程开发）
4. 安装完成后重启，执行系统更新：

```bash
sudo apt update && sudo apt upgrade -y
```

### 步骤 3：安装增强工具

安装 VirtualBox 增强功能或 VMware Tools，实现：
- 共享剪贴板（Windows ↔ Ubuntu 复制粘贴）
- 共享文件夹（Windows 文件直接在 Ubuntu 中访问）
- 自适应分辨率

```bash
# VirtualBox 增强功能安装
sudo apt install -y virtualbox-guest-utils virtualbox-guest-x11
```

---

## 1.2.3 安装 ROS Noetic

### 配置 Ubuntu 软件源

确保 `restricted`、`universe`、`multiverse` 源已启用：

```bash
sudo apt install -y software-properties-common
sudo add-apt-repository universe
sudo add-apt-repository multiverse
sudo apt update
```

### 添加 ROS 软件源

```bash
# 添加 ROS 官方源
sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'

# 添加密钥
sudo apt install -y curl
curl -s https://raw.githubusercontent.com/ros/rosdistro/master/ros.asc | sudo apt-key add -

# 更新软件包索引
sudo apt update
```

### 安装 ROS Noetic 桌面完整版

```bash
sudo apt install -y ros-noetic-desktop-full
```

> ⏱️ 安装大约需要 15-30 分钟，取决于网络速度。

### 安装依赖工具

```bash
# 安装 rosdep（依赖管理工具）
sudo apt install -y python3-rosdep python3-rosinstall python3-rosinstall-generator python3-wstool build-essential

# 初始化 rosdep
sudo rosdep init
rosdep update
```

> ⚠️ 如果 `rosdep update` 失败（网络问题），可以使用国内镜像：
> ```bash
> # 使用 tuna 镜像
> sudo pip3 install -i https://pypi.tuna.tsinghua.edu.cn/simple rosdepc
> sudo rosdepc init
> rosdepc update
> ```

### 配置环境变量

```bash
# 将 ROS 环境变量加入 bashrc（每次打开终端自动加载）
echo "source /opt/ros/noetic/setup.bash" >> ~/.bashrc
source ~/.bashrc
```

---

## 1.2.4 方案 C：Docker 快速安装（免 Ubuntu）

如果你不想装虚拟机，可以用 Docker 快速体验 ROS：

```bash
# 安装 Docker（如果还没有）
curl -fsSL https://get.docker.com | sh
sudo usermod -aG docker $USER
# 重新登录使组权限生效

# 拉取 ROS Noetic 镜像
docker pull osrf/ros:noetic-desktop-full

# 启动容器（带图形界面支持）
docker run -it \
    --env="DISPLAY" \
    --env="QT_X11_NO_MITSHM=1" \
    --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
    osrf/ros:noetic-desktop-full \
    bash

# 在容器内 source ROS 环境
source /opt/ros/noetic/setup.bash
```

> ⚠️ Docker 方案适合快速体验，但长期使用推荐虚拟机或物理机，因为：
> - 共享文件夹性能较差
> - 网络配置复杂
> - 无法直接访问 USB 设备（如机器人）

---

## 1.2.5 测试 ROS 安装

### 检查环境变量

```bash
# 检查 ROS 环境变量是否正确
echo $ROS_DISTRO
# 应输出：noetic

echo $ROS_PACKAGE_PATH
# 应输出：/opt/ros/noetic/share
```

### 运行 roscore

打开终端，执行：

```bash
roscore
```

如果看到类似以下输出，说明安装成功：

```
... logging to /home/username/.ros/log/xxx.log
Checking log disk usage for logging purposes. This may take a while.
Press Ctrl-C to interrupt
Done checking log disk usage. Usage is <1GB.

started roslaunch server http://localhost:53973/
ros_comm version 1.15.14


SUMMARY
========

PARAMETERS
 * /rosdistro: noetic
 * /rosversion: 1.15.14

NODES

auto-starting new master
process[master-1]: started with PID [xxxxx]
started core service [/rosout]
```

> 💡 **roscore 做了什么？**
> - 启动了 **ROS Master**（节点注册与发现中心）
> - 启动了 **Parameter Server**（参数服务器）
> - 启动了 **rosout** 日志节点

### 按 Ctrl+C 停止 roscore

---

## 1.2.6 常见问题排查

| 问题 | 原因 | 解决方案 |
|------|------|---------|
| `roscore: command not found` | 环境变量未配置 | 执行 `source /opt/ros/noetic/setup.bash` 并检查 `.bashrc` |
| `rosdep update` 超时 | 网络问题 | 使用 `rosdepc` 国内镜像 |
| 虚拟机无法联网 | 网络配置错误 | 检查虚拟机网络模式（NAT 或桥接） |
| 安装过程报依赖错误 | 源未更新 | 执行 `sudo apt update` 后重试 |
| Docker 图形界面无法显示 | X11 未配置 | 先执行 `xhost +local:docker` |

---

## 本节小结

| 步骤 | 命令 | 作用 |
|------|------|------|
| 添加 ROS 源 | `sudo sh -c 'echo "deb ..." > /etc/apt/sources.list.d/ros-latest.list'` | 让 apt 能找到 ROS 包 |
| 安装 ROS | `sudo apt install -y ros-noetic-desktop-full` | 安装完整 ROS |
| 初始化 rosdep | `sudo rosdep init && rosdep update` | 依赖管理工具 |
| 配置环境 | `echo "source /opt/ros/noetic/setup.bash" >> ~/.bashrc` | 自动加载 ROS 环境 |
| 验证安装 | `roscore` | 启动 ROS Master |

---

## 理解检查

学完本节后，你应该能够：

- ✅ 在一台虚拟机中安装 Ubuntu 20.04 和 ROS Noetic
- ✅ 成功运行 `roscore` 并理解它的作用
- ✅ 使用 `echo $ROS_DISTRO` 验证环境变量
- ✅ 遇到 `command not found` 时知道如何排查

---

## 下一节

环境已就绪，接下来我们写第一个 ROS 程序——HelloWorld：

→ **[1.3 ROS 快速体验](03-helloworld.md)**
