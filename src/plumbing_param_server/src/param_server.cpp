#include <ros/ros.h>


int main(int argc, char * argv[])
{
    ros::init(argc, argv, "param_server");

    ros::NodeHandle nh;

    nh.setParam("type", "turtlesim");       // string
    nh.setParam("radius", 0.5);             // double
    nh.setParam("wheels", 4);               // int
    nh.setParam("use_sim", true);           // bool

    ROS_INFO("Param is set");

    std::string type;
    double radius;
    int wheels;
    bool use_sim;

    nh.getParam("type", type);
    nh.getParam("radius", radius);
    nh.getParam("wheels", wheels);
    nh.getParam("use_sim", use_sim);

    ROS_INFO("type: %s", type.c_str());
    ROS_INFO("radius: %f", radius);
    ROS_INFO("wheels: %d", wheels);
    ROS_INFO("use_sim: %s", use_sim ? "true" : "false");

    // param() 模板函数：如果参数存在则读取到变量，不存在则使用默认值
    nh.param<std::string>("type_not_exist", type, "default_type");
    ROS_INFO("type_not_exist: %s", type.c_str());

    if (!nh.hasParam("type_not_exist"))
    {
        ROS_WARN("Param 'type_not_exist' does not exist");
    }

    // getParamCached()：从缓存中获取参数，避免频繁访问参数服务器
    nh.getParamCached("type", type);
    ROS_INFO("type (cached): %s", type.c_str());

    // getParamNames()：获取所有参数的名称
    std::vector<std::string> param_names;
    nh.getParamNames(param_names);
    ROS_INFO("All param names:");
    for (const auto & name : param_names) {
        ROS_INFO("  %s", name.c_str());
    }

    nh.deleteParam("type");
    ROS_INFO("Param 'type' deleted");
    if (!nh.hasParam("type"))
    {
        ROS_INFO("Param 'type' is deleted successfully");
    }

    return 0;
}
