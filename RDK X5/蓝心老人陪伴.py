import pandas as pd
from typing import Any, Dict, List, Optional, Union, Literal
import uuid
import requests
import json
import time
import hashlib
import hmac
import base64
import random
import string
import urllib.parse
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel, Extra
from bs4 import BeautifulSoup
import re

# 确保Python版本兼容
if tuple(map(int, re.findall(r'\d+', requests.__version__))) < (2, 25, 1):
    raise RuntimeError("requests库版本需不低于2.25.1")

app = FastAPI(
    title="蓝心大模型老人陪伴工具集成服务",
    description="基于BlueLM-TB-Pro的老人陪伴工具，提供老年关怀建议、健康提醒和陪伴资源"
)

# 读取老人关怀资源文件
try:
    excel_file = pd.ExcelFile('E:/老人关怀资源库.xlsx')
    # 获取指定工作表中的数据
    df = excel_file.parse('老年活动与服务')
    print("老人关怀资源文件加载成功")
except Exception as e:
    print(f"老人关怀资源文件加载失败: {e}")
    df = pd.DataFrame(columns=['服务名称', '服务链接', '相关图片'])  # 确保有必要列名的空DataFrame


# 定义统一返回格式
class ApiResult:
    @staticmethod
    def success(function_name: str, result: Dict[str, Any]) -> Dict[str, Any]:
        return {
            "success": True,
            "function_name": function_name,
            "result": result
        }

    @staticmethod
    def error(error_code: str, error_message: str) -> Dict[str, Any]:
        return {
            "success": False,
            "error_code": error_code,
            "error_message": error_message
        }


class ElderlyServiceSearcher:
    """老年服务专属搜索工具，搜索适合老年人的服务与活动"""

    def __init__(self, timeout: int = 15, use_proxy: bool = False):
        self.base_url = "https://www.laonianwang.com"  # 老年网作为示例
        self.search_path = "/search"
        self.timeout = timeout
        self.use_proxy = use_proxy

        # 模拟浏览器的User-Agent
        self.user_agents = [
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:109.0) Gecko/20100101 Firefox/115.0",
            "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/16.5 Safari/605.1.15"
        ]

        # 基础请求头
        self.base_headers = {
            "Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8",
            "Accept-Language": "zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2",
            "Connection": "keep-alive",
            "Referer": f"{self.base_url}/",
            "Upgrade-Insecure-Requests": "1"
        }

        # 代理配置（可选）
        self.proxies = {
            'http': 'http://127.0.0.1:8080',
            'https': 'http://127.0.0.1:8080'
        } if use_proxy else None

        # 会话保持
        self.session = requests.Session()
        try:
            self.session.get(self.base_url, timeout=5, headers=self._get_headers())
        except:
            pass

    def _get_headers(self) -> Dict[str, str]:
        """生成模拟浏览器的请求头"""
        headers = self.base_headers.copy()
        headers["User-Agent"] = random.choice(self.user_agents)
        headers["Host"] = "www.laonianwang.com"
        return headers

    def _build_search_url(self, keyword: str) -> str:
        """构建搜索URL"""
        encoded_keyword = urllib.parse.quote(keyword)
        return f"{self.base_url}{self.search_path}?q={encoded_keyword}"

    def _parse_first_service(self, soup: BeautifulSoup, search_url: str) -> Optional[Dict[str, str]]:
        """从搜索结果页解析第一个服务项目"""
        service_items = soup.select("div.service-item")
        if not service_items:
            service_items = soup.select("div.activity-item")

        if not service_items:
            print("[解析失败] 未找到服务列表")
            return None

        first_item = service_items[0]

        # 提取服务名称
        name_tag = first_item.select_one("div.service-name a") or first_item.select_one("div.activity-title a")
        if not name_tag:
            print("[解析失败] 未找到服务名称")
            return None
        service_name = name_tag.get_text(strip=True)

        # 提取服务链接
        link_tag = first_item.select_one("div.service-link a") or first_item.select_one("div.service-name a")
        if not link_tag:
            print("[解析失败] 未找到服务链接")
            return None
        service_url = link_tag.get("href", "")
        if not service_url:
            print("[解析失败] 服务链接为空")
            return None
        full_url = urllib.parse.urljoin(search_url, service_url)

        # 提取服务图片
        image_tag = first_item.select_one("div.service-img img")
        image_url = ""
        if image_tag:
            image_url = image_tag.get("src") or image_tag.get("data-src") or image_tag.get("data-original")
            image_url = urllib.parse.urljoin(search_url, image_url) if image_url else ""

        return {
            "image_url": image_url,
            "service_url": full_url,
            "source": "老年网(laonianwang.com)",
            "service_name": service_name,
            "success": True
        }

    def search_first_service(self, keyword: str) -> Dict[str, str]:
        """搜索关键词并返回第一个服务项目"""
        print(f"[搜索] 关键词: {keyword}")
        search_url = self._build_search_url(keyword)
        print(f"[访问] 搜索页面: {search_url}")

        for attempt in range(3):
            try:
                time.sleep(random.uniform(1, 2))
                response = self.session.get(
                    search_url,
                    headers=self._get_headers(),
                    timeout=self.timeout,
                    proxies=self.proxies
                )
                response.raise_for_status()

                if "搜索结果" not in response.text:
                    print("[异常] 未找到搜索结果标识")
                    if any(word in response.text for word in ["登录", "注册", "login"]):
                        print("[异常] 触发登录验证")
                        return {
                            "success": False,
                            "error_message": "需要登录才能访问",
                            "search_url": search_url
                        }
                    continue

                soup = BeautifulSoup(response.text, "html.parser")
                result = self._parse_first_service(soup, search_url)
                if result:
                    print(f"[成功] 找到第一个服务: {result['service_name']}")
                    return result
                else:
                    print(f"[尝试{attempt + 1}] 无法解析第一个服务，重试中...")
                    time.sleep(2)

            except requests.exceptions.HTTPError as e:
                print(f"[HTTP错误] 状态码: {response.status_code}, 错误: {e}")
                if response.status_code == 403:
                    print("[警告] 被服务器拒绝，可能触发反爬机制")
                    return {
                        "success": False,
                        "error_message": "被服务器拒绝，请稍后重试",
                        "search_url": search_url
                    }
                time.sleep(3)
            except requests.exceptions.Timeout:
                print("[超时] 搜索请求超时")
                time.sleep(3)
            except Exception as e:
                print(f"[错误] {str(e)}")
                time.sleep(3)

        print("[失败] 所有尝试均失败")
        return {
            "success": False,
            "error_message": "未找到有效服务",
            "search_url": search_url,
            "image_url": "https://picsum.photos/400/300?text=无图片",
            "service_url": self.base_url,
            "source": "老年网(laonianwang.com)",
            "service_name": keyword
        }


class LanXinClient:
    """蓝心大模型API客户端"""

    def __init__(
            self,
            app_id: str,
            app_key: str,
            api_base: str = "https://api-ai.vivo.com.cn/vivogpt/completions",
            model_name: str = "vivo-BlueLM-TB-Pro",
            timeout: int = 60,
            debug: bool = False
    ):
        self.app_id = app_id
        self.app_key = app_key
        self.api_base = api_base
        self.model_name = model_name
        self.timeout = timeout
        self.debug = debug
        self.service_searcher = ElderlyServiceSearcher(timeout=15)

    def _generate_nonce(self) -> str:
        """生成随机数"""
        chars = string.ascii_lowercase + string.digits
        return ''.join(random.choice(chars) for _ in range(8))

    def _gen_canonical_query_string(self, params: Dict[str, Any]) -> str:
        """生成规范化查询字符串"""
        if not params:
            return ""
        sorted_items = sorted(params.items(), key=lambda x: x[0])
        encoded_items = [
            f"{urllib.parse.quote(str(k))}={urllib.parse.quote(str(v) if v is not None else '')}"
            for k, v in sorted_items
        ]
        return "&".join(encoded_items)

    def _gen_signing_string(
            self,
            method: str,
            uri: str,
            query_params: Dict[str, Any],
            timestamp: str,
            nonce: str
    ) -> bytes:
        """生成签名字符串"""
        canonical_query = self._gen_canonical_query_string(query_params)
        signed_headers = (
            f"x-ai-gateway-app-id:{self.app_id}\n"
            f"x-ai-gateway-timestamp:{timestamp}\n"
            f"x-ai-gateway-nonce:{nonce}"
        )
        return (
            f"{method}\n{uri}\n{canonical_query}\n{self.app_id}\n{timestamp}\n{signed_headers}"
        ).encode('utf-8')

    def _gen_signature(self, signing_string: bytes) -> str:
        """生成签名"""
        hmac_obj = hmac.new(
            key=self.app_key.encode('utf-8'),
            msg=signing_string,
            digestmod=hashlib.sha256
        )
        return base64.b64encode(hmac_obj.digest()).decode('utf-8')

    def _gen_auth_headers(self, method: str, uri: str, query_params: Dict[str, Any]) -> Dict[str, str]:
        """生成认证请求头"""
        timestamp = str(int(time.time()))
        nonce = self._generate_nonce()
        signing_string = self._gen_signing_string(
            method=method,
            uri=uri,
            query_params=query_params,
            timestamp=timestamp,
            nonce=nonce
        )
        signature = self._gen_signature(signing_string)
        return {
            "X-AI-GATEWAY-APP-ID": self.app_id,
            "X-AI-GATEWAY-TIMESTAMP": timestamp,
            "X-AI-GATEWAY-NONCE": nonce,
            "X-AI-GATEWAY-SIGNED-HEADERS": "x-ai-gateway-app-id;x-ai-gateway-timestamp;x-ai-gateway-nonce",
            "X-AI-GATEWAY-SIGNATURE": signature,
            "Content-Type": "application/json; charset=utf-8"
        }

    def generate(self, **kwargs) -> Union[str, Dict[str, Any]]:
        """通用生成函数"""
        request_id = str(uuid.uuid4())
        session_id = str(uuid.uuid4())
        parsed_url = urllib.parse.urlparse(self.api_base)
        http_method = "POST"
        uri = parsed_url.path
        query_params = dict(urllib.parse.parse_qsl(parsed_url.query))
        query_params["requestId"] = request_id

        headers = self._gen_auth_headers(
            method=http_method,
            uri=uri,
            query_params=query_params
        )

        messages = kwargs.get("messages")
        prompt = kwargs.get("prompt")
        if messages is None:
            if not prompt:
                return ApiResult.error("INVALID_PARAMS", "prompt和messages不能同时为空")
            messages = [{"role": "user", "content": prompt}]

        payload = {
            "model": self.model_name,
            "messages": messages,
            "temperature": kwargs.get("temperature", 0.7),
            "max_tokens": kwargs.get("max_tokens", 1024),
            "requestId": request_id,
            "sessionId": session_id
        }
        if kwargs.get("stop"):
            payload["stop"] = kwargs["stop"]
        payload.update({k: v for k, v in kwargs.items() if k not in payload})

        if self.debug:
            print(f"[调试] 生成参数: {payload}")

        try:
            encoded_params = urllib.parse.urlencode(query_params, safe=':/')
            full_url = f"{self.api_base}?{encoded_params}"
            payload_json = json.dumps(payload, ensure_ascii=False)
            response = requests.post(
                url=full_url,
                headers=headers,
                data=payload_json.encode('utf-8'),
                timeout=self.timeout
            )
            response.encoding = 'utf-8'
            response_data = response.json()

            if self.debug:
                print(f"[调试] 响应数据: {response_data}")

            if response_data.get("code") == 0:
                return ApiResult.success(
                    "generate_content",
                    {"text": response_data["data"]["content"]}  # 确保返回字典格式
                )
            else:
                return ApiResult.error(
                    response_data.get("code"),
                    response_data.get("msg", "未知错误")
                )
        except Exception as e:
            return ApiResult.error(
                "INTERNAL_ERROR",
                f"生成失败: {str(e)}"
            )

    def call_function(self, function_name: str, **kwargs) -> Union[str, Dict[str, Any]]:
        """调用特定功能"""
        function_map = {
            "generate_content": self.generate,
            "elderly_care_advice": self.elderly_care_advice,
            "companionship_topics": self.companionship_topics,
            "health_reminders": self.health_reminders,
            "elderly_activity_recommendation": self.elderly_activity_recommendation,
            "elderly_service_links": self.elderly_service_links
        }

        if function_name not in function_map:
            return ApiResult.error(
                "INVALID_FUNCTION",
                f"不支持的函数: {function_name}，支持的函数: {list(function_map.keys())}"
            )

        return function_map[function_name](**kwargs)

    def elderly_care_advice(self, question: str, age: int = None, health_condition: str = None, **kwargs) -> Dict[
        str, Any]:
        """老人关怀建议"""
        if not question:
            return ApiResult.error("INVALID_PARAMS", "查询问题不能为空")

        additional_info = ""
        if age:
            additional_info += f"年龄: {age}岁 "
        if health_condition:
            additional_info += f"健康状况: {health_condition}"

        prompt = f"作为老人关怀专家，请提供专业的老人照顾建议：{question} {additional_info}。回答应包含具体、可操作的建议，语言通俗易懂，适合老年人理解和家人实施。"
        return self.generate(
            prompt=prompt,
            temperature=kwargs.get("temperature", 0.3),
            max_tokens=kwargs.get("max_tokens", 2048)
        )

    def companionship_topics(self, interests: List[str] = None, age: int = None, **kwargs) -> Dict[str, Any]:
        """陪伴话题推荐"""
        interests_str = ", ".join(interests) if interests else "未指定"
        age_str = f"{age}岁" if age else "未指定"

        prompt = f"请为{age_str}的老人推荐适合的陪伴话题，已知兴趣爱好：{interests_str}。推荐10个话题，每个话题简要说明适合的原因和聊天要点，话题应积极健康，能唤起美好回忆或带来愉悦感。"
        return self.generate(
            prompt=prompt,
            temperature=kwargs.get("temperature", 0.7),
            max_tokens=kwargs.get("max_tokens", 1500)
        )

    def health_reminders(self, condition: str, time_of_day: str = None, season: str = None, **kwargs) -> Dict[str, Any]:
        """健康提醒生成"""
        if not condition:
            return ApiResult.error("INVALID_PARAMS", "健康状况不能为空")

        time_info = ""
        if time_of_day:
            time_info += f"时间段: {time_of_day} "
        if season:
            time_info += f"季节: {season}"

        prompt = f"为有{condition}的老人生成健康提醒：{time_info}。提醒内容应具体实用，包括饮食建议、活动提醒、注意事项等，语言亲切温和，易于老年人理解和遵循。"
        return self.generate(
            prompt=prompt,
            temperature=kwargs.get("temperature", 0.4),
            max_tokens=kwargs.get("max_tokens", 1024)
        )

    def elderly_activity_recommendation(self, activity_type: str, age: int, physical_condition: str,
                                        location: str = None, **kwargs) -> Dict[str, Any]:
        """老年人活动推荐"""
        if not all([activity_type, physical_condition]):
            return ApiResult.error("INVALID_PARAMS", "活动类型、身体状况为必填参数")

        prompt = f"""请为老年人推荐合适的活动：
1. 基本信息：
   - 希望活动类型：{activity_type}
   - 年龄：{age}岁
   - 身体状况：{physical_condition}
   - 所在地区：{location or '未指定'}
2. 推荐要求：
   - 推荐3-4种适合的活动
   - 每种活动说明适合原因、益处和注意事项
   - 考虑安全性和可行性
3. 输出格式：严格JSON结构，包含：
   - "推荐活动列表"：活动数组（含活动名称、适合原因、益处、注意事项、建议频率）
   - "实施建议"：总体注意事项
"""

        result = self.generate(
            prompt=prompt,
            temperature=kwargs.get("temperature", 0.2),
            max_tokens=kwargs.get("max_tokens", 4096)
        )

        if result.get("success"):
            try:
                if isinstance(result["result"]["text"], str):
                    activity_data = json.loads(result["result"]["text"])
                else:
                    activity_data = result["result"]["text"]

                # 为推荐活动补充相关服务链接和图片
                if "推荐活动列表" in activity_data:
                    for activity in activity_data["推荐活动列表"]:
                        activity_name = activity.get("活动名称", "")
                        if activity_name and not df.empty:
                            activity_info = df[df['服务名称'].str.contains(activity_name, na=False)].head(1)
                            if not activity_info.empty:
                                activity.update({
                                    "image_url": activity_info.iloc[0]['相关图片'],
                                    "service_url": activity_info.iloc[0]['服务链接'],
                                    "source": "老年服务资源库",
                                    "full_name": activity_info.iloc[0]['服务名称']
                                })
                            else:
                                service_info = self.service_searcher.search_first_service(activity_name)
                                activity.update({
                                    "image_url": service_info.get("image_url"),
                                    "service_url": service_info.get("service_url"),
                                    "source": service_info.get("source"),
                                    "full_name": service_info.get("service_name")
                                })
                        else:
                            activity.update({
                                "image_url": "https://picsum.photos/400/300?text=无图片",
                                "service_url": self.service_searcher.base_url,
                                "source": "老年网(laonianwang.com)",
                                "full_name": activity_name or "未知活动"
                            })

                result["result"] = activity_data

            except Exception as e:
                return ApiResult.error(
                    "PARSING_ERROR",
                    f"解析失败: {str(e)}"
                )

        return result

    def elderly_service_links(self, demand: str, **kwargs) -> Dict[str, Any]:
        """老年人服务链接推荐"""
        if not demand:
            return ApiResult.error("INVALID_PARAMS", "需求描述不能为空")

        prompt = f"""根据需求推荐老年人相关服务，并仅返回核心信息：
1. 需求：{demand}
2. 输出要求：
   - 推荐3-4种相关服务（仅需服务名称）
   - 严格使用JSON格式：
     {{
       "recommended_services": [
         {{ "name": "服务名称" }}
       ]
     }}
   - 不要多余文字，不要包含链接
"""

        result = self.generate(
            prompt=prompt,
            temperature=kwargs.get("temperature", 0.1),
            max_tokens=kwargs.get("max_tokens", 1024)
        )

        if result.get("success"):
            try:
                if isinstance(result["result"]["text"], str):
                    service_data = json.loads(result["result"]["text"])
                else:
                    service_data = result["result"]["text"]

                # 补充服务链接和图片
                if "recommended_services" in service_data:
                    for service in service_data["recommended_services"]:
                        service_name = service.get("name", "")
                        if service_name and not df.empty:
                            service_info = df[df['服务名称'].str.contains(service_name, na=False)].head(1)
                            if not service_info.empty:
                                service.update({
                                    "image_url": service_info.iloc[0]['相关图片'],
                                    "service_url": service_info.iloc[0]['服务链接'],
                                    "source": "老年服务资源库",
                                    "full_name": service_info.iloc[0]['服务名称']
                                })
                            else:
                                links = self.service_searcher.search_first_service(service_name)
                                service.update({
                                    "image_url": links.get("image_url"),
                                    "service_url": links.get("service_url"),
                                    "source": links.get("source"),
                                    "full_name": links.get("service_name")
                                })
                        else:
                            service.update({
                                "image_url": "https://picsum.photos/400/300?text=无图片",
                                "service_url": self.service_searcher.base_url,
                                "source": "老年网(laonianwang.com)",
                                "full_name": service_name or "未知服务"
                            })

                result["result"] = service_data

            except Exception as e:
                return ApiResult.error(
                    "PARSING_ERROR",
                    f"解析失败: {str(e)}"
                )

        return result


# 通用请求模型
class FunctionCallRequest(BaseModel):
    function_name: str
    temperature: float = 0.7
    max_tokens: int = 1024

    class Config:
        extra = Extra.allow


# 初始化客户端
client = LanXinClient(
    app_id="*************",  # 替换为实际app_id
    app_key="******************",  # 替换为实际app_key
    debug=True
)


# 统一API端点
@app.post("/function/call", summary="通用功能调用接口")
async def handle_function_call(request: FunctionCallRequest):
    """
    统一功能调用接口，通过function_name参数区分不同功能
    支持的function_name:
    - generate_content: 生成内容
    - elderly_care_advice: 老人关怀建议
    - companionship_topics: 陪伴话题推荐
    - health_reminders: 健康提醒
    - elderly_activity_recommendation: 老年人活动推荐
    - elderly_service_links: 老年人服务链接推荐
    """
    try:
        func_params = request.dict(exclude={"function_name"})
        result = client.call_function(
            function_name=request.function_name, **func_params
        )
        return result
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"服务错误: {str(e)}")


# 直接搜索接口
@app.post("/search/elderly_services", summary="直接搜索老年人服务")
async def search_elderly_services(keyword: str):
    """直接搜索老年人相关服务并返回第一个结果"""
    if not keyword.strip():
        raise HTTPException(status_code=400, detail="搜索关键词不能为空")

    result = client.service_searcher.search_first_service(keyword.strip())
    return ApiResult.success(
        "search_elderly_services",
        {
            "service_name": result.get("service_name", keyword),
            "image_url": result.get("image_url", "https://picsum.photos/400/300?text=无图片"),
            "service_url": result.get("service_url", "https://www.laonianwang.com"),
            "source": result.get("source", "老年网(laonianwang.com)"),
            "success": result.get("success", False)
        }
    )


# 启动服务
if __name__ == "__main__":
    try:
        import uvicorn

        print("首次运行请安装依赖: pip install fastapi uvicorn requests beautifulsoup4 pandas openpyxl")
        print("服务启动中...访问 http://localhost:8000/docs 查看所有接口")
        print("所有服务链接均来自老年服务资源库")
        uvicorn.run(app, host="0.0.0.0", port=8000)
    except ImportError as e:
        print(f"依赖缺失: {e}\n请执行: pip install fastapi uvicorn requests beautifulsoup4 pandas openpyxl")
    except Exception as e:
        print(f"启动失败: {e}")