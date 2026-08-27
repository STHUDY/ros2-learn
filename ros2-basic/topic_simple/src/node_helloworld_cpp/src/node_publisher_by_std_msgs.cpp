#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class NodeHelloworld : public rclcpp::Node
{
public:
    NodeHelloworld(std::string name) : Node(name)
    {
        timer_ = this->create_wall_timer(
            std::chrono::seconds(1),
            std::bind(&NodeHelloworld::timer_callback, this));

        publisher_ = this->create_publisher<std_msgs::msg::String>("helloworld_cpp_by_std_msgs", 10);
    }

private:
    void timer_callback()
    {
        std_msgs::msg::String msg;
        msg.data = "Hello World C++(std_msgs) Publisher: " + std::to_string(count_);
        publisher_->publish(msg);
        RCLCPP_INFO(this->get_logger(), "Hello World Topic C++(std_msgs) Publisher: %d", ++count_);
    }

private:
    int count_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<NodeHelloworld>("node_topic_publisher_helloworld_cpp_by_std_msgs");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}