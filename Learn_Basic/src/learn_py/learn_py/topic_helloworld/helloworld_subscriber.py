import rclpy
from rclpy.node import Node
from std_msgs.msg import String

class SubscriberNode(Node):
    def __init__(self, name):
        super().__init__(name)
        self.subscriber_ = self.create_subscription(\
            String, "learn_helloworld_topic", self.callback, 10) 
    def callback(self, msg):
        self.get_logger().info("I heard: '%s'" % msg.data)
            

def main(args=None):
    rclpy.init(args=args)
    node = SubscriberNode("helloworld_topic_subscriber_node")
    rclpy.spin(node)
    rclpy.shutdown()