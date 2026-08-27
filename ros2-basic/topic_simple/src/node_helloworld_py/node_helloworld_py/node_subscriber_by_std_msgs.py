import rclpy
from rclpy.node import Node
from std_msgs.msg import String


class NodeHelloworld(Node):
    def __init__(self, name: str):
        super().__init__(name)
        self.subscription_ = self.create_subscription(
            String, "helloworld_py_by_std_msgs", self.subscriber_callback, 10
        )

    def subscriber_callback(self, msg):
        self.get_logger().info(msg.data)


def main(args=None):
    rclpy.init(args=args)
    node = NodeHelloworld("node_topic_subscriber_helloworld_py_by_std_msgs")
    rclpy.spin(node)
    rclpy.shutdown()
