#!/usr/bin/env python3
import serial
import time


def send_robot_command_Right(group_id, count, port='/dev/ttyS1', baudrate=9600):
    """
    向机械臂发送执行指令

    参数:
        group_id (int): 动作组编号 (0-230)
        count (int): 执行次数 (1-65535)
        port (str): 串口设备名，默认为 '/dev/ttyS1'
        baudrate (int): 波特率，默认为 9600

    返回:
        bool: 发送成功返回 True，失败返回 False
        bytes: 机械臂的响应数据，如果没有则返回空字节
    """
    # 参数验证
    if not (0 <= group_id <= 230):
        print(f"错误: 动作组编号超出范围 (0-230)，当前值: {group_id}")
        return False, b''

    if not (1 <= count <= 65535):
        print(f"错误: 执行次数超出范围 (1-65535)，当前值: {count}")
        return False, b''

    # 打开串口
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
    except Exception as e:
        print(f"串口1连接失败: {e}")
        return False, b''

    try:
        # 构建指令
        cmd = bytearray([
            0x55, 0x55,  # 帧头
            0x05, 0x06,  # 指令码
            group_id,  # 动作组编号
            count & 0xFF,  # 执行次数低八位
            (count >> 8) & 0xFF  # 执行次数高八位
        ])

        # 发送指令
        ser.write(cmd)
        print(f"已发送指令: {['0x{:02X}'.format(b) for b in cmd]}")

        # 等待响应
        time.sleep(0.1)  # 等待机械臂处理
        response = ser.read_all()

        if response:
            print(f"收到响应: {['0x{:02X}'.format(b) for b in response]}")
            return True, response
        else:
            print("未收到响应")
            return True, b''

    except Exception as e:
        print(f"指令发送失败: {e}")
        return False, b''
    finally:
        ser.close()  # 确保串口被关闭

def send_robot_command_Left(group_id, count, port='/dev/ttyS2', baudrate=9600):
    """
    向机械臂发送执行指令

    参数:
        group_id (int): 动作组编号 (0-230)
        count (int): 执行次数 (1-65535)
        port (str): 串口设备名，默认为 '/dev/ttyS2'
        baudrate (int): 波特率，默认为 9600

    返回:
        bool: 发送成功返回 True，失败返回 False
        bytes: 机械臂的响应数据，如果没有则返回空字节
    """
    # 参数验证
    if not (0 <= group_id <= 230):
        print(f"错误: 动作组编号超出范围 (0-230)，当前值: {group_id}")
        return False, b''

    if not (1 <= count <= 65535):
        print(f"错误: 执行次数超出范围 (1-65535)，当前值: {count}")
        return False, b''

    # 打开串口
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
    except Exception as e:
        print(f"串口连接2失败: {e}")
        return False, b''

    try:
        # 构建指令
        cmd = bytearray([
            0x55, 0x55,  # 帧头
            0x05, 0x06,  # 指令码
            group_id,  # 动作组编号
            count & 0xFF,  # 执行次数低八位
            (count >> 8) & 0xFF  # 执行次数高八位
        ])

        # 发送指令
        ser.write(cmd)
        print(f"已发送指令: {['0x{:02X}'.format(b) for b in cmd]}")

        # 等待响应
        time.sleep(0.1)  # 等待机械臂处理
        response = ser.read_all()

        if response:
            print(f"收到响应: {['0x{:02X}'.format(b) for b in response]}")
            return True, response
        else:
            print("未收到响应")
            return True, b''

    except Exception as e:
        print(f"指令发送失败: {e}")
        return False, b''
    finally:
        ser.close()  # 确保串口被关闭