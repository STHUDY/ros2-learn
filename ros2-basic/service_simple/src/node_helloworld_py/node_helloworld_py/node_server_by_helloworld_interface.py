#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from node_helloworld_interface.srv import Answer


class NodeHelloworld(Node):
    def __init__(self, name: str):
        super().__init__(name)
        self.count_ = 0
        # 创建服务
        self.server = self.create_service(
            Answer,
            "helloworld_py_by_helloworld_interface",  # 服务名称标识为 Python
            self.answer_callback,
        )

    def answer_callback(self, request, response):
        name = request.name
        self.count_ += 1
        response.message = f"[{name}] Hello World Python(node_helloworld_interface) Server: {self.count_}"
        self.get_logger().info(
            f"Received request from [{name}], response count: {self.count_}"
        )
        return response


def main(args=None):
    rclpy.init(args=args)
    node = NodeHelloworld("node_server_helloworld_py_by_helloworld_interface")
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()
