from openai import OpenAI
from test import XunfeiSpeechRecognizer
from speech import record_and_save
from tts3 import tts_convert
import serial
import time
import traceback

# 配置串口参数
SERIAL_PORT = "ttyTHS1"  # 根据实际情况修改
BAUD_RATE = 115200
ser = None  # 全局串口对象

# 机械臂动作配置
Action_Config = {
    "待机": {"id": 0, "trigger_words": ["待机", "休息", "停下", "别动"]},
    "挥手": {"id": 1, "trigger_words": ["挥手", "摆手", "你好", "嗨"]},
    "拥抱": {"id": 2, "trigger_words": ["拥抱", "抱抱", "抱一下"]},
    "敬礼": {"id": 3, "trigger_words": ["敬礼", "行礼", "军礼"]},
    "举手": {"id": 4, "trigger_words": ["举手", "抬起手", "举高"]},
    "握左手": {"id": 5, "trigger_words": ["左手", "左握", "左握手", "握左手"]},
    "握右手": {"id": 6, "trigger_words": ["右握手", "握手", "右手", "右握", "握右手"]}
}

# 创建关键词到ID的映射表
Keyword_To_Id = {}
for action in Action_Config.values():
    for word in action["trigger_words"]:
        Keyword_To_Id[word] = action["id"]


# 初始化OpenAI客户端
def init_openai_client():
    try:
        client = OpenAI(
            api_key="填写你的API",
            base_url="https://api.deepseek.com"
        )
        # 测试连接
        models = client.models.list()
        print("OpenAI API 连接成功")
        return client
    except Exception as e:
        print(f"OpenAI 初始化失败: {e}")
        return None


# 初始化语音识别
def init_speech_recognizer():
    try:
        recognizer = XunfeiSpeechRecognizer(
            APPID="4c8660b2",
            APIKey="b9883ee83a51f75ebd16cd85212de3fa",
            APISecret="MTdhMGU3Mzg3MDYxNDYyZjdkYjA1MTAw",
            AudioFile="./output.wav",
        )
        print("语音识别模块初始化成功")
        return recognizer
    except Exception as e:
        print(f"语音识别初始化失败: {e}")
        return None


# 初始化串口
def init_serial():
    global ser
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"串口已初始化: {SERIAL_PORT}, 波特率: {BAUD_RATE}")
        # 发送初始化指令
        send_action_id(0)  # 发送待机指令
        return True
    except Exception as e:
        print(f"串口初始化失败: {e}")
        return False


# 发送动作ID（修改为发送字符串格式）
def send_action_id(action_id):
    try:
        # 将整数ID转换为字符串并添加换行符
        command = f"{action_id}\n"
        ser.write(command.encode('utf-8'))
        print(f"已发送动作ID: {action_id} (字符串格式: {command.strip()})")

        # 等待响应
        time.sleep(0.1)
        response = ser.read(ser.in_waiting).decode('utf-8').strip()
        if response:
            print(f"收到下位机响应: {response}")
            return True
        else:
            print("未收到下位机响应")
            return False
    except Exception as e:
        print(f"发送动作ID失败: {e}")
        traceback.print_exc()
        return False


# 检测文本中的动作关键词
def detect_action_keywords(text):
    if not text:
        return None

    for keyword, action_id in Keyword_To_Id.items():
        if keyword in text:
            print(f"检测到动作关键词: {keyword}, 发送动作ID: {action_id}")
            return action_id
    return None


# 测试语音识别功能
def test_speech_recognition(recognizer):
    print("开始测试语音识别...")
    try:
        record_and_save(duration=3, filename="test.wav")
        result = recognizer.recognize()
        print(f"测试识别结果: {result}")
        return result
    except Exception as e:
        print(f"语音识别测试失败: {e}")
        return None


# 主程序
def main():
    print("=== 启动情感陪伴AI机器人小智 ===")

    # 初始化各组件
    client = init_openai_client()
    if not client:
        print("OpenAI客户端初始化失败，程序退出")
        return

    recognizer = init_speech_recognizer()
    if not recognizer:
        print("语音识别模块初始化失败，程序退出")
        return

    if not init_serial():
        print("串口初始化失败，程序退出")
        return

    # 测试语音识别
    test_result = test_speech_recognition(recognizer)
    if not test_result:
        print("语音识别测试未通过，建议检查麦克风和语音识别配置")

    # 初始化对话
    messages = []
    assistant_role = "你是一个专注于情感陪伴的AI机器人小智，每个提问者都是你的主人，请你一直保持这个身份，请幽默的回答问题,不要输出表情字体"
    if assistant_role:
        messages.append({
            "role": "system",
            "content": f"你是一个{assistant_role}，在整个对话中都保持这个身份。",
        })

    print("\n=== 系统初始化完成，开始对话 ===")
    print("提示: 说'退出'结束对话")

    try:
        while True:
            print("\n请说话...")
            record_and_save(duration=5, filename="output.wav")
            result = recognizer.recognize()
            print(f"识别结果: {result}")

            user_input = result
            if not user_input:
                print("未识别到语音，请重试")
                continue

            if user_input.lower() == "退出":
                break

            # 添加用户消息
            messages.append({"role": "user", "content": user_input})

            try:
                # 获取AI回复
                response = client.chat.completions.create(
                    model="deepseek-chat",
                    messages=messages
                )
                assistant_response = response.choices[0].message
                messages.append(assistant_response)

                print(f"Assistant: {assistant_response.content}")

                # 检测关键词并发送动作ID
                action_id = detect_action_keywords(assistant_response.content)
                if action_id:
                    send_action_id(action_id)
                else:
                    print("未检测到动作关键词")

                # 文本转语音
                text = assistant_response.content
                file_name, duration = tts_convert(text)
                print(f"音频文件保存为: {file_name}")
                if duration:
                    print(f"音频播放完毕，时长为: {duration} 秒")

            except Exception as e:
                print(f"处理用户输入时发生错误: {e}")
                # 保留对话历史，继续循环
                continue

    except KeyboardInterrupt:
        print("\n程序被用户中断")
    finally:
        # 清理资源
        if ser and ser.is_open:
            ser.close()
            print("串口已关闭")
        print("=== 程序已退出 ===")


if __name__ == "__main__":
    main()