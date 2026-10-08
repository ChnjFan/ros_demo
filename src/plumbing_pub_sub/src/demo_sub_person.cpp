#include <ros/ros.h>
#include "plumbing_pub_sub/Person.h"

void personCallback(const plumbing_pub_sub::Person::ConstPtr& msg)
{
    ROS_INFO("Received person info: name=%s, age=%d, height=%.2f",
             msg->name.c_str(), msg->age, msg->height);
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "demo_sub_person");
    ros::NodeHandle nh;

    ros::Subscriber sub = nh.subscribe("person_info", 10, personCallback);

    ros::spin();

    return 0;
}
