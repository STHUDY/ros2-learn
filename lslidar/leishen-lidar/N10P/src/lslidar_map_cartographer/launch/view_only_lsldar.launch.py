"""
  Copyright 2018 The Cartographer Authors
  Copyright 2022 Wyca Robotics (for the ros2 conversion)

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
"""

from ament_index_python import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node, SetRemap
from launch_ros.substitutions import FindPackageShare
from launch.launch_description_sources import PythonLaunchDescriptionSource
import os


def generate_launch_description():

    # ***** Launch arguments *****
    use_sim_time_arg = DeclareLaunchArgument(
        'use_sim_time', default_value='False')

    bringup_dir = FindPackageShare('lslidar_driver').find('lslidar_driver')
    launch_dir = os.path.join(bringup_dir, 'launch')

    # 启动雷达驱动（假设它发布 /scan 话题和必要的 tf，如 laser_link 的静态变换？通常不包含）
    lslidar_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(launch_dir, 'lslidar_launch.py')
        )
    )

    cartographer_node = Node(
        package='cartographer_ros',
        executable='cartographer_node',
        parameters=[{'use_sim_time': LaunchConfiguration('use_sim_time')}],
        arguments=[
            '-configuration_directory', FindPackageShare('lslidar_map_cartographer').find(
                'lslidar_map_cartographer') + '/config',
            '-configuration_basename', 'only_lsldar.lua'],
        remappings=[
            ('echoes', 'horizontal_laser_2d')],
        output='screen'
    )

    cartographer_occupancy_grid_node = Node(
        package='cartographer_ros',
        executable='cartographer_occupancy_grid_node',
        parameters=[
            {'use_sim_time': True},
            {'resolution': 0.05}],
    )

    rviz_dir = os.path.join(get_package_share_directory(
        'lslidar_map_cartographer'), 'rviz', 'map.rviz')

    rviz_node = Node(
        package='rviz2',
        namespace='',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_dir],
        output='screen')

    return LaunchDescription([
        use_sim_time_arg,
        lslidar_launch,
        cartographer_node,
        cartographer_occupancy_grid_node,
        rviz_node
    ])
