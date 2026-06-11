# Leishen LiDAR N10P ROS2 Driver

## 概述

本包提供了雷赛 N10P 激光雷达的 ROS2 驱动支持。

## 构建

```bash
cd ~/ros2_ws
colcon build --packages-select lslidar_driver
source install/setup.bash
```

## 使用方法

### 1. 启动单个雷达（使用配置文件）

```bash
# 使用默认串口 /dev/ttyACM0
ros2 launch lslidar_driver lslidar_launch.py

# 指定串口号
ros2 launch lslidar_driver lslidar_launch.py serial_port:=/dev/ttyUSB0
```

此命令会加载 `config/n10p.yaml` 中的配置并启动驱动节点。

### 2. 启动雷达并打开 RViz2 可视化

```bash
ros2 launch lslidar_driver viewer_launch.py
```

此命令会同时启动驱动节点和 RViz2，自动加载预设的可视化配置。

### 3. 启动双雷达系统

```bash
ros2 launch lslidar_driver dual_lidar_launch.py
```

此命令会启动两个雷达节点，分别使用不同的串口和命名空间。

## 配置文件说明

### config/n10p.yaml

主配置文件，包含以下参数：

- `lidar_name`: 雷达名称
- `serial_port`: 串口号（如 `/dev/ttyACM0`）
- `frame_id`: TF 坐标系 ID
- `topic`:
  - `open_scan`: 是否发布 LaserScan 话题
  - `scan`: LaserScan 话题名称
  - `open_point_cloud2`: 是否发布 PointCloud2 话题
  - `point_cloud2`: PointCloud2 话题名称
- `angle_range`: 角度裁剪范围（度）
- `distance_range`: 距离裁剪范围（厘米）

### config/n10p_2.yaml

第二个雷达的配置文件，用于双雷达场景。

## 话题

- `/scan` - 激光扫描数据 (sensor_msgs/msg/LaserScan)
- `/lslidar_point_cloud2` - 点云数据 (sensor_msgs/msg/PointCloud2)

## 故障排查

### 1. 检查串口设备

```bash
# 查看所有串口设备
ls -l /dev/ttyACM* /dev/ttyUSB*

# 查看串口权限
ls -l /dev/ttyACM0
```

### 2. 设置串口权限

```bash
# 将用户添加到 dialout 组
sudo usermod -a -G dialout $USER

# 注意：需要重新登录才能生效
```

### 3. 测试串口连接

```bash
# 安装 minicom
sudo apt install minicom

# 测试串口（波特率 460800）
minicom -D /dev/ttyACM0 -b 460800
```

### 4. 检查节点状态

```bash
# 查看节点是否运行
ros2 node list

# 查看话题列表
ros2 topic list

# 查看话题频率
ros2 topic hz /scan

# 查看消息内容
ros2 topic echo /scan --once
```

### 5. 常见问题

**问题：没有数据输出**
- 检查串口设备是否正确连接
- 确认串口号与配置文件一致
- 检查用户是否有串口访问权限
- 查看节点日志：`ros2 log`

**问题：权限被拒绝**
```bash
sudo chmod 666 /dev/ttyACM0
```

**问题：找不到设备**
- 确认雷达已上电
- 检查 USB 连接线
- 尝试其他 USB 端口

## 注意事项

1. 确保用户有串口访问权限：
   ```bash
   sudo usermod -a -G dialout $USER
   ```
   然后重新登录。

2. 检查串口号是否正确：
   ```bash
   ls -l /dev/ttyACM*
   ```

3. 如果修改了配置文件，需要重新构建包：
   ```bash
   colcon build --packages-select lslidar_driver
   ```
