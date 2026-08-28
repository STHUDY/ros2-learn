#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from node_helloworld_interface.srv import Answer


class NodeHelloworldClient(Node):
    def __init__(self, name: str):
        super().__init__(name)

        # 创建服务客户端
        self.client = self.create_client(Answer, "helloworld_py_by_helloworld_interface")

        # 等待服务端启动（循环检测）
        while not self.client.wait_for_service(timeout_sec=1.0):
            if not rclpy.ok():
                self.get_logger().error("Interrupted while waiting for service.")
                return
            self.get_logger().warn("Waiting for service 'helloworld_py_by_helloworld_interface'...")

        # 创建定时器，每秒发送一次请求
        self.timer = self.create_timer(1.0, self.timer_callback)

    def timer_callback(self):
        # 构造请求
        request = Answer.Request()
        request.name = "client_py"

        # 异步发送请求，并绑定回调处理响应
        future = self.client.call_async(request)
        future.add_done_callback(self.response_callback)

    def response_callback(self, future):
        try:
            response = future.result()
            self.get_logger().info(f"Server response: {response.message}")
        except Exception as e:
            self.get_logger().error(f"Service call failed: {e}")


def main(args=None):
    rclpy.init(args=args)
    node = NodeHelloworldClient("node_client_helloworld_py_by_helloworld_interface")
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()
