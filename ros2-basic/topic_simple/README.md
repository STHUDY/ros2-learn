## 介绍 
这是简单的话题发布与订阅的（包含自定义节点）例程

## 依赖

```
sudo apt install colcon python3-colcon-common-extensions
```

## 编译

首先编译自定义接口

```
colcon build --packages-select node_helloworld_interface
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


使用std_msgs接口发布

C++

```
ros2 launch node_helloworld_cpp node_publisher_by_std_msgs.launch.py
ros2 launch node_helloworld_cpp node_subscriber_by_std_msgs.launch.py
```

Python

```
ros2 launch node_helloworld_py node_publisher_by_std_msgs.launch.py
ros2 launch node_helloworld_py node_subscriber_by_std_msgs.launch.py
```

使用自定义接口发布

C++ 

```
ros2 launch node_helloworld_cpp node_publisher_by_helloworld_interface.launch.py
ros2 launch node_helloworld_cpp node_subscriber_by_helloworld_interface.launch.py
```

Python
```
ros2 launch node_helloworld_py node_publisher_by_helloworld_interface.launch.py
ros2 launch node_helloworld_py node_subscriber_by_helloworld_interface.launch.py
```
