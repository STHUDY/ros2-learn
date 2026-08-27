import rclpy
from rclpy.node import Node


class NodeHelloworld(Node):
    def __init__(self, name: str):
        super().__init__(name)
        self.count_ = 0
        # 创建 1 秒周期的定时器
        self.timer_ = self.create_timer(1.0, self.timer_callback)

    def timer_callback(self):
        self.count_ += 1
        self.get_logger().info(f"Hello World Python: {self.count_}")


def main(args=None):
    rclpy.init(args=args)
    node = NodeHelloworld("node_helloworld_cpp")
    rclpy.spin(node)
    rclpy.shutdown()

