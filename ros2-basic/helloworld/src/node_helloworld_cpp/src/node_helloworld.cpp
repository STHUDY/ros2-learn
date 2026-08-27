#include "rclcpp/rclcpp.hpp"

class NodeHelloworld : public rclcpp::Node
{
public:
    NodeHelloworld(std::string name) : Node(name)
    {
        timer_ = this->create_wall_timer(
            std::chrono::seconds(1),
            std::bind(&NodeHelloworld::timer_callback, this));
    }

private:
    void timer_callback()
    {
        RCLCPP_INFO(this->get_logger(), "Hello World C++: %d", ++count_);
    }

private:
    int count_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<NodeHelloworld>("node_helloworld_cpp");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}