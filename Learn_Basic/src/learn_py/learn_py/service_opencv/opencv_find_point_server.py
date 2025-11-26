import cv2
import numpy as np
import rclpy
from rclpy.node import Node
from learn_interface.srv import TopicOpencv
from geometry_msgs.msg import Point
from sensor_msgs.msg import Image
from cv_bridge import CvBridge

def Find_Point(origin):
    # 将图像从BGR转换为HSV颜色空间
    hsv = cv2.cvtColor(origin, cv2.COLOR_BGR2HSV)

    # 定义红色的HSV范围
    hsv_lower = np.array([156, 43, 46])
    hsv_upper = np.array([180, 255, 255])
    mask = cv2.inRange(hsv, hsv_lower, hsv_upper)
    kernel = np.ones((2, 2), np.uint8)
    mask_opened = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
    contours, _ = cv2.findContours(mask_opened, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)

    # 处理查找结果
    radius_max = 0
    point_result = []
    for cnt in contours:
        epsilon = 0.1 * cv2.arcLength(cnt, True)
        approx = cv2.approxPolyDP(cnt, epsilon, True)
        # 拟合
        (x, y), radius = cv2.minEnclosingCircle(approx)
        if radius_max < radius:
            radius_max = radius
            point_result.append((int(x), int(y), int(radius)))  # 添加半径到结果中
            
    return radius_max != 0, point_result


def draw_point_circle(image, posArray):
    for pos in posArray:
        center = (pos[0], pos[1])
        radius = pos[2]  # 使用传入的半径
        cv2.circle(image, center, radius, (255, 0, 0), 2)
        
class PublisherNode(Node):
    def __init__(self, name, cap):
        super().__init__(name)
        self.server = self.create_service(TopicOpencv, "learn_service_opencv_find_point", self.handle_service_msg)
        self.bridge = CvBridge()
        self.cap = cap
        
    def handle_service_msg(self, request, response):
        self.get_logger().info(f'收到请求: scan_once={request.scan_once}')
        
        data = [Image(),[]]
        
        # 根据请求处理
        if request.scan_once:
            data = self.scan_camera()
            
        return self.echo(data, response=response)
    
    def scan_camera(self):
        ret, frame = self.cap.read()
        
        sendmessage = [frame,[]]
        
        if not ret:
            ret
        try:
            isFind, points = Find_Point(frame)
            
            pub_points = []
            
            for point in points:
                if(point[2] > 10):
                    pub_points.append(point)
            
            sendmessage[1] = pub_points
            
            if isFind:
                draw_point_circle(frame, pub_points)

        except Exception as e:
            print(e)  # 打印异常信息
        
        finally:
            return sendmessage
        
    def echo(self, data, response):
        response.image = self.bridge.cv2_to_imgmsg(data[0], encoding="bgr8")
        
        # 转换点数据
        point_msgs = []
        for point in data[1]:
            p_msg = Point()
            p_msg.x = float(point[0])
            p_msg.y = float(point[1])
            p_msg.z = float(point[2])
            point_msgs.append(p_msg)
        
        response.points = point_msgs
        return response

def main(args = None):
    
    rclpy.init(args=args)
    
    cap = cv2.VideoCapture(0)
    
    server = PublisherNode("opencv_find_point_service_node", cap)
    
    try:
        rclpy.spin(server)
    except KeyboardInterrupt:
        pass
    finally:
        cap.release()
        server.destroy_node()
        rclpy.shutdown()