# encoding: utf-8
import base64
import uuid
import time
import requests
import cv2
import threading
from queue import Queue
from auth_util import gen_sign_headers

# 请替换APP_ID、APP_KEY
APP_ID = '**************'
APP_KEY = 'KiMANYdWbGDdFOyU'
URI = '/vivogpt/completions/stream'
DOMAIN = 'api-ai.vivo.com.cn'
METHOD = 'POST'
PIC_FILE = 'test.jpg'


def capture_image(camera_index=0, output_queue=None):
    """摄像头图像捕获线程"""
    cap = cv2.VideoCapture(camera_index)
    if not cap.isOpened():
        print("无法打开摄像头，请检查摄像头连接")
        return
    
    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                print("无法从摄像头获取帧")
                break
                
            # 调整图像大小以提高处理速度
            frame = cv2.resize(frame, (640, 480))
            
            _, buffer = cv2.imencode('.jpg', frame, [int(cv2.IMWRITE_JPEG_QUALITY), 90])
            image_bytes = buffer.tobytes()
            
            if output_queue:
                if output_queue.full():
                    output_queue.get()  # 丢弃旧帧
                output_queue.put(image_bytes)
                
            cv2.imshow('Camera - 按Q键退出', frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
    finally:
        cap.release()
        cv2.destroyAllWindows()

def process_frame(image_bytes, params):
    """处理单帧图像并调用API"""
    image = base64.b64encode(image_bytes).decode('utf-8')
    data = {
        'prompt': '你好',
        'sessionId': str(uuid.uuid4()),
        'requestId': params['requestId'],
        'model': 'vivo-BlueLM-V-2.0',
        "messages": [
            {
                "role": "user",
                "content": "data:image/JPEG;base64," + image,
                "contentType": "image"
            },
            {
                "role": "user",
                "content": "根据图片的内容来描述相对位置",
                "contentType": "text"
            }
        ],
    }
    headers = gen_sign_headers(APP_ID, APP_KEY, METHOD, URI, params)
    headers['Content-Type'] = 'application/json'

    url = 'http://{}{}'.format(DOMAIN, URI)
    response = requests.post(url, json=data, headers=headers, params=params, stream=True)

    if response.status_code == 200:
        for line in response.iter_lines():
            if line:
                print(line.decode('utf-8').encode('gbk', errors='replace').decode('gbk'))
    else:
        print(str(response.status_code).encode('gbk', errors='replace').decode('gbk'), 
              response.text.encode('gbk', errors='replace').decode('gbk'))

def stream_vivogpt():
    params = {
        'requestId': str(uuid.uuid4())
    }
    print('requestId:', params['requestId'])
    
    # 创建图像队列和处理状态
    image_queue = Queue(maxsize=1)  # 只保留最新一帧
    processing_done = threading.Event()
    processing_done.set()  # 初始状态为已完成
    
    # 启动摄像头线程
    camera_thread = threading.Thread(
        target=capture_image,
        kwargs={'output_queue': image_queue}
    )
    camera_thread.daemon = True
    camera_thread.start()
    
    # 持续处理视频流
    try:
        while True:
            if processing_done.is_set():  # 只有前一次处理完成才获取新帧
                b_image = image_queue.get()
                processing_done.clear()  # 标记为处理中
                
                # 启动处理线程
                processing_thread = threading.Thread(
                    target=process_frame_wrapper,
                    args=(b_image, params, processing_done)
                )
                processing_thread.daemon = True
                processing_thread.start()
                
            time.sleep(0.1)  # 避免忙等待
    except KeyboardInterrupt:
        print("\n停止视频流处理")

def process_frame_wrapper(image_bytes, params, done_event):
    """包装处理函数，完成后设置事件"""
    try:
        process_frame(image_bytes, params)
    finally:
        done_event.set()  # 处理完成


if __name__ == "__main__":
    print("启动视频流处理，按Ctrl+C停止")
    stream_vivogpt()