"""
Launch file for dual Leishen LiDAR N10P drivers.

This launch file starts two lslidar_driver_node instances with
different configurations.
"""

from launch import LaunchDescription
from launch_ros.actions import Node
from launch.substitutions import PathJoinSubstitution
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    """Generate launch description for dual lslidar drivers."""
    # Get package share directory
    package_name = 'lslidar_driver'
    package_share = get_package_share_directory(package_name)

    # Path to config files
    config_file_1 = PathJoinSubstitution(
        [package_share, 'config', 'n10p.yaml']
    )

    config_file_2 = PathJoinSubstitution(
        [package_share, 'config', 'n10p_2.yaml']
    )

    # First LiDAR driver node
    lslidar_driver_node_1 = Node(
        package=package_name,
        executable='lslidar_driver_node',
        name='lslidar_driver_node_1',
        namespace='lidar1',
        output='screen',
        parameters=[config_file_1],
        emulate_tty=True,
        remappings=[
            ('/scan', '/lidar1/scan'),
            ('/lslidar_point_cloud2', '/lidar1/lslidar_point_cloud2')
        ]
    )

    # Second LiDAR driver node
    lslidar_driver_node_2 = Node(
        package=package_name,
        executable='lslidar_driver_node',
        name='lslidar_driver_node_2',
        namespace='lidar2',
        output='screen',
        parameters=[config_file_2],
        emulate_tty=True,
        remappings=[
            ('/scan', '/lidar2/scan'),
            ('/lslidar_point_cloud2', '/lidar2/lslidar_point_cloud2')
        ]
    )

    return LaunchDescription([
        lslidar_driver_node_1,
        lslidar_driver_node_2
    ])
