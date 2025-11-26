#include "rclcpp/rclcpp.hpp"
#include <chrono>
#include <thread>

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<rclcpp::Node>("hello_world_test_node");

    while (rclcpp::ok())
    {
        RCLCPP_INFO(node->get_logger(), "Hello, world!");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    rclcpp::shutdown();
    return 0;
}
