#include "rclcpp/rclcpp.hpp"
#include "learn_interface/msg/topic_opencv.hpp"
#include "geometry_msgs/msg/point.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "cv_bridge/cv_bridge.hpp"

#include <opencv2/opencv.hpp>
#include <memory>

class SubscriberNode : public rclcpp::Node
{
public:
    SubscriberNode(const std::string & name)
    : Node(name)
    {
        subscription_ = this->create_subscription<learn_interface::msg::TopicOpencv>(
            "learn_topic_opencv_find_point",
            10,
            std::bind(&SubscriberNode::callback, this, std::placeholders::_1)
        );
    }

private:
    void callback(const learn_interface::msg::TopicOpencv::SharedPtr msg)
    {
        // 处理图像
        try
        {
            cv::Mat cv_image = cv_bridge::toCvCopy(msg->image, "bgr8")->image;
            cv::imshow("Received Image", cv_image);
            cv::waitKey(1);
        }
        catch (const cv_bridge::Exception &e)
        {
            RCLCPP_ERROR(this->get_logger(), "图像错误: %s", e.what());
        }

        // 处理点
        if (!msg->points.empty())
        {
            RCLCPP_INFO(this->get_logger(), "检测到 %zu 个点", msg->points.size());
            for (size_t i = 0; i < msg->points.size(); ++i)
            {
                const auto &point = msg->points[i];
                RCLCPP_INFO(this->get_logger(),
                            "点%zu: (%.1f, %.1f, %.1f)",
                            i + 1, point.x, point.y, point.z);
            }
        }
    }

    rclcpp::Subscription<learn_interface::msg::TopicOpencv>::SharedPtr subscription_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SubscriberNode>("opencv_find_point_subscriber_node");

    try
    {
        rclcpp::spin(node);
    }
    catch (const std::exception &e)
    {
        RCLCPP_ERROR(node->get_logger(), "异常: %s", e.what());
    }

    cv::destroyAllWindows();
    rclcpp::shutdown();
    return 0;
}
