# 使用 Docker Compose 开发 ROS 1

## Docker Compose 是什么

Dockerfile 描述“ROS 开发镜像中安装什么”，Docker Compose 描述“项目需要启动哪些容器，以及这些容器如何联网、挂载目录和协同工作”。

本项目包含两个服务：

| 服务 | 作用 |
|------|------|
| `ros-master` | 运行 `roscore`，提供 ROS Master、参数服务器和日志服务 |
| `ros-dev` | 挂载项目源码，用于执行 `catkin_make`、`rosrun` 和 `roslaunch` |

两个服务位于 Compose 自动创建的同一个网络中。`ros-dev` 通过 `http://ros-master:11311` 访问 ROS Master。

## 首次启动

确保 Docker Desktop 已经启动，然后在项目根目录执行：

```bash
docker compose up -d --build
docker compose ps
```

第一次构建需要下载 Ubuntu 20.04/ROS Noetic 镜像并安装开发工具，因此耗时会稍长。

## 安装功能包依赖

每当 `package.xml` 中的依赖发生变化，可以执行：

```bash
docker compose exec ros-dev \
  rosdep install --from-paths src --ignore-src -r -y
```

## 编译项目

```bash
docker compose exec ros-dev catkin_make
```

`build` 和 `devel` 被保存在 Docker named volume 中，不会把 Linux 编译产物写入 macOS 项目目录。源码则通过 bind mount 实时同步，Mac 上修改代码后不需要重新构建 Docker 镜像，只需要重新执行 `catkin_make`。

## 运行示例

启动 hello_world launch 文件：

```bash
docker compose exec ros-dev \
  roslaunch hello_world start_turtle.launch
```

运行 Topic 发布节点：

```bash
docker compose exec ros-dev \
  rosrun plumbing_pub_sub pub_sub_node
```

在另一个终端中查看 Topic：

```bash
docker compose exec ros-dev rostopic echo /chatter
```

进入交互式 ROS 开发终端：

```bash
docker compose exec ros-dev bash
```

镜像已经自动配置 `/opt/ros/noetic/setup.bash` 和工作空间的 `devel/setup.bash`。

## 日常管理命令

```bash
# 查看服务状态
docker compose ps

# 查看 roscore 日志
docker compose logs -f ros-master

# 停止并移除容器、保留编译卷
docker compose down

# 重新启动
docker compose up -d

# Dockerfile 改动后重新构建
docker compose up -d --build
```

如需清空 Linux 编译产物并完全重新编译：

```bash
docker compose down -v
docker compose up -d --build
docker compose exec ros-dev catkin_make
```

注意：`docker compose down -v` 会删除本项目的 Compose named volumes，包括容器内的 `build`、`devel` 和 ROS 日志，但不会删除挂载的项目源码。

## Apple Silicon 说明

基础镜像 `ros:noetic-ros-base-focal` 同时提供 ARM64 和 AMD64 版本。M1/M2/M3/M4 Mac 会自动使用 ARM64 镜像，无需添加 `platform`。只有依赖仅支持 x86 的旧软件时，才考虑在 Compose 服务中加入：

```yaml
platform: linux/amd64
```

模拟运行 AMD64 镜像通常比原生 ARM64 慢。

## GUI 工具

当前配置面向命令行编译和节点开发。RViz、rqt 和 Gazebo 在 macOS Docker Desktop 上需要额外的 XQuartz、OpenGL 或远程桌面配置，不建议直接塞进基础开发环境；需要时可以再增加独立的 GUI 服务。
