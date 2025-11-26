#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class PublisherNode : public rclcpp::Node
{
public:
    PublisherNode(const std::string & name)
    : Node(name), count_(0)
    {
        // 创建 Publisher
        publisher_ = this->create_publisher<std_msgs::msg::String>("learn_helloworld_topic", 10);

        // 创建定时器，每 0.5 秒调用一次回调
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(500),
            std::bind(&PublisherNode::timer_callback, this));
    }

private:
    void timer_callback()
    {
        auto msg = std_msgs::msg::String();
        msg.data = "hello world " + std::to_string(count_);
        publisher_->publish(msg);
        RCLCPP_INFO(this->get_logger(), "publishing: '%s'", msg.data.c_str());
        count_++;
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    int count_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<PublisherNode>("helloworld_topic_publisher_node");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
