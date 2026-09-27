#include "ros/ros.h"
#include "std_msgs/String.h"
#include <sstream>

int main(int argc, char *argv[])
{
    // 初始化 ROS 节点
    ros::init(argc, argv, "talker");
    // 创建节点句柄
    ros::NodeHandle nh;
    // 创建发布者对象
    ros::Publisher pub = nh.advertise<std_msgs::String>("/chatter", 10);

    // 发布逻辑发布数据
    int count = 0;
    while (ros::ok())
    {
        std_msgs::String msg;
        std::stringstream ss;
        ss << "Hello World [" << count << "]";
        msg.data = ss.str();

        pub.publish(msg);
        ROS_INFO("Publishing message %s", msg.data.c_str());

        count++;
    }
    

    return 0;
}

