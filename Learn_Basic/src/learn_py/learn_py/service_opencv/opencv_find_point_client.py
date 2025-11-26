import cv2
import rclpy
from rclpy.node import Node
from learn_interface.srv import TopicOpencv
from cv_bridge import CvBridge
import time

class SubscriberNode(Node):
    def __init__(self, name):
        super().__init__(name)
        self.client = self.create_client(TopicOpencv, "learn_service_opencv_find_point")
        self.bridge = CvBridge()
        
    def wait_for_server(self, timeout_sec = 60):
        self.get_logger().info('等待服务可用...')
        if self.client.wait_for_service(timeout_sec=timeout_sec):
            self.get_logger().info('服务连接成功')
            return True
        else:
            self.get_logger().error(f'服务在 {timeout_sec} 秒内不可用')
            return False
        
    
    def send_request(self, scan_once=True):
        if not self.client.service_is_ready():
            self.get_logger().error('服务未就绪')
            return None
            
        request = TopicOpencv.Request()
        request.scan_once = scan_once
        
        # self.get_logger().info(f'发送请求: scan_once={scan_once}')
        
        future = self.client.call_async(request)
        return future
        
    def handle_msg(self, response):
        
        if not response:
            self.get_logger().error('无响应数据')
            return False
        
        
        # 处理图像
        try:
            cv_image = self.bridge.imgmsg_to_cv2(response.image, "bgr8")
            cv2.imshow("Received Image", cv_image)
            cv2.waitKey(1)
        except Exception as e:
            self.get_logger().error(f"图像错误: {str(e)}")
        
        # 处理点
        if response.points:
            self.get_logger().info(f"检测到 {len(response.points)} 个点")
            for i, point in enumerate(response.points):
                self.get_logger().info(f"点{i+1}: ({point.x:.1f}, {point.y:.1f} {point.z:.1f})")

def main(args=None):
    rclpy.init(args=args)
    client = SubscriberNode("opencv_find_point_service_node")
    
    if not client.wait_for_server():
        return
    
    fps = 24
    
    while rclpy.ok():
        future = client.send_request(scan_once=True)
        if future:
            rclpy.spin_until_future_complete(client, future)
            response = future.result()
            client.handle_msg(response)
        
        time.sleep(1 / fps)
    
    try:
        rclpy.spin(client)
    except KeyboardInterrupt:
        pass
    finally:
        cv2.destroyAllWindows()
        client.destroy_node()
        rclpy.shutdown()
