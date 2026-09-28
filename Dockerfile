FROM ros:noetic-ros-base-focal

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        git \
        iputils-ping \
        nano \
        python3-catkin-tools \
        python3-rosdep \
        ros-noetic-turtlesim \
        vim \
    && rm -rf /var/lib/apt/lists/*

ENV LANG=C.UTF-8 \
    LC_ALL=C.UTF-8

WORKDIR /workspace

COPY docker/ros-entrypoint.sh /usr/local/bin/ros-entrypoint

RUN chmod +x /usr/local/bin/ros-entrypoint \
    && printf '%s\n' \
        'source /opt/ros/noetic/setup.bash' \
        '[ ! -f /workspace/devel/setup.bash ] || source /workspace/devel/setup.bash' \
        >> /root/.bashrc

ENTRYPOINT ["/usr/local/bin/ros-entrypoint"]
CMD ["bash"]
