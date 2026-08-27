import os

from ament_index_python import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():

    node_helloworld = Node(
        package="node_helloworld_cpp",
        executable="node_helloworld",
        output="screen",
        emulate_tty=True,
    )

    return LaunchDescription(
        [
            node_helloworld,
        ]
    )
