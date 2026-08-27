#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class NodeHelloworld : public rclcpp::Node
{
public:
    NodeHelloworld(std::string name) : Node(name)
    {
        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "helloworld_cpp_by_std_msgs",
            rclcpp::QoS(rclcpp::KeepLast(10)),
            std::bind(&NodeHelloworld::subscriber_callback, this, std::placeholders::_1));
    }

private:
    void subscriber_callback(const std_msgs::msg::String::SharedPtr msg)
    {
        RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg->data.c_str());
    }

private:
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<NodeHelloworld>("node_topic_subscriber_helloworld_cpp_by_std_msgs");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}