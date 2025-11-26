import rclpy
from rclpy.node import Node
from std_msgs.msg import String

class PublisherNode(Node):
    def __init__(self, name):
        super().__init__(name)
        self.publisher_ = self.create_publisher(String, "learn_helloworld_topic", 10)
        self.timer = self.create_timer(0.5, self.timer_callback)
        self.i = 0
        
    def timer_callback(self):
        msg = String()
        msg.data = "hello world %d" % self.i
        self.publisher_.publish(msg)
        self.get_logger().info("publishing: '%s'" % msg.data)
        self.i += 1

def main(args=None):
    rclpy.init(args=args)
    node = PublisherNode("helloworld_topic_publisher_node")
    
    rclpy.spin(node)
        
    node.destroy_node()
    rclpy.shutdown()