import cv2
import numpy as np
import rclpy
from rclpy.node import Node

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
        
def main(args = None):
    
    rclpy.init(args=args)
    
    node = Node("opencv_node")
    
    cap = cv2.VideoCapture(0)

    while True:
        ret, frame = cap.read()
        if not ret:
            break
        try:
            isFind, points = Find_Point(frame)
            if isFind:
                draw_point_circle(frame, points)
                node.get_logger().info(f"Found {len(points)} points : {points}")

            cv2.imshow("photo", frame)
        except Exception as e:
            print(e)  # 打印异常信息
            continue

        # 按下'q'键退出循环
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # 释放摄像头资源
    cap.release()
    cv2.destroyAllWindows()
    rclpy.shutdown()