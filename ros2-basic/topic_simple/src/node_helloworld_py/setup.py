from glob import glob
import os

from setuptools import find_packages, setup

package_name = "node_helloworld_py"

setup(
    name=package_name,
    version="0.0.0",
    packages=find_packages(exclude=["test"]),
    data_files=[
        ("share/ament_index/resource_index/packages", ["resource/" + package_name]),
        ("share/" + package_name, ["package.xml"]),
        (os.path.join("share", package_name, "launch"), glob("launch/*.launch.py")),
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    maintainer="sthudy",
    maintainer_email="zylqqwyy@outlook.com",
    description="TODO: Package description",
    license="TODO: License declaration",
    extras_require={
        "test": [
            "pytest",
        ],
    },
    entry_points={
        "console_scripts": [
            "node_publisher_by_std_msgs = node_helloworld_py.node_publisher_by_std_msgs:main",
            "node_subscriber_by_std_msgs = node_helloworld_py.node_subscriber_by_std_msgs:main",
            "node_publisher_by_helloworld_interface = node_helloworld_py.node_publisher_by_helloworld_interface:main",
            "node_subscriber_by_helloworld_interface = node_helloworld_py.node_subscriber_by_helloworld_interface:main",
        ],
    },
)
