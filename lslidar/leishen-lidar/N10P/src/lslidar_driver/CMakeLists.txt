cmake_minimum_required(VERSION 3.8)
project(lslidar_driver)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(PCL REQUIRED COMPONENTS common)

# 添加 lwrb 库（C 语言库）
add_library(lwrb STATIC
  depends/lwrb/lwrb.c
  depends/lwrb/lwrb_ex.c
)
target_include_directories(lwrb PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/depends
)
# 确保 lwrb 作为 C 代码编译
set_target_properties(lwrb PROPERTIES
  LINKER_LANGUAGE C
)

# 添加 Uart 库
add_library(uart STATIC
  depends/Uart/Uart.cpp
)
target_include_directories(uart PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/depends
)

# 添加 serial 库
add_library(serial_lib STATIC
  src/driver/serial.cpp
)
target_include_directories(serial_lib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include/driver
  ${CMAKE_CURRENT_SOURCE_DIR}/depends
  ${PCL_INCLUDE_DIRS}
)
ament_target_dependencies(serial_lib
  rclcpp
  sensor_msgs
  geometry_msgs
  pcl_conversions
)
target_link_libraries(serial_lib
  uart
  lwrb
  ${PCL_LIBRARIES}
)

# 创建可执行文件
add_executable(lslidar_driver_node src/lslidar_driver_node.cpp)
target_link_libraries(lslidar_driver_node
  serial_lib
)
ament_target_dependencies(lslidar_driver_node
  rclcpp
)

# 安装头文件
install(DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

# 安装配置文件
install(DIRECTORY config/
  DESTINATION share/${PROJECT_NAME}/config
)

# 安装 launch 文件
install(DIRECTORY launch/
  DESTINATION share/${PROJECT_NAME}/launch
)

# 安装 rviz 配置文件
install(DIRECTORY rviz/
  DESTINATION share/${PROJECT_NAME}/rviz
)

# 安装可执行文件
install(TARGETS
  lslidar_driver_node
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
