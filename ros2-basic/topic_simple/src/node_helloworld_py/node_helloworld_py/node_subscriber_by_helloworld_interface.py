import rclpy
from rclpy.node import Node
from node_helloworld_interface.msg import Message


class NodeHelloworld(Node):
    def __init__(self, name: str):
        super().__init__(name)
        self.subscription_ = self.create_subscription(
            Message,
            "helloworld_py_by_helloworld_interface",
            self.subscriber_callback,
            10,
        )

    def subscriber_callback(self, msg):
        self.get_logger().info(f"[{msg.name}] I heard: '{msg.message}'")


def main(args=None):
    rclpy.init(args=args)
    node = NodeHelloworld("node_topic_subscriber_helloworld_py_by_helloworld_interface")
    rclpy.spin(node)
    rclpy.shutdown()
