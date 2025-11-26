from setuptools import find_packages, setup

package_name = 'learn_py'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='nzh',
    maintainer_email='nzh@todo.todo',
    description='TODO: Package description',
    license='TODO: License declaration',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            "node_helloword = learn_py.helloworld:main",
            "node_opencv_find_point = learn_py.opencv_find_point:main",
            "node_topic_helloworld_publisher = learn_py.topic_helloworld.helloworld_publisher:main",
            "node_topic_helloworld_subscriber = learn_py.topic_helloworld.helloworld_subscriber:main",
            "node_topic_opencv_find_point_publisher = learn_py.topic_opencv.opencv_find_point_publisher:main",
            "node_topic_opencv_find_point_subscriber = learn_py.topic_opencv.opencv_find_point_subscriber:main",
            "node_service_opencv_find_point_server = learn_py.service_opencv.opencv_find_point_server:main",
            "node_service_opencv_find_point_client = learn_py.service_opencv.opencv_find_point_client:main",
            ],
    },
)
