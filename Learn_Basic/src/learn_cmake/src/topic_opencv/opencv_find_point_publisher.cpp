#include "rclcpp/rclcpp.hpp"
#include "learn_interface/msg/topic_opencv.hpp"
#include "geometry_msgs/msg/point.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "cv_bridge/cv_bridge.hpp"

#include <opencv2/opencv.hpp>
#include <vector>
#include <tuple>
#include <memory>

using namespace std::chrono_literals;

std::pair<bool, std::vector<std::tuple<int, int, int>>> Find_Point(const cv::Mat &origin)
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
    std::vector<std::tuple<int, int, int>> point_result;

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

void draw_point_circle(cv::Mat &image, const std::vector<std::tuple<int, int, int>> &posArray)
{
    for (auto &pos : posArray)
    {
        cv::Point center(std::get<0>(pos), std::get<1>(pos));
        int radius = std::get<2>(pos);
        cv::circle(image, center, radius, cv::Scalar(255, 0, 0), 2);
    }
}

class PublisherNode : public rclcpp::Node
{
public:
    PublisherNode(const std::string &name)
        : Node(name)
    {
        publisher_ = this->create_publisher<learn_interface::msg::TopicOpencv>(
            "learn_topic_opencv_find_point", 10);

        cap_.open(0);
        if (!cap_.isOpened())
        {
            RCLCPP_ERROR(this->get_logger(), "Error opening video capture");
        }

        timer_ = this->create_wall_timer(50ms, std::bind(&PublisherNode::timer_callback, this));
    }

private:
    void timer_callback()
    {
        cv::Mat frame;
        cap_ >> frame;
        if (frame.empty())
            return;

        try
        {
            auto [isFind, points] = Find_Point(frame);

            std::vector<std::tuple<int, int, int>> pub_points;
            for (auto &point : points)
            {
                if (std::get<2>(point) > 10)
                    pub_points.push_back(point);
            }

            if (isFind)
            {
                draw_point_circle(frame, pub_points);
            }

            // 构造消息
            learn_interface::msg::TopicOpencv msg;
            auto image_ptr = cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", frame).toImageMsg();
            msg.image = *image_ptr;

            std::vector<geometry_msgs::msg::Point> point_msgs;
            for (auto &point : pub_points)
            {
                geometry_msgs::msg::Point p;
                p.x = static_cast<double>(std::get<0>(point));
                p.y = static_cast<double>(std::get<1>(point));
                p.z = static_cast<double>(std::get<2>(point));
                point_msgs.push_back(p);
            }
            msg.points = point_msgs;

            publisher_->publish(msg);
        }
        catch (const cv::Exception &e)
        {
            RCLCPP_ERROR(this->get_logger(), "OpenCV exception: %s", e.what());
        }
    }

    rclcpp::Publisher<learn_interface::msg::TopicOpencv>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    cv::VideoCapture cap_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<PublisherNode>("opencv_find_point_publisher_node");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
