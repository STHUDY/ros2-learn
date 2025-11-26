cmake_minimum_required(VERSION 3.8)
project(learn_cmake)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(OpenCV REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(learn_interface REQUIRED)
find_package(cv_bridge REQUIRED)

# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

# set helloworld node
add_executable(node_helloword src/helloworld.cpp)
ament_target_dependencies(node_helloword rclcpp)

# set opencv node
add_executable(node_opencv_find_point src/opencv_find_point.cpp)
ament_target_dependencies(node_opencv_find_point rclcpp OpenCV)
target_link_libraries(node_opencv_find_point ${OpenCV_LIBS})

# set helloworld topic publisher node
add_executable(node_topic_helloworld_publisher src/topic_helloworld/helloworld_publisher.cpp)
ament_target_dependencies(node_topic_helloworld_publisher rclcpp std_msgs)

# set helloworld topic subscriber node
add_executable(node_topic_helloworld_subscriber src/topic_helloworld/helloworld_subscriber.cpp)
ament_target_dependencies(node_topic_helloworld_subscriber rclcpp std_msgs)

# set opencv topic publisher node
add_executable(node_topic_opencv_find_point_publisher src/topic_opencv/opencv_find_point_publisher.cpp)
ament_target_dependencies(node_topic_opencv_find_point_publisher rclcpp std_msgs sensor_msgs geometry_msgs learn_interface cv_bridge)
target_link_libraries(node_topic_opencv_find_point_publisher ${OpenCV_LIBS})

# set opencv topic subscriber node
add_executable(node_topic_opencv_find_point_subscriber src/topic_opencv/opencv_find_point_subscriber.cpp)
ament_target_dependencies(node_topic_opencv_find_point_subscriber rclcpp std_msgs sensor_msgs geometry_msgs learn_interface cv_bridge)
target_link_libraries(node_topic_opencv_find_point_subscriber ${OpenCV_LIBS})

# set opencv service publisher node
add_executable(node_service_opencv_find_point_server src/service_opencv/opencv_find_point_server.cpp)
ament_target_dependencies(node_service_opencv_find_point_server rclcpp std_msgs sensor_msgs geometry_msgs learn_interface cv_bridge)
target_link_libraries(node_service_opencv_find_point_server ${OpenCV_LIBS})

# set opencv service subscriber node
add_executable(node_service_opencv_find_point_client src/service_opencv/opencv_find_point_client.cpp)
ament_target_dependencies(node_service_opencv_find_point_client rclcpp std_msgs sensor_msgs geometry_msgs learn_interface cv_bridge)
target_link_libraries(node_service_opencv_find_point_client ${OpenCV_LIBS})

# Install lib for command run
install(TARGETS
  node_helloword
  node_opencv_find_point
  node_topic_helloworld_publisher
  node_topic_helloworld_subscriber
  node_topic_opencv_find_point_publisher
  node_topic_opencv_find_point_subscriber
  node_service_opencv_find_point_server
  node_service_opencv_find_point_client
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
