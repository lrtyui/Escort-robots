import sys
import serial
import time
import threading
import os
import traceback

sys.path.append('/app/Project_Demo')
from Uart_Send_Project.Uart_Moudel import Uart_Send  # 导入左右机械臂函数

# 定义动作字典（包含机械臂类型和参数）
Action_Dict = {
    "待机": {"id": 0, "arms": ["left", "right"], "count": 1},
    "挥手": {"id": 1, "arms": ["right"], "count": 3},
    "拥抱": {"id": 2, "arms": ["left", "right"], "count": 1},
    "敬礼": {"id": 3, "arms": ["left"], "count": 2},
    "举手": {"id": 4, "arms": ["left", "right"], "count": 1},
    "握左手": {"id": 5, "arms": ["left"], "count": 1},
    "握右手": {"id": 6, "arms": ["right"], "count": 1}
}

# 反转字典，通过ID查找动作名称
Id_To_Action = {v["id"]: k for k, v in Action_Dict.items()}

# 配置串口参数
SERIAL_PORT = "/dev/ttyUSB0"  # 根据实际情况修改串口名称
BAUD_RATE = 115200  # 波特率，需与上位机保持一致
TIMEOUT = 0.1  # 超时时间

# 串口连接重试配置
MAX_RETRIES = 5
RETRY_DELAY = 2  # 秒

# 全局变量
ser = None  # 初始化全局变量


# 检查串口设备是否存在
def check_serial_port(device_path):
    return os.path.exists(device_path)


# 初始化串口，带重试机制
def initialize_serial(max_retries=MAX_RETRIES):
    global ser
    retries = 0
    ser = None

    while retries < max_retries:
        try:
            if not check_serial_port(SERIAL_PORT):
                print(f"错误: 串口设备 {SERIAL_PORT} 不存在，尝试重新连接 ({retries + 1}/{max_retries})")
                retries += 1
                time.sleep(RETRY_DELAY)
                continue

            # 关闭已存在的串口连接
            if ser is not None and ser.is_open:
                ser.close()

            # 尝试打开新的串口连接
            ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=TIMEOUT)
            print(f"成功连接到串口: {SERIAL_PORT}")

            # 初始化后发送待机指令（二进制格式0x00）
            ser.write(bytes([0]))
            time.sleep(0.2)
            print("机械臂已初始化，进入待机状态")
            return True

        except serial.SerialException as e:
            print(f"串口初始化失败 ({retries + 1}/{max_retries}): {e}")
            traceback.print_exc()

        except Exception as e:
            print(f"发生未知错误 ({retries + 1}/{max_retries}): {e}")
            traceback.print_exc()

        retries += 1
        time.sleep(RETRY_DELAY)

    print(f"达到最大重试次数 ({max_retries})，串口初始化失败")
    return False


def execute_action(action_name, execute_count=1):
    """执行动作（支持单臂/双臂协同）"""
    action = Action_Dict.get(action_name)
    if not action:
        print(f"错误：未定义动作 '{action_name}'")
        return False

    # 解析动作参数
    group_id = action["id"]
    arms = action["arms"]
    count = execute_count if execute_count else action["count"]  # 优先使用用户输入次数

    # 调用对应机械臂函数
    success = True
    for arm in arms:
        if arm == "right":
            ok, _ = Uart_Send.send_robot_command_Right(group_id, count)
        elif arm == "left":
            ok, _ = Uart_Send.send_robot_command_Left(group_id, count)
        else:
            print(f"错误：未知机械臂类型 '{arm}'")
            ok = False
        success = success and ok  # 所有机械臂调用成功才返回True

    return success


def receive_serial_commands():
    """持续监听串口并执行接收到的指令"""
    global ser  # 声明使用全局变量ser
    if ser is None or not ser.is_open:
        print(f"串口未初始化，无法监听")
        return

    print(f"开始监听串口 {SERIAL_PORT}，波特率 {BAUD_RATE}")

    while True:
        try:
            # 检查串口连接状态
            if ser is None or not ser.is_open:
                print("串口已关闭，尝试重新连接...")
                if not initialize_serial():
                    print("重新连接失败，等待一段时间后重试")
                    time.sleep(RETRY_DELAY)  # 等待一段时间后重试
                    continue

            # 读取串口数据（二进制格式）
            if ser.in_waiting > 0:
                try:
                    # 读取1字节二进制数据
                    data = ser.read(1)
                    if len(data) == 1:
                        # 直接获取二进制值
                        command_id = data[0]
                        # 新增：如果接收到的是ASCII字符'0'-'6'，转换为数值0-6
                        if 0x30 <= command_id <= 0x36:  # ASCII '0'到'6'的范围
                            command_id = command_id - 0x30  # 转换为数值0-6
                            print(f"检测到ASCII字符指令，转换后ID: {command_id}")
                        # 打印原始数据和解析结果
                        print(f"收到二进制数据: 0x{command_id:02X}, 十进制: {command_id}")

                        # 验证ID范围
                        if 0 <= command_id <= 6:
                            action_name = Id_To_Action.get(command_id)
                            if action_name:
                                print(f"执行动作：{action_name} (ID={command_id})")
                                execute_action(action_name)
                            else:
                                print(f"收到未知指令 ID={command_id}")
                        else:
                            print(f"无效指令ID: {command_id} (有效范围: 0-6)")

                except Exception as e:
                    print(f"解析数据时发生错误: {e}")

        except Exception as e:
            print(f"发生错误: {e}")
            traceback.print_exc()
            time.sleep(RETRY_DELAY)
def initialize_robot_arm():
    """机械臂初始化函数，启动时执行待机动作"""
    print("正在初始化机械臂...")
    if execute_action("待机"):
        print("机械臂初始化成功，已进入待机状态")
    else:
        print("机械臂初始化失败")


def main():
    try:
        # 初始化串口
        if not initialize_serial():
            print("程序因串口初始化失败而退出")
            return

        # 机械臂初始化，执行待机动作
        initialize_robot_arm()
        # 创建并启动串口监听线程（确保ser已初始化）
        serial_thread = threading.Thread(target=receive_serial_commands, daemon=True)
        serial_thread.start()

        print("机械臂控制程序已启动，等待指令...")

        # 保持主线程运行
        while True:
            time.sleep(1)

    except KeyboardInterrupt:
        print("\n程序已停止")
    finally:
        # 清理资源
        try:
            if ser is not None and ser.is_open:
                ser.close()
                print("串口已关闭")
        except:
            print("关闭串口时发生错误")


if __name__ == "__main__":
    main()
# import sys
# sys.path.append('/app/Project_Demo')
# from Uart_Send_Project.Uart_Moudel import Uart_Send  # 导入左右机械臂函数
# import time
#
# # 定义动作字典（包含机械臂类型和参数）
# Action_Dict = {
#     "待机":   {"id": 0,  "arms": ["left", "right"], "count": 1},
#     "挥手":   {"id": 1,  "arms": ["right"], "count": 3},
#     "拥抱":   {"id": 2,  "arms": ["left", "right"], "count": 1},
#     "敬礼":   {"id": 3,  "arms": ["left"], "count": 2},
#     "举手":   {"id": 4,  "arms": ["left", "right"], "count": 1},
#     "握左手":   {"id": 5,  "arms": ["left"], "count": 1},
#     "握右手":   {"id": 6,  "arms": ["right"], "count": 1}
# }
#
# def execute_action(action_name, execute_count):
#     """执行动作（支持单臂/双臂协同）"""
#     action = Action_Dict.get(action_name)
#     if not action:
#         print(f"错误：未定义动作 '{action_name}'")
#         return False
#
#     # 解析动作参数
#     group_id = action["id"]
#     arms = action["arms"]
#     count = execute_count if execute_count else action["count"]  # 优先使用用户输入次数
#
#     # 调用对应机械臂函数
#     success = True
#     for arm in arms:
#         if arm == "right":
#             ok, _ = Uart_Send.send_robot_command_Right(group_id, count)
#         elif arm == "left":
#             ok, _ = Uart_Send.send_robot_command_Left(group_id, count)
#         else:
#             print(f"错误：未知机械臂类型 '{arm}'")
#             ok = False
#         success = success and ok  # 所有机械臂调用成功才返回True
#
#     return success
#
# def action_judge():
#     while True:
#         action_name = input("\n请输入动作名称（待机/挥手/拥抱/敬礼/举手）: ")
#         execute_count = int(input("请输入执行次数（默认值: 1）: ") or 1)  # 允许直接回车使用默认值
#
#         if execute_action(action_name, execute_count):
#             print("动作已执行")
#         else:
#             print("执行失败，请检查参数")
#
#         # 询问是否继续
#         cont = input("是否继续执行动作？(y/n): ").lower()
#         if cont != 'y':
#             break
#     time.sleep(3)
#     # 最终回到待机状态（双机械臂同步）
#     print("\n执行待机动作")
#     execute_action("待机", 1)
#
# if __name__ == '__main__':
#     action_judge()