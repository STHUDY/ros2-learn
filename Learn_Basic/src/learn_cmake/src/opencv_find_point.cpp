#include "rclcpp/rclcpp.hpp"
#include <opencv2/opencv.hpp>
#include <vector>
#include <chrono>
#include <thread>

std::pair<bool, std::vector<std::tuple<int, int, float>>> Find_Point(const cv::Mat &origin)
{
    cv::Mat hsv;
    cv::cvtColor(origin, hsv, cv::COLOR_BGR2HSV);

    // Define HSV range for red
    cv::Scalar hsv_lower(60, 43, 46);
    cv::Scalar hsv_upper(77, 255, 255);

    cv::Mat mask;
    cv::inRange(hsv, hsv_lower, hsv_upper, mask);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

    int radius_max = 0;
    std::vector<std::tuple<int, int, float>> point_result;

    for (const auto &cnt : contours)
    {
        double epsilon = 0.1 * cv::arcLength(cnt, true);
        std::vector<cv::Point> approx;
        cv::approxPolyDP(cnt, approx, epsilon, true);

        cv::Point2f center;
        float radius;
        cv::minEnclosingCircle(approx, center, radius);

        if (radius_max < radius)
        {
            radius_max = radius;
            point_result.emplace_back(
                static_cast<int>(center.x),
                static_cast<int>(center.y),
                radius);
        }
    }

    return {radius_max != 0, point_result};
}

void draw_point_circle(cv::Mat &image, const std::vector<std::tuple<int, int, float>> &posArray)
{
    for (const auto &pos : posArray)
    {
        cv::Point center(std::get<0>(pos), std::get<1>(pos));
        int radius = std::get<2>(pos);
        cv::circle(image, center, radius, cv::Scalar(255, 0, 0), 2);
    }
}

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);

    cv::VideoCapture cap(0);
    if (!cap.isOpened())
    {
        std::cerr << "Error opening video capture" << std::endl;
        return EXIT_FAILURE;
    }

    cv::Mat frame;
    auto node = std::make_shared<rclcpp::Node>("hello_world_test_node");

    while (rclcpp::ok())
    {
        cap >> frame;
        if (frame.empty())
            break;

        try
        {
            auto [isFind, points] = Find_Point(frame);

            if (isFind)
            {
                std::string outputInfo;

                for (const auto &point : points)
                {
                    outputInfo += "Found " + std::to_string(points.size()) + " points : ";
                    outputInfo += "(" + std::to_string(std::get<0>(point)) + ", " + std::to_string(std::get<1>(point)) + ", " + std::to_string(std::get<2>(point)) + ") ";
                }

                RCLCPP_INFO(node->get_logger(), "%s", outputInfo.c_str());
            }

            if (isFind)
            {
                draw_point_circle(frame, points);
            }

            cv::imshow("photo", frame);
        }
        catch (const cv::Exception &e)
        {
            std::cerr << "OpenCV exception: " << e.what() << std::endl;
            continue;
        }

        if (cv::waitKey(1) == 'q')
        {
            break;
        }
    }
    cap.release();
    cv::destroyAllWindows();

    rclcpp::shutdown();
    return 0;
}
