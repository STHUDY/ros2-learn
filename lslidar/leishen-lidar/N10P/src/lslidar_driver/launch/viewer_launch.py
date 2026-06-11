"""
Launch file for Leishen LiDAR N10P driver with RViz2 visualization.

This launch file starts the lslidar_driver_node and RViz2
for visualization.
"""

from launch import LaunchDescription
from launch_ros.actions import Node
from launch.substitutions import PathJoinSubstitution
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    """Generate launch description for lslidar driver with RViz2."""
    # Get package share directory
    package_name = 'lslidar_driver'
    package_share = get_package_share_directory(package_name)

    # Path to config file
    config_file = PathJoinSubstitution(
        [package_share, 'config', 'n10p.yaml']
    )

    # Create lslidar driver node
    lslidar_driver_node = Node(
        package=package_name,
        executable='lslidar_driver_node',
        name='lslidar_driver_node',
        output='screen',
        parameters=[config_file],
        emulate_tty=True
    )

    # Create RViz2 node
    rviz_config_file = PathJoinSubstitution(
        [package_share, 'rviz', 'lslidar_rviz.rviz']
    )

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', rviz_config_file]
    )

    return LaunchDescription([
        lslidar_driver_node,
        rviz_node
    ])
