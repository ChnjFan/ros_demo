#include <ros/ros.h>
#include "plumbing_pub_sub/Person.h"

int main(int argc, char **argv)
{
    ros::init(argc, argv, "demo_pub_person");
    ros::NodeHandle nh;

    ros::Publisher pub = nh.advertise<plumbing_pub_sub::Person>("person_info", 10);
    plumbing_pub_sub::Person person;
    person.name = "Alice";
    person.age = 30;
    person.height = 1.65;

    while (ros::ok())
    {
        pub.publish(person);
        ROS_INFO("Published person info: name=%s, age=%d, height=%.2f",
             person.name.c_str(), person.age, person.height);
        ros::Duration(1.0).sleep(); // Publish at 1 Hz
    }

    return 0;
}
