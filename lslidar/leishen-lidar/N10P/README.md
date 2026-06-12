## 介绍 
这是2D激光雷达

## 依赖

```
sudo apt install colcon python3-colcon-common-extensions
sudo apt install libpcap-dev
sudo apt install ros-$ROS_DISTRO-slam-toolbox
sudo apt install ros-$ROS_DISTRO-cartographer ros-$ROS_DISTRO-cartographer-ros
```

## 编译

首先编译自定义接口

```
colcon build --packages-select lslidar_msgs
```

编译代码

```
colcon build
```

低内存设备（从下面命令当中选择一个）

```
colcon build --executor sequential
```

```
colcon build --parallel-workers 1 
```

超低内存编译命令

```
MAKEFLAGS="-j1" colcon build --executor sequential
```

## 运行

静默启动

启动与关闭
```
ros2 topic pub -1 /lslidar_order std_msgs/msg/Int8 data:\ 1 		(开启雷达)
ros2 topic pub -1 /lslidar_order std_msgs/msg/Int8 data:\ 0 		(关闭雷达)
```

## 提示

SLAM 建模必须使用 IMU 数据

Cartographer 可单激光雷达建模