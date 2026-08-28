#include "rclcpp/rclcpp.hpp"
#include "node_helloworld_interface/srv/answer.hpp"

class NodeHelloworldClient : public rclcpp::Node
{
public:
    NodeHelloworldClient(const std::string &name) : Node(name)
    {
        // 创建服务客户端
        client_ = this->create_client<node_helloworld_interface::srv::Answer>(
            "helloworld_cpp_by_helloworld_interface");

        // 等待服务端启动（可选，建议加上）
        while (!client_->wait_for_service())
        {
            if (!rclcpp::ok())
            {
                RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for service.");
                return;
            }
            RCLCPP_WARN(this->get_logger(), "Waiting for service 'helloworld_cpp_by_helloworld_interface'...");
        }

        // 创建定时器，每秒发送一次请求
        timer_ = this->create_wall_timer(
            std::chrono::seconds(1),
            std::bind(&NodeHelloworldClient::timer_callback, this));
    }

private:
    void timer_callback()
    {
        // 构造请求
        auto request = std::make_shared<node_helloworld_interface::srv::Answer::Request>();
        request->name = "client_cpp";

        // 异步发送请求，并绑定回调处理响应
        auto future = client_->async_send_request(
            request,
            std::bind(&NodeHelloworldClient::response_callback, this,
                      std::placeholders::_1));
    }

    void response_callback(
        rclcpp::Client<node_helloworld_interface::srv::Answer>::SharedFuture future)
    {
        try
        {
            auto response = future.get();
            RCLCPP_INFO(this->get_logger(), "Server response: %s", response->message.c_str());
        }
        catch (const std::exception &e)
        {
            RCLCPP_ERROR(this->get_logger(), "Service call failed: %s", e.what());
        }
    }

private:
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Client<node_helloworld_interface::srv::Answer>::SharedPtr client_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<NodeHelloworldClient>(
        "node_client_helloworld_cpp_by_helloworld_interface");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
