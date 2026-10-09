#include <ros/ros.h>
#include "plumbing_server_client/AddInt.h"

bool add(plumbing_server_client::AddInt::Request & req,
         plumbing_server_client::AddInt::Response & res)
{
    res.sum = req.num1 + req.num2;
    ROS_INFO("Received request: num1=%d, num2=%d, sum=%d", req.num1, req.num2, res.sum);
    return true;
}

int main(int argc, char * argv[])
{
    ros::init(argc, argv, "add_int_server");
    ros::NodeHandle nh;

    ros::ServiceServer service = nh.advertiseService("add_int", add);

    ros::spin();
    return 0;
}
