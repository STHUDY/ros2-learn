import cv2
import rclpy
from rclpy.node import Node
from learn_interface.msg import TopicOpencv
from cv_bridge import CvBridge

class SubscriberNode(Node):
    def __init__(self, name):
        super().__init__(name)
        self.subscriber_ = self.create_subscription(
            TopicOpencv, "learn_topic_opencv_find_point", self.callback, 10)
        self.bridge = CvBridge()
        
    def callback(self, msg):
        # 处理图像
        try:
            cv_image = self.bridge.imgmsg_to_cv2(msg.image, "bgr8")
            cv2.imshow("Received Image", cv_image)
            cv2.waitKey(1)
        except Exception as e:
            self.get_logger().error(f"图像错误: {str(e)}")
        
        # 处理点
        if msg.points:
            self.get_logger().info(f"检测到 {len(msg.points)} 个点")
            for i, point in enumerate(msg.points):
                self.get_logger().info(f"点{i+1}: ({point.x:.1f}, {point.y:.1f} {point.z:.1f})")

def main(args=None):
    rclpy.init(args=args)
    node = SubscriberNode("opencv_find_point_subscriber_node")
    
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        cv2.destroyAllWindows()
        node.destroy_node()
        rclpy.shutdown()
