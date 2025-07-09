# encoding: utf-8
import uuid
import time
import requests
from auth_util import gen_sign_headers

# 请替换APP_ID、APP_KEY
APP_ID = '************'
APP_KEY = 'KiMANYdWbGDdFOyU'
URI = '/vivogpt/completions'
DOMAIN = 'api-ai.vivo.com.cn'
METHOD = 'POST'


def sync_vivogpt():
    session_id = str(uuid.uuid4())
    messages = []
    
    while True:
        user_input = input("请输入您的问题(输入q退出): ").strip()
        if user_input.lower() == 'q':
            break
            
        messages.append({"role": "user", "content": user_input})
        
        params = {
            'requestId': str(uuid.uuid4())
        }
        print('requestId:', params['requestId'])

        data = {
            'messages': messages,
            'model': 'vivo-BlueLM-TB-Pro',
            'sessionId': session_id,
            'extra': {
                'temperature': 0.9
            }
        }
        headers = gen_sign_headers(APP_ID, APP_KEY, METHOD, URI, params)
        headers['Content-Type'] = 'application/json'

        start_time = time.time()
        url = 'https://{}{}'.format(DOMAIN, URI)
        response = requests.post(url, json=data, headers=headers, params=params)

        if response.status_code == 200:
            res_obj = response.json()
            print(f'response:{str(res_obj).encode("gbk", errors="replace").decode("gbk")}')
            if res_obj['code'] == 0 and res_obj.get('data'):
                content = res_obj['data']['content']
                print(f'AI回复:\n{content.encode("gbk", errors="replace").decode("gbk")}')
                messages.append({"role": "assistant", "content": content})
        else:
            print(str(response.status_code).encode("gbk", errors="replace").decode("gbk"), 
                  response.text.encode("gbk", errors="replace").decode("gbk"))
        end_time = time.time()
        timecost = end_time - start_time
        print('请求耗时: %.2f秒' % timecost)


if __name__ == '__main__':
    try:
        sync_vivogpt()
    except KeyboardInterrupt:
        print("\n对话已结束")