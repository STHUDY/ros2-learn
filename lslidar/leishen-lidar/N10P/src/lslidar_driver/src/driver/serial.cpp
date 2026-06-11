#include "serial.hpp"
#include <cmath>
#include <cstring>
#include <unistd.h>

namespace Driver
{

    // ========== 工具函数 ==========
    std::vector<std::pair<double, double>> Serial::convertFlatToPairs(
        const std::vector<double> &flat, const std::string &param_name)
    {
        std::vector<std::pair<double, double>> result;
        if (flat.size() % 2 != 0)
        {
            RCLCPP_WARN(this->get_logger(),
                        "Parameter '%s' has odd number of elements (%zu). Last element discarded.",
                        param_name.c_str(), flat.size());
        }
        for (size_t i = 0; i + 1 < flat.size(); i += 2)
        {
            result.emplace_back(flat[i], flat[i + 1]);
        }
        if (result.empty())
        {
            RCLCPP_WARN(this->get_logger(),
                        "Parameter '%s' produced no valid intervals. Using default [0,360].",
                        param_name.c_str());
            result.emplace_back(0.0, 360.0);
        }
        return result;
    }

    bool Serial::isCrossStart(uint16_t lastAngle, uint16_t angle_raw, uint16_t startAngle, uint16_t maxAngle)
    {
        lastAngle %= maxAngle;
        angle_raw %= maxAngle;
        startAngle %= maxAngle;

        if (lastAngle == angle_raw)
            return true;

        if (angle_raw > lastAngle)
        {
            return (startAngle > lastAngle && startAngle <= angle_raw);
        }
        else
        {
            return (startAngle > lastAngle || startAngle <= angle_raw);
        }
    }

    bool Serial::isPointValid(uint16_t angle_raw, uint16_t distance_raw)
    {
        // 角度检查（度）
        double deg = (static_cast<double>(angle_raw) / 100.0);
        bool angle_ok = false;
        for (const auto &interval : angle_range_)
        {
            if (deg >= interval.first && deg <= interval.second)
            {
                angle_ok = true;
                break;
            }
        }
        if (!angle_ok)
            return false;

        // 距离检查（厘米）
        double dist_cm = static_cast<double>(distance_raw) / 100.0;
        for (const auto &interval : distance_range_)
        {
            if (dist_cm >= interval.first && dist_cm <= interval.second)
            {
                return true;
            }
        }
        return false;
    }

    // ========== 读线程 ==========

    void Serial::threadRead()
    {
        // 根据文档，N10 Plus 协议固定长度为 108 字节
        char buffer[PACKET_SIZE_ * 10];
        uint16_t distancePoints[PACKET_FPS_ONCE_];
        uint8_t intensityPoints[PACKET_FPS_ONCE_];

        enum class State
        {
            HEADER1,       // 寻找 0xA5
            HEADER2,       // 寻找 0x5A
            LENGTH,        // 读取长度
            RPM_HIGH,      // 转速高位
            RPM_LOW,       // 转速低位
            ANGLE_START_H, // 起始角度高位
            ANGLE_START_L, // 起始角度低位
            POINT_DATA,    // 点云数据区 (循环解析)
            RESERVE_BIT,   // 预留位
            ANGLE_END_H,   // 结束角度高位 (预留位后)
            ANGLE_END_L,   // 结束角度低位
            CHECKSUM       // 校验和
        };

        State state = State::HEADER1;
        size_t pkg_index = 0;
        uint8_t calc_checksum = 0; // 用于计算校验和

        // 临时变量，用于存储解析中的数据
        uint16_t rpm_raw = 0;
        uint16_t angle_start_raw = 0;
        uint16_t angle_end_raw = 0;
        uint16_t dist_raw = 0;
        uint8_t intensity = 0;
        uint8_t recvPointIndex = 0;

        while (running_)
        {
            if (!uart_device_->checkUartOpen())
            {
                RCLCPP_WARN(this->get_logger(), "Uart port '%s' is not open.", serial_port_.c_str());
                usleep(1000 * 1000);
                uart_device_->openUartSerial(serial_port_);
                continue;
            }

            int buffer_len = uart_device_->readData(buffer, sizeof(buffer));
            if (buffer_len <= 0)
            {
                uart_device_->closeUart();
                RCLCPP_WARN(this->get_logger(), "Uart port '%s' read error.", serial_port_.c_str());
                continue;
            }

            for (int i = 0; i < buffer_len; ++i)
            {
                uint8_t byte = static_cast<uint8_t>(buffer[i]);

                // 在处理数据前累加校验和 (校验和是 Byte0 到 Byte106 的和)
                // 注意：当到达 CHECKSUM 状态时，我们正在处理 Byte107，此时不应再累加
                if (state != State::HEADER1 && state != State::CHECKSUM)
                {
                    calc_checksum += byte;
                }

                switch (state)
                {
                case State::HEADER1:
                    if (byte == 0xA5)
                    {
                        // 重置所有状态和累加器
                        calc_checksum = 0;
                        pkg_index = 0;
                        state = State::HEADER2;
                    }
                    break;

                case State::HEADER2:
                    if (byte == 0x5A)
                    {
                        state = State::LENGTH;
                    }
                    else if (byte == 0xA5)
                    {
                        // 丢弃当前字节，保持在 HEADER2 状态等待 0x5A
                        // 防止连续的 A5 导致状态机重置
                    }
                    else
                    {
                        state = State::HEADER1;
                    }
                    break;

                case State::LENGTH:
                    if (byte == PACKET_SIZE_)
                    {
                        state = State::RPM_HIGH; // 长度正确，进入转速解析
                    }
                    else
                    {
                        state = State::HEADER1; // 长度错误，重新寻找帧头
                    }
                    break;

                case State::RPM_HIGH:
                    rpm_raw = byte << 8;
                    state = State::RPM_LOW;
                    break;

                case State::RPM_LOW:
                    rpm_raw |= byte;
                    // 转速 rpm_raw 单位为 us，如需转速 Hz:  freq = 1000.0 / (rpm_raw * 24);
                    state = State::ANGLE_START_H;
                    break;

                case State::ANGLE_START_H:
                    angle_start_raw = byte << 8;
                    state = State::ANGLE_START_L;
                    break;

                case State::ANGLE_START_L:
                    angle_start_raw |= byte;
                    // 实际角度 = angle_start_raw / 100.0 (单位: 度)
                    state = State::POINT_DATA;
                    // 注意：此时 pkg_index 应该指向点云数据区的起始位置 (Byte 7)
                    pkg_index = 7;
                    break;

                case State::POINT_DATA:
                {
                    // 点云数据区总长度：Byte 7 到 Byte 102 (共 96 字节)
                    // 每 3 个字节代表一个点：[Dist_H, Dist_L, Peak]
                    size_t offset_in_point = (pkg_index - 7) % 3;

                    if (offset_in_point == 0) // 距离高位
                    {
                        dist_raw = byte << 8;
                    }
                    else if (offset_in_point == 1) // 距离低位
                    {
                        dist_raw |= byte;
                    }
                    else if (offset_in_point == 2) // 强度
                    {
                        if (pkg_index >= 102)
                        {
                            state = State::RESERVE_BIT;
                        }
                        intensity = byte;

                        distancePoints[recvPointIndex] = dist_raw;
                        intensityPoints[recvPointIndex] = intensity;
                        recvPointIndex++;
                    }
                    pkg_index++;
                    break;
                }
                case State::RESERVE_BIT:
                {
                    // Byte 103 (实际索引 103)
                    if (pkg_index == 104)
                    {
                        state = State::ANGLE_END_H;
                    }
                    pkg_index++;
                    break;
                }
                case State::ANGLE_END_H:
                    // Byte 105 (实际索引 105)
                    if (pkg_index == 105)
                    {
                        angle_end_raw = byte << 8;
                        state = State::ANGLE_END_L;
                    }
                    pkg_index++;
                    break;

                case State::ANGLE_END_L:
                    // Byte 106 (实际索引 106)
                    if (pkg_index == 106)
                    {
                        angle_end_raw |= byte;
                        state = State::CHECKSUM;
                    }
                    pkg_index++;
                    break;

                case State::CHECKSUM:
                    // Byte 107 (实际索引 107)
                    if (pkg_index == 107)
                    {
                        uint8_t recv_checksum = byte;
                        // calc_checksum 现在包含了 Byte0 到 Byte106 的和
                        if (calc_checksum == recv_checksum)
                        {
                            bool isPublish = false;
                            for (int i = 0; i < recvPointIndex; i++)
                            {
                                int angleRangeRaw = angle_end_raw - angle_start_raw;
                                if (angleRangeRaw < 0)
                                {
                                    
                                }
                                uint16_t angleRangeDegreeRaw = angle_start_raw + (i * angleRangeRaw / recvPointIndex);
                                double degree = static_cast<double>(angleRangeDegreeRaw) / 100.0;
                                double dist_m;
                                double intensity_float;
                                if (isPointValid(angleRangeDegreeRaw, distancePoints[i]))
                                {
                                    dist_m = static_cast<double>(distancePoints[i]) / 1000.0;
                                    intensity_float = static_cast<double>(intensityPoints[i]);
                                }
                                else
                                {
                                    dist_m = std::numeric_limits<float>::infinity();
                                    intensity_float = 0;
                                }

                                if (points_count_ >= distance_points_.size())
                                {
                                    degree_points_.push_back(degree);
                                    distance_points_.push_back(dist_m);
                                    intensity_points_.push_back(intensity_float);
                                }
                                else
                                {
                                    degree_points_[points_count_] = degree;
                                    distance_points_[points_count_] = dist_m;
                                    intensity_points_[points_count_] = intensity_float;
                                }
                                points_count_++;
                            }
                            publish_cv_.notify_one();
                        }
                        else
                        {
                            RCLCPP_WARN(this->get_logger(),
                                        "Check sum error: calc=0x%02X, recv=0x%02X",
                                        calc_checksum, recv_checksum);
                        }
                        // 无论成功失败，重置状态机寻找下一包
                        state = State::HEADER1;
                        pkg_index = 0;
                        calc_checksum = 0;
                    }
                    break;
                }
            }
        }
    }

    // ========== 发布线程 ==========
    void Serial::threadPublish()
    {
        while (running_)
        {
            std::unique_lock<std::mutex> lock(publish_mtx_);

            // 关键：必须用 while 检查条件（防虚假唤醒）
            publish_cv_.wait(lock);
            // 解析包头
            uint16_t rpm_raw = (packet[3] << 8) | packet[4];           // 转速 (μs)
            uint16_t angle_start_raw = (packet[5] << 8) | packet[6];   // 起始角度 (0.01°)
            uint16_t angle_end_raw = (packet[105] << 8) | packet[106]; // 结束角度 (0.01°)
            (void)rpm_raw;                                             // 未使用转速，避免警告

            // 帧同步逻辑
            if (frame_start_angle_raw_ == 0xFFFF)
            {
                // 第一个包
                frame_start_angle_raw_ = angle_start_raw;
                frame_end_angle_raw_ = angle_end_raw;
                last_angle_raw_ = angle_end_raw;
            }
            else
            {
                // 检查是否跨过帧起始角
                if (isCrossStart(last_angle_raw_, angle_start_raw, frame_start_angle_raw_, ANGLE_MAX_RAW_))
                {
                    // 上一帧完整，发布
                    publishFrame();
                    // 重置为新帧
                    scan_points_.clear();
                    cloud_points_.clear();
                    frame_start_angle_raw_ = angle_start_raw;
                    frame_end_angle_raw_ = angle_end_raw;
                }
                else
                {
                    // 更新结束角
                    frame_end_angle_raw_ = angle_end_raw;
                }
                last_angle_raw_ = angle_end_raw;
            }

            // 解析点云数据段 (Byte 7 ~ 102)
            // 每组6字节：第一回波距离(2B)+强度(1B)，第二回波距离(2B)+强度(1B)
            constexpr int DATA_START = 7;
            constexpr int DATA_END = 102;
            constexpr int GROUP_SIZE = 6;
            constexpr int GROUP_COUNT = (DATA_END - DATA_START + 1) / GROUP_SIZE; // 16

            for (int k = 0; k < GROUP_COUNT; ++k)
            {
                int base = DATA_START + k * GROUP_SIZE;
                uint16_t dist1 = (packet[base] << 8) | packet[base + 1]; // mm
                uint8_t inten1 = packet[base + 2];
                uint16_t dist2 = (packet[base + 3] << 8) | packet[base + 4]; // mm
                uint8_t inten2 = packet[base + 5];

                // 插值该组的角度
                double start_deg = angle_start_raw;
                double end_deg = angle_end_raw;
                if (end_deg < start_deg)
                    end_deg += ANGLE_MAX_RAW; // 处理跨零
                double angle_raw_d = start_deg + (end_deg - start_deg) * k / (GROUP_COUNT - 1.0);
                if (angle_raw_d >= ANGLE_MAX_RAW)
                    angle_raw_d -= ANGLE_MAX_RAW; // 归一化

                double angle_rad = (angle_raw_d / 100.0) * (M_PI / 180.0);

                // 第一回波点
                PointData pt1;
                pt1.degree = angle_rad;
                pt1.distance = dist1 / 1000.0; // mm → m
                pt1.intensity = static_cast<double>(inten1);
                scan_points_.push_back(pt1);
                cloud_points_.push_back(pt1);

                // 第二回波点
                PointData pt2;
                pt2.degree = angle_rad;
                pt2.distance = dist2 / 1000.0;
                pt2.intensity = static_cast<double>(inten2);
                cloud_points_.push_back(pt2);
            }
        }
    }

    // 发布一帧 LaserScan 和 PointCloud2
    void Serial::publishFrame()
    {
        if (scan_points_.empty())
            return;

        // ---------- LaserScan ----------
        if (topic_open_scan_ && scan_pub_)
        {
            auto scan = std::make_unique<sensor_msgs::msg::LaserScan>();
            scan->header.frame_id = frame_id_;
            scan->header.stamp = this->now();

            // 计算弧度范围
            double start_raw = frame_start_angle_raw_;
            double end_raw = frame_end_angle_raw_;
            if (end_raw < start_raw)
                end_raw += ANGLE_MAX_RAW;
            double start_rad = (start_raw / 100.0) * (M_PI / 180.0);
            double end_rad = (end_raw / 100.0) * (M_PI / 180.0);

            size_t N = scan_points_.size();
            scan->angle_min = start_rad;
            scan->angle_max = end_rad;
            scan->angle_increment = (N > 1) ? (end_rad - start_rad) / (N - 1) : 0.0;
            scan->time_increment = 0.0; // 可从转速计算
            scan->range_min = range_cm_min_ / 100.0;
            scan->range_max = range_cm_max_ / 100.0;

            scan->ranges.resize(N);
            scan->intensities.resize(N);

            for (size_t i = 0; i < N; ++i)
            {
                const auto &pt = scan_points_[i];
                if (isPointValid(pt))
                {
                    scan->ranges[i] = static_cast<float>(pt.distance);
                    scan->intensities[i] = static_cast<float>(pt.intensity);
                }
                else
                {
                    scan->ranges[i] = std::numeric_limits<float>::infinity();
                    scan->intensities[i] = 0.0f;
                }
            }
            scan_pub_->publish(std::move(scan));
        }

        // ---------- PointCloud2 ----------
        if (topic_open_point_cloud2_ && pointcloud2_pub_)
        {
            sensor_msgs::msg::PointCloud2 cloud;
            cloud.header.frame_id = frame_id_;
            cloud.header.stamp = this->now();
            cloud.height = 1;
            cloud.is_bigendian = false;
            cloud.is_dense = true; // 只包含有效点

            // 定义字段: x, y, z, intensity (float32)
            sensor_msgs::msg::PointField field;
            field.name = "x";
            field.offset = 0;
            field.datatype = sensor_msgs::msg::PointField::FLOAT32;
            field.count = 1;
            cloud.fields.push_back(field);
            field.name = "y";
            field.offset = 4;
            cloud.fields.push_back(field);
            field.name = "z";
            field.offset = 8;
            cloud.fields.push_back(field);
            field.name = "intensity";
            field.offset = 12;
            cloud.fields.push_back(field);

            cloud.point_step = 16; // 4 floats

            // 先统计有效点数
            size_t valid_count = 0;
            for (const auto &pt : cloud_points_)
                if (isPointValid(pt))
                    valid_count++;

            cloud.width = valid_count;
            cloud.row_step = cloud.point_step * valid_count;
            cloud.data.resize(cloud.row_step);

            // 填充数据
            size_t offset = 0;
            for (const auto &pt : cloud_points_)
            {
                if (!isPointValid(pt))
                    continue;

                float x = static_cast<float>(pt.distance * cos(pt.degree));
                float y = static_cast<float>(pt.distance * sin(pt.degree));
                float z = 0.0f;
                float intensity = static_cast<float>(pt.intensity);

                memcpy(&cloud.data[offset], &x, sizeof(float));
                offset += sizeof(float);
                memcpy(&cloud.data[offset], &y, sizeof(float));
                offset += sizeof(float);
                memcpy(&cloud.data[offset], &z, sizeof(float));
                offset += sizeof(float);
                memcpy(&cloud.data[offset], &intensity, sizeof(float));
                offset += sizeof(float);
            }

            pointcloud2_pub_->publish(cloud);
        }
    }

    // ========== 构造函数 ==========
    Serial::Serial(std::string nodeName) : Node(nodeName)
    {
        // 声明参数
        this->declare_parameter<std::string>("lidar_name", "N10P");
        this->declare_parameter<std::string>("serial_port", "/dev/ttyUSB0");
        this->declare_parameter<std::string>("frame_id", "laser");
        this->declare_parameter<bool>("topic_open_scan", true);
        this->declare_parameter<std::string>("topic_scan_name", "/scan");
        this->declare_parameter<bool>("topic_open_point_cloud2", true);
        this->declare_parameter<std::string>("topic_point_cloud2_name", "/lslidar_point_cloud2");
        this->declare_parameter<std::vector<double>>("angle_range", {0.0, 360.0});
        this->declare_parameter<std::vector<double>>("distance_range", {0.0, 200.0});

        // 获取基础参数
        this->get_parameter("lidar_name", lidar_name_);
        this->get_parameter("serial_port", serial_port_);
        this->get_parameter("frame_id", frame_id_);
        this->get_parameter("topic_open_scan", topic_open_scan_);
        this->get_parameter("topic_scan_name", topic_scan_name_);
        this->get_parameter("topic_open_point_cloud2", topic_open_point_cloud2_);
        this->get_parameter("topic_point_cloud2_name", topic_point_cloud2_name_);

        // 获取并转换角度范围
        std::vector<double> angle_flat;
        if (this->get_parameter("angle_range", angle_flat))
            angle_range_ = convertFlatToPairs(angle_flat, "angle_range");
        else
            angle_range_ = {{0.0, 360.0}};

        // 获取并转换距离范围
        std::vector<double> dist_flat;
        if (this->get_parameter("distance_range", dist_flat))
            distance_range_ = convertFlatToPairs(dist_flat, "distance_range");
        else
            distance_range_ = {{0.0, 200.0}};

        // 计算全局距离最值（cm）
        range_cm_min_ = distance_range_.front().first;
        range_cm_max_ = distance_range_.front().second;
        for (const auto &p : distance_range_)
        {
            if (p.first < range_cm_min_)
                range_cm_min_ = p.first;
            if (p.second > range_cm_max_)
                range_cm_max_ = p.second;
        }

        // 打印配置
        RCLCPP_INFO(this->get_logger(), "Angle ranges:");
        for (const auto &p : angle_range_)
            RCLCPP_INFO(this->get_logger(), "  [%.1f, %.1f]", p.first, p.second);
        RCLCPP_INFO(this->get_logger(), "Distance ranges: [%.1f, %.1f]", range_cm_min_, range_cm_max_);
        RCLCPP_INFO(this->get_logger(), "LiDAR: %s, Port: %s, Frame: %s, Baud: %d",
                    lidar_name_.c_str(), serial_port_.c_str(), frame_id_.c_str(), BAUD_RATE_);

        // 创建发布器
        if (topic_open_scan_)
            scan_pub_ = this->create_publisher<sensor_msgs::msg::LaserScan>(topic_scan_name_, 10);
        if (topic_open_point_cloud2_)
            pointcloud2_pub_ = this->create_publisher<sensor_msgs::msg::PointCloud2>(topic_point_cloud2_name_, 10);

        // 预分配内存
        degree_points_.reserve(1152);
        distance_points_.reserve(1152);
        intensity_points_.reserve(1152);
    }

    // ========== 启动线程 ==========
    void Serial::threadStart()
    {
        RCLCPP_INFO(this->get_logger(), "Initializing UART on %s with baud %d...",
                    serial_port_.c_str(), BAUD_RATE_);

        uart_device_ = new Uart::Uart;
        uart_device_->setBaudrate(BAUD_RATE_);
        uart_device_->setDataFlagBit(Uart::UART_DATA8);
        uart_device_->setStopFlagBit(Uart::UART_STOP1);
        uart_device_->setCheckFlagBit(Uart::UART_NONE);

        if (!uart_device_->openUartSerial(serial_port_))
        {
            RCLCPP_ERROR(this->get_logger(), "Failed to open serial port %s", serial_port_.c_str());
            return;
        }
        if (!uart_device_->checkUartOpen())
        {
            RCLCPP_ERROR(this->get_logger(), "Port %s not open. Check permissions!", serial_port_.c_str());
            return;
        }
        RCLCPP_INFO(this->get_logger(), "Serial port opened successfully.");

        running_ = true;
        read_thread_ = std::thread(&Serial::threadRead, this);
        // publish_thread_ = std::thread(&Serial::threadPublish, this);
        RCLCPP_INFO(this->get_logger(), "Driver started.");
    }

    // ========== 析构函数 ==========
    Serial::~Serial()
    {
        running_ = false;
        if (read_thread_.joinable())
            read_thread_.join();
        if (publish_thread_.joinable())
            publish_thread_.join();
        delete uart_device_;
    }

} // namespace Driver