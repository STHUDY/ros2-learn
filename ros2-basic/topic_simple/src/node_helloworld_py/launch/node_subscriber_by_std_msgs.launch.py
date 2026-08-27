import os

from ament_index_python import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():

    node_subscriber_by_std_msgs = Node(
        package="node_helloworld_py",
        executable="node_subscriber_by_std_msgs",
        output="screen",
        emulate_tty=True,
    )

    return LaunchDescription(
        [
            node_subscriber_by_std_msgs,
        ]
    )
