#include "rclcpp/rclcpp.hpp"
#include "learn_interface/srv/topic_opencv.hpp"
#include "geometry_msgs/msg/point.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "cv_bridge/cv_bridge.hpp"

#include <opencv2/opencv.hpp>
#include <vector>
#include <tuple>
#include <memory>

// 查找红色点
std::pair<bool, std::vector<std::tuple<int,int,int>>> Find_Point(const cv::Mat &origin)
{
    cv::Mat hsv;
    cv::cvtColor(origin, hsv, cv::COLOR_BGR2HSV);

    cv::Scalar hsv_lower(156, 43, 46);
    cv::Scalar hsv_upper(180, 255, 255);
    cv::Mat mask;
    cv::inRange(hsv, hsv_lower, hsv_upper, mask);

    cv::Mat kernel = cv::Mat::ones(2, 2, CV_8U);
    cv::Mat mask_opened;
    cv::morphologyEx(mask, mask_opened, cv::MORPH_OPEN, kernel);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask_opened, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

    int radius_max = 0;
    std::vector<std::tuple<int,int,int>> point_result;

    for (auto &cnt : contours)
    {
        double epsilon = 0.1 * cv::arcLength(cnt, true);
        std::vector<cv::Point> approx;
        cv::approxPolyDP(cnt, approx, epsilon, true);

        cv::Point2f center;
        float radius;
        cv::minEnclosingCircle(approx, center, radius);

        if (radius_max < radius)
        {
            radius_max = static_cast<int>(radius);
            point_result.push_back({static_cast<int>(center.x), static_cast<int>(center.y), static_cast<int>(radius)});
        }
    }

    return {radius_max != 0, point_result};
}

// 在图像上画圈
void draw_point_circle(cv::Mat &image, const std::vector<std::tuple<int,int,int>> &posArray)
{
    for (auto &pos : posArray)
    {
        cv::Point center(std::get<0>(pos), std::get<1>(pos));
        int radius = std::get<2>(pos);
        cv::circle(image, center, radius, cv::Scalar(255, 0, 0), 2);
    }
}

class ServiceNode : public rclcpp::Node
{
public:
    ServiceNode(const std::string & name)
    : Node(name)
    {
        server_ = this->create_service<learn_interface::srv::TopicOpencv>(
            "learn_service_opencv_find_point",
            std::bind(&ServiceNode::handle_service_msg, this,
                      std::placeholders::_1, std::placeholders::_2));
        cap_.open(0);
        if (!cap_.isOpened())
        {
            RCLCPP_ERROR(this->get_logger(), "Error opening video capture");
        }
    }

private:
    void handle_service_msg(
        const std::shared_ptr<learn_interface::srv::TopicOpencv::Request> request,
        std::shared_ptr<learn_interface::srv::TopicOpencv::Response> response)
    {
        RCLCPP_INFO(this->get_logger(), "收到请求: scan_once=%d", request->scan_once);

        cv::Mat frame;
        cap_ >> frame;
        if (frame.empty())
        {
            RCLCPP_ERROR(this->get_logger(), "摄像头读取失败");
            return;
        }

        try
        {
            auto [isFind, points] = Find_Point(frame);

            std::vector<std::tuple<int,int,int>> pub_points;
            for (auto &point : points)
            {
                if (std::get<2>(point) > 10)
                    pub_points.push_back(point);
            }

            if (isFind)
            {
                draw_point_circle(frame, pub_points);
            }

            // 转换图像
            response->image = *cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", frame).toImageMsg();

            // 转换点数据
            std::vector<geometry_msgs::msg::Point> point_msgs;
            for (auto &point : pub_points)
            {
                geometry_msgs::msg::Point p;
                p.x = static_cast<double>(std::get<0>(point));
                p.y = static_cast<double>(std::get<1>(point));
                p.z = static_cast<double>(std::get<2>(point));
                point_msgs.push_back(p);
            }
            response->points = point_msgs;
        }
        catch (const cv::Exception &e)
        {
            RCLCPP_ERROR(this->get_logger(), "OpenCV异常: %s", e.what());
        }
    }

    rclcpp::Service<learn_interface::srv::TopicOpencv>::SharedPtr server_;
    cv::VideoCapture cap_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ServiceNode>("opencv_find_point_service_node");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
