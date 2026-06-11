#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <memory>
#include <cstdint>
#include <limits>
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"
#include "sensor_msgs/msg/point_field.hpp"
#include "Uart/Uart.hpp"

namespace Driver
{
    class Serial : public rclcpp::Node
    {
    public:
        explicit Serial(std::string nodeName);
        ~Serial();
        void threadStart();

    private:
        // ---------- 参数 ----------
        std::string lidar_name_;
        std::string serial_port_;
        std::string frame_id_;
        bool topic_open_scan_;
        std::string topic_scan_name_;
        bool topic_open_point_cloud2_;
        std::string topic_point_cloud2_name_;
        std::vector<std::pair<double, double>> angle_range_;    // 角度区间（度）
        std::vector<std::pair<double, double>> distance_range_; // 距离区间（厘米）
        double range_cm_min_, range_cm_max_;
        static constexpr int BAUD_RATE_ = 460800;
        static constexpr size_t PACKET_SIZE_ = 108;
        static constexpr uint16_t ANGLE_MAX_RAW_ = 36000; // 0.01° 单位
        static constexpr uint16_t PACKET_FPS_ONCE_ = 64;  // 根据协议修改为64个点（原代码为32，此处修正）

        // ---------- UART & 缓冲区 ----------
        Uart::Uart *uart_device_ = nullptr;

        // ---------- 线程控制 ----------
        volatile bool running_ = false;
        std::thread read_thread_;
        std::thread publish_thread_;
        std::mutex publish_mtx_;
        std::condition_variable publish_cv_;

        // ---------- 帧累积 ----------
        std::vector<double> degree_points_;
        std::vector<double> distance_points_;
        std::vector<double> intensity_points_;
        int points_count_ = 0;
        uint16_t frame_start_angle_raw_ = 0xFFFF; // 无效值
        uint16_t frame_end_angle_raw_ = 0;
        uint16_t last_angle_raw_ = 0;

        // ---------- 发布器 ----------
        rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr scan_pub_;
        rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr pointcloud2_pub_;

        // ---------- 私有函数 ----------
        std::vector<std::pair<double, double>> convertFlatToPairs(
            const std::vector<double> &flat, const std::string &param_name);
        bool isCrossStart(uint16_t lastAngle, uint16_t angle_raw, uint16_t startAngle, uint16_t maxAngle);
        bool isPointValid(uint16_t angle_raw, uint16_t distance_raw);

        // 线程函数
        void threadRead();
        void threadPublish();
        void publishFrame(); // 声明发布帧函数
    };
} // namespace Driver