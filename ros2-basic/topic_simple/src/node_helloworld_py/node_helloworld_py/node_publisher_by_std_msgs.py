import rclpy
from rclpy.node import Node
from std_msgs.msg import String


class NodeHelloworld(Node):
    def __init__(self, name: str):
        super().__init__(name)
        self.count_ = 0
        # 创建 1 秒周期的定时器
        self.timer_ = self.create_timer(1.0, self.timer_callback)

        self.publisher_ = self.create_publisher(String, "helloworld_py_by_std_msgs", 10)

    def timer_callback(self):
        msg = String()
        msg.data = f"Hello World Python(std_msgs) Publisher:: {self.count_}"
        self.publisher_.publish(msg)
        self.get_logger().info(f"Hello World Python: {self.count_}")
        self.count_ += 1


def main(args=None):
    rclpy.init(args=args)
    node = NodeHelloworld("node_topic_publisher_helloworld_py_by_std_msgs")
    rclpy.spin(node)
    rclpy.shutdown()
