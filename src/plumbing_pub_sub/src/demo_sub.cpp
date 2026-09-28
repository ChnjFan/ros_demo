#include "ros/ros.h"
#include "std_msgs/String.h"
#include <sstream>

// void do_msg_handle(const std_msgs::String::ConstPtr &msg)
// {
//     ROS_INFO("Received message: %s", msgç->data.c_str());
// }

int main(int argc, char * argv[])
{
    ros::init(argc, argv, "listener");
    ros::NodeHandle nh;
    // ros::Subscriber sub = nh.subscribe("/chatter", 10, do_msg_handle);
    ros::Subscriber sub = nh.subscribe<std_msgs::String>(
        "chatter", 10,
        [&](const std_msgs::String::ConstPtr& msg)
        {
            ROS_INFO("msg: %s", msg->data.c_str());
        }
    );

    ros::spin(); // 进入消息循环阻塞当前线程
    return 0;
}

