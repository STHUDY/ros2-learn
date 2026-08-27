#include "rclcpp/rclcpp.hpp"
#include "node_helloworld_interface/msg/message.hpp"

class NodeHelloworld : public rclcpp::Node
{
public:
    NodeHelloworld(std::string name) : Node(name)
    {
        subscription_ = this->create_subscription<node_helloworld_interface::msg::Message>(
            "helloworld_cpp_by_helloworld_interface",
            rclcpp::QoS(rclcpp::KeepLast(10)),
            std::bind(&NodeHelloworld::subscriber_callback, this, std::placeholders::_1));
    }

private:
    void subscriber_callback(const node_helloworld_interface::msg::Message::SharedPtr msg)
    {
        RCLCPP_INFO(this->get_logger(), "[%s] I heard: '%s'", msg->name.c_str(), msg->message.c_str());
    }

private:
    rclcpp::Subscription<node_helloworld_interface::msg::Message>::SharedPtr subscription_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<NodeHelloworld>("node_topic_subscriber_helloworld_cpp_by_helloworld_interface");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}