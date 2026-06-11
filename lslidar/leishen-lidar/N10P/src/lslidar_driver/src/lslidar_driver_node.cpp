#include "serial.hpp"
#include <rclcpp/rclcpp.hpp>

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Driver::Serial>("lslidar_driver_node");
    node->threadStart(); // 初始化串口和缓冲区
    rclcpp::spin(node);  // 保持节点运行
    rclcpp::shutdown();
    return 0;
}