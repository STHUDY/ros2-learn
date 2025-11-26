#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class SubscriberNode : public rclcpp::Node
{
public:
    SubscriberNode(const std::string &name)
        : Node(name)
    {
        // 创建订阅者
        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "learn_helloworld_topic", // 话题名（注意拼写和 Publisher 保持一致）
            10,                       // QoS 队列长度
            std::bind(&SubscriberNode::callback, this, std::placeholders::_1));
    }

private:
    void callback(const std_msgs::msg::String::SharedPtr msg)
    {
        RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg->data.c_str());
    }

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SubscriberNode>("helloworld_topic_subscriber_node");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
