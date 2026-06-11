## 介绍 
这是2D激光雷达

## 依赖

```
sudo apt install colcon python3-colcon-common-extensions
sudo apt install libpcap-dev
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