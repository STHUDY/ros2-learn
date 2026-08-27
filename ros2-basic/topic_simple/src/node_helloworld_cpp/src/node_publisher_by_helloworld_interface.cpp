#include "rclcpp/rclcpp.hpp"
#include "node_helloworld_interface/msg/message.hpp"

class NodeHelloworld : public rclcpp::Node
{
public:
    NodeHelloworld(std::string name) : Node(name)
    {
        timer_ = this->create_wall_timer(
            std::chrono::seconds(1),
            std::bind(&NodeHelloworld::timer_callback, this));

        publisher_ = this->create_publisher<node_helloworld_interface::msg::Message>("helloworld_cpp_by_helloworld_interface", 10);
    }

private:
    void timer_callback()
    {
        node_helloworld_interface::msg::Message msg;
        msg.name = "publisher";
        msg.message = "Hello World C++(node_helloworld_interface) Publisher: " + std::to_string(count_);
        publisher_->publish(msg);
        RCLCPP_INFO(this->get_logger(), "Hello World Topic C++(node_helloworld_interface) Publisher: %d", ++count_);
    }

private:
    int count_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<node_helloworld_interface::msg::Message>::SharedPtr publisher_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<NodeHelloworld>("node_topic_publisher_helloworld_cpp_by_helloworld_interface");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}