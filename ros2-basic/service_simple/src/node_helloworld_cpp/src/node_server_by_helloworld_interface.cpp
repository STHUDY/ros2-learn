#include "rclcpp/rclcpp.hpp"
#include "node_helloworld_interface/srv/answer.hpp"

class NodeHelloworld : public rclcpp::Node
{
public:
    explicit NodeHelloworld(const std::string &name) : Node(name), count_(0)
    {
        server_ = this->create_service<node_helloworld_interface::srv::Answer>(
            "helloworld_cpp_by_helloworld_interface",
            std::bind(&NodeHelloworld::answer_callback, this,
                      std::placeholders::_1, std::placeholders::_2));
    }

private:
    void answer_callback(
        const node_helloworld_interface::srv::Answer::Request::SharedPtr request,
        node_helloworld_interface::srv::Answer::Response::SharedPtr response)
    {
        auto name = request->name;
        response->message = "[" + name + "] "
                                         "Hello World C++(node_helloworld_interface) Server: " +
                            std::to_string(++count_);
        RCLCPP_INFO(this->get_logger(),
                    "Received request from [%s], response count: %d",
                    name.c_str(), count_);
    }

private:
    int count_;
    rclcpp::Service<node_helloworld_interface::srv::Answer>::SharedPtr server_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<NodeHelloworld>(
        "node_server_helloworld_cpp_by_helloworld_interface");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
