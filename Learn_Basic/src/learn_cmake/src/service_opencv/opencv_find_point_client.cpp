#include "rclcpp/rclcpp.hpp"
#include "learn_interface/srv/topic_opencv.hpp"
#include "geometry_msgs/msg/point.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "cv_bridge/cv_bridge.hpp"

#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>

class SubscriberNode : public rclcpp::Node
{
public:
    SubscriberNode(const std::string &name)
        : Node(name)
    {
        client_ = this->create_client<learn_interface::srv::TopicOpencv>(
            "learn_service_opencv_find_point");
    }

    bool wait_for_server(int timeout_sec = 60)
    {
        RCLCPP_INFO(this->get_logger(), "等待服务可用...");
        if (client_->wait_for_service(std::chrono::seconds(timeout_sec)))
        {
            RCLCPP_INFO(this->get_logger(), "服务连接成功");
            return true;
        }
        else
        {
            RCLCPP_ERROR(this->get_logger(), "服务在 %d 秒内不可用", timeout_sec);
            return false;
        }
    }

    rclcpp::Client<learn_interface::srv::TopicOpencv>::SharedFuture send_request(bool scan_once = true)
    {
        if (!client_->service_is_ready())
        {
            RCLCPP_ERROR(this->get_logger(), "服务未就绪");
            return rclcpp::Client<learn_interface::srv::TopicOpencv>::SharedFuture();
        }

        auto request = std::make_shared<learn_interface::srv::TopicOpencv::Request>();
        request->scan_once = scan_once;

        auto future_and_id = client_->async_send_request(request);
        return future_and_id.future.share();
    }

    void handle_msg(const std::shared_ptr<learn_interface::srv::TopicOpencv::Response> response)
    {
        if (!response)
        {
            RCLCPP_ERROR(this->get_logger(), "无响应数据");
            return;
        }

        // 处理图像
        try
        {
            cv::Mat cv_image = cv_bridge::toCvCopy(response->image, "bgr8")->image;
            cv::imshow("Received Image", cv_image);
            cv::waitKey(1);
        }
        catch (const cv_bridge::Exception &e)
        {
            RCLCPP_ERROR(this->get_logger(), "图像错误: %s", e.what());
        }

        // 处理点
        if (!response->points.empty())
        {
            RCLCPP_INFO(this->get_logger(), "检测到 %zu 个点", response->points.size());
            for (size_t i = 0; i < response->points.size(); ++i)
            {
                const auto &point = response->points[i];
                RCLCPP_INFO(this->get_logger(),
                            "点%zu: (%.1f, %.1f, %.1f)",
                            i + 1, point.x, point.y, point.z);
            }
        }
    }

private:
    rclcpp::Client<learn_interface::srv::TopicOpencv>::SharedPtr client_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    auto client = std::make_shared<SubscriberNode>("opencv_find_point_service_node");

    if (!client->wait_for_server())
    {
        return 0;
    }

    int fps = 24;

    while (rclcpp::ok())
    {
        auto future = client->send_request(true);
        if (future.valid())
        {
            if (rclcpp::spin_until_future_complete(client, future) ==
                rclcpp::FutureReturnCode::SUCCESS)
            {
                auto response = future.get();
                client->handle_msg(response);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps));
    }

    cv::destroyAllWindows();
    rclcpp::shutdown();
    return 0;
}
