#include <ros/ros.h>
#include "plumbing_server_client/AddInt.h"

int main(int argc, char * argv[])
{
    ros::init(argc, argv, "add_int_client");
    ros::NodeHandle nh;

    ros::ServiceClient client = nh.serviceClient<plumbing_server_client::AddInt>("add_int");

    // 可以指定等待服务的时间和等待服务，默认是无限等待
    ros::service::waitForService("add_int_not_exist", ros::Duration(5.0));

    while (!client.waitForExistence(ros::Duration(1.0)))
    {
        if (!ros::ok())
        {
            ROS_ERROR("Interruped while waiting for the server.");
            return 1;
        }
        ROS_INFO("Server not available, waiting again...");
    }

    plumbing_server_client::AddInt srv;
    srv.request.num1 = 100;
    srv.request.num2 = 200;

    ROS_INFO("Sending request");
    if (client.call(srv))
    {
        int response = srv.response.sum;
        ROS_INFO("Received response %d", response);
    }
    else
    {
        ROS_ERROR("Failed to call service");
    }

    ros::spin();
    ros::shutdown();
    return 0;
}
