## 依赖

```
sudo apt install colcon python3-colcon-common-extensions
sudo apt install libopencv-dev
```

## 编译

首先编译自定义接口

```
colcon build --packages-select learn_interface
```

编译代码

```
colcon build
```

## 使用

### 编译完成后加载环境

```
source install/setup.bash
```

### 一般调用

```
# python
ros2 run learn_py node_helloword 
ros2 run learn_py node_opencv_find_point

# cmake
ros2 run learn_cmake node_helloword 
ros2 run learn_cmake node_opencv_find_point
```

### topic话题

```
# python
ros2 run learn_py node_topic_helloworld_publisher
ros2 run learn_py node_topic_helloworld_subscriber

# cmake
ros2 run learn_cmake node_topic_helloworld_publisher
ros2 run learn_cmake node_topic_helloworld_subscriber
```

### topic话题(自定义)

topic消息包 learn_interface.msg.TopicOpencv

```
# python
ros2 run learn_py node_topic_opencv_find_point_publisher
ros2 run learn_py node_topic_opencv_find_point_subscriber

# cmake
ros2 run learn_cmake node_topic_opencv_find_point_publisher
ros2 run learn_cmake node_topic_opencv_find_point_subscriber
```

### service服务(自定义)

服务消息包 learn_interface.msg.TopicOpencv

```
# python
ros2 run learn_py node_service_opencv_find_point_server
ros2 run learn_py node_service_opencv_find_point_client

# cmake
ros2 run learn_cmake node_service_opencv_find_point_server
ros2 run learn_cmake node_service_opencv_find_point_client
```
