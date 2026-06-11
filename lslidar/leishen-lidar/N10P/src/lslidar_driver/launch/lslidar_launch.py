"""
Launch file for Leishen LiDAR N10P driver.

This launch file starts the lslidar_driver_node with parameters
loaded from config file.
"""

from launch import LaunchDescription
from launch_ros.actions import Node
from launch.substitutions import PathJoinSubstitution
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    """Generate launch description for lslidar driver."""
    # Get package share directory
    package_name = 'lslidar_driver'
    package_share = get_package_share_directory(package_name)

    # Declare launch arguments
    serial_port_arg = DeclareLaunchArgument(
        'serial_port',
        default_value='/dev/ttyCH343USB0',
        description='Serial port for LiDAR'
    )

    # Path to config file
    config_file = PathJoinSubstitution(
        [package_share, 'config', 'n10p.yaml']
    )

    # Create node with parameters from config file
    lslidar_driver_node = Node(
        package=package_name,
        executable='lslidar_driver_node',
        name='lslidar_driver_node',
        output='screen',
        parameters=[
            config_file,
            {
                'serial_port': LaunchConfiguration('serial_port')
            }
        ],
        emulate_tty=True
    )

    return LaunchDescription([
        serial_port_arg,
        lslidar_driver_node
    ])
