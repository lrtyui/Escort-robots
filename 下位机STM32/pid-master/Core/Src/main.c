
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../../BSP/LED/led.h"
//#include "../../BSP/KEY/key.h"
#include "../../BSP/LCD/lcd.h"
#include "../../SYSTEM/delay/delay.h"
#include "../../BSP/REMOTE/remote.h"
#include "./mpu6050.h"
#include "string.h"
#include "stdio.h"
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
//#define user_main_printf(format, ...)  printf( format "\r\n", ##__VA_ARGS__)
int16_t Get_Left_Motor_Speed(int16_t left_speed_now);
int16_t Get_Right_Motor_Speed(int16_t left_speed_now);
//typedef struct
//{
//  float Kp;                       //锟斤拷锟斤拷系锟斤拷Proportional
//  float Ki;                       //锟斤拷锟斤拷系锟斤拷Integral
//  float Kd;                       //微锟斤拷系锟斤拷Derivative
//
//  float Ek;                       //锟斤拷前 锟斤拷锟�???
//  float Ek1;                      //前一锟斤拷锟斤拷锟� e(k-1)
//  float Ek2;                      //锟斤拷前�???锟斤拷锟斤拷锟� e(k-2)
//  float LocSum;                   //锟桔计伙拷锟斤拷位锟斤拷
//}PID_LocTypeDef;

typedef struct
{
    float Kp;   // 比例系数
    float Ki;   // 积分系数
    float Kd;   // 微分系数

    float Ek;   // 当前误差 e(k)
    float Ek1;  // 前一次误差 e(k-1)
    float Ek2;  // 前两次误差 e(k-2)
    float LastOutput; // 上次输出(用于增量累加)
} PID_IncTypeDef;

void Send_USART_String(UART_HandleTypeDef *huart, const char *str) {
    HAL_UART_Transmit(huart, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}
//float PID_Loc(float SetValue, float ActualValue, PID_LocTypeDef *PID)
//{
//  float PIDLoc;                                  //位锟斤拷
//	static int16_t err_lowout,err_lowout_last;
//
//  PID->Ek = SetValue - ActualValue;
//	err_lowout = 0.3* PID->Ek+0.7*err_lowout_last;
//	err_lowout_last=err_lowout;
//-

//  PID->LocSum += err_lowout;                         //锟桔硷拷锟斤拷锟�???
//
//  PIDLoc = PID->Kp * PID->Ek + (PID->Ki * PID->LocSum) + PID->Kd * (PID->Ek1 - PID->Ek);
//
//  PID->Ek1 = PID->Ek;  return PIDLoc;
//}
#define IntegralLimit 300.0  // 锟斤拷锟斤拷锟睫凤拷�???
uint8_t len;
uint8_t value1=0;
uint8_t value2;
uint8_t value3;
double number;
char data1[20], data2[20];
//float PID_Loc(float SetValue, float ActualValue, PID_LocTypeDef *PID)
//{
//    float PIDLoc;                                  // PID 锟斤拷锟�???
//    static int16_t err_lowout, err_lowout_last;    // �???锟斤拷锟剿诧拷锟桔伙拷锟斤拷锟街�???
//    static float prev_diff = 0.0;                  // 锟剿诧拷前锟斤拷微锟斤拷锟斤�???
//
//    PID->Ek = SetValue - ActualValue;              // 锟斤拷前锟斤拷锟�???
//
//    // �???锟斤拷锟剿诧拷锟斤拷锟斤拷锟斤拷锟�
//    err_lowout = 0.3 * PID->Ek + 0.7 * err_lowout_last;
//    err_lowout_last = err_lowout;
//
//    // 锟桔伙拷锟斤拷睿拷锟斤拷锟斤拷睿�
//    PID->LocSum += err_lowout;
//
//    // 锟斤拷锟斤拷锟斤拷锟斤拷锟狡ｏ拷锟斤拷止锟斤拷锟街憋拷锟酵ｏ�???
//    if (PID->LocSum > IntegralLimit) {
//        PID->LocSum = IntegralLimit;
//    } else if (PID->LocSum < -IntegralLimit) {
//        PID->LocSum = -IntegralLimit;
//    }
//
//
//    float diff = PID->Ek - PID->Ek1;
//    prev_diff = 0.6 * diff + 0.4 * prev_diff;
//
//    PIDLoc = PID->Kp * PID->Ek + (PID->Ki * PID->LocSum) + PID->Kd * prev_diff;
//
//
//    PID->Ek1 = PID->Ek;
//
//    return PIDLoc;
//}

/* 增量式PID计算函数 */
float PID_Loc(float SetValue, float ActualValue, PID_IncTypeDef *PID)
{
    float deltaU;  // 增量输出
    static int16_t err_lowout, err_lowout_last;

    /* 误差处理（保留原有的低通滤波） */
    PID->Ek = SetValue - ActualValue;
    err_lowout = 0.3 * PID->Ek + 0.7 * err_lowout_last;
    err_lowout_last = err_lowout;

    /* 增量PID计算 */
    deltaU = PID->Kp * (err_lowout - PID->Ek1) +
             PID->Ki * err_lowout +
             PID->Kd * (err_lowout - 2 * PID->Ek1 + PID->Ek2);

    /* 更新误差历史 */
    PID->Ek2 = PID->Ek1;
    PID->Ek1 = err_lowout;

    /* 计算实际输出（增量累加） */
    PID->LastOutput += deltaU;

    /* 输出限幅（根据实际需求调整限幅值） */
    const float OutMax = 5000.0f;
    const float OutMin = -5000.0f;

    if(PID->LastOutput > OutMax) PID->LastOutput = OutMax;
    else if(PID->LastOutput < OutMin) PID->LastOutput = OutMin;

    return PID->LastOutput;
}


float PID_Loc1(float SetValue, float ActualValue, PID_IncTypeDef *PID)
{
    float deltaU;  // 增量输出
    static int16_t err_lowout, err_lowout_last;

    /* 误差处理（保留原有的低通滤波） */
    PID->Ek = SetValue - ActualValue;
    err_lowout = 0.3 * PID->Ek + 0.7 * err_lowout_last;
    err_lowout_last = err_lowout;

    /* 增量PID计算 */
    deltaU = PID->Kp * (err_lowout - PID->Ek1) +
             PID->Ki * err_lowout +
             PID->Kd * (err_lowout - 2 * PID->Ek1 + PID->Ek2);

    /* 更新误差历史 */
    PID->Ek2 = PID->Ek1;
    PID->Ek1 = err_lowout;

    /* 计算实际输出（增量累加） */
    PID->LastOutput += deltaU;

    /* 输出限幅（根据实际需求调整限幅值） */
    const float OutMax = 5000.0f;
    const float OutMin = -5000.0f;

    if(PID->LastOutput > OutMax) PID->LastOutput = OutMax;
    else if(PID->LastOutput < OutMin) PID->LastOutput = OutMin;

    return PID->LastOutput;
}



/**
 * 从字符串中提取 z 和 x 之间的数据，并转换为浮点数
 * @param input 输入字符串
 * @param value 存储提取的浮点数结果
 * @return 0 成功，-1 失败
 */
int extractZXData(const char *input, double *value) {
    // 查找 'z' 的位置
    const char *start = strchr(input, 'z');
    if (start == NULL) return -1;

    // 在 'z' 后查找第一个 'x'
    const char *end = strchr(start + 1, 'x');
    if (end == NULL) return -1;

    // 计算数据长度
    size_t len = end - start - 1;
    if (len <= 0) return -1;

    // 提取字符串数据
    char buffer[64]; // 假设数据不超过63字符
    strncpy(buffer, start + 1, len);
    buffer[len] = '\0';

    // 转换为浮点数
    char *endptr;
    *value = strtod(buffer, &endptr);

    // 检查转换是否完全成功
    if (endptr == buffer || *endptr != '\0') {
        return -1; // 转换失败（非数字内容）
    }

    return 0;
}


int extractData(const char *input, char *data1, char *data2) {
    // 查找第一个x和y的位置
    const char *start1 = strchr(input, 'x');
    if (!start1) return -1; // 找不到x，返回错误

    const char *end1 = strchr(start1 + 1, 'y');
    if (!end1) return -1; // 找不到y，返回错误

    // 提取第一个数据
    size_t len1 = end1 - start1 - 1;
    if (len1 <= 0) return -1; // 数据长度为0
    strncpy(data1, start1 + 1, len1);
    data1[len1] = '\0'; // 确保字符串终止

    // 查找第二个x和y的位置
    const char *start2 = strchr(end1 + 1, 'x');
    if (!start2) return -1;

    const char *end2 = strchr(start2 + 1, 'y');
    if (!end2) return -1;

    // 提取第二个数据
    size_t len2 = end2 - start2 - 1;
    if (len2 <= 0) return -1;
    strncpy(data2, start2 + 1, len2);
    data2[len2] = '\0';

    return 0; // 成功提取
}

void parse_data(char *input) {
    char *token = strtok(input, ","); // 使用逗号分割
    while (token != NULL) {
    	char q1;
//    	q1=token;
        value1 = atoi(token);
         printf("Data: %d\n", value1);
         printf("\r\nData: %s\n", token);
//         if(token == '0x01')printf("ok");
        token = strtok(NULL, ","); // 获取下一个数据
    }
}



void Right_Motor_Forward(){
  HAL_GPIO_WritePin(INA1_GPIO_Port, INA1_Pin,GPIO_PIN_SET);
            HAL_GPIO_WritePin(INA2_GPIO_Port, INA2_Pin,GPIO_PIN_RESET);
//            HAL_GPIO_WritePin(INB1_GPIO_Port, INB1_Pin,GPIO_PIN_SET);
//            HAL_GPIO_WritePin(INB2_GPIO_Port, INB2_Pin,GPIO_PIN_RESET);
}
void Right_Motor_Reverse(){
  HAL_GPIO_WritePin(INA1_GPIO_Port, INA1_Pin,GPIO_PIN_RESET);
            HAL_GPIO_WritePin(INA2_GPIO_Port, INA2_Pin,GPIO_PIN_SET);
//            HAL_GPIO_WritePin(INB1_GPIO_Port, INB1_Pin,GPIO_PIN_RESET);
//            HAL_GPIO_WritePin(INB2_GPIO_Port, INB2_Pin,GPIO_PIN_SET);
}
void Left_Motor_Forward(){

            HAL_GPIO_WritePin(INB1_GPIO_Port, INB1_Pin,GPIO_PIN_SET);
            HAL_GPIO_WritePin(INB2_GPIO_Port, INB2_Pin,GPIO_PIN_RESET);
}
void Left_Motor_Reverse(){

            HAL_GPIO_WritePin(INB1_GPIO_Port, INB1_Pin,GPIO_PIN_RESET);
            HAL_GPIO_WritePin(INB2_GPIO_Port, INB2_Pin,GPIO_PIN_SET);
}

int fputc(int ch, FILE *f){
 uint8_t temp[1] = {ch};
 HAL_UART_Transmit(&huart1, temp, 1, 0xffff);
return ch;
}
int fgetc(FILE * f)
{
  uint8_t ch = 0;
  HAL_UART_Receive(&huart1,&ch, 1, 0xffff);
  return ch;
}
/* USER CODE END PV */


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
unsigned int  leftSpeed; // 锟斤拷锟斤拷锟角帮拷俣锟斤拷锟街碉拷锟斤拷颖锟斤拷锟斤拷锟斤拷谢锟饺�???
  unsigned int rightSpeed;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  uint8_t x = 0;
  uint8_t lcd_id[12];
  uint8_t key;
  uint8_t len;
  uint8_t angle=70;
  uint8_t t = 0;
  char *str = 0;
  int16_t left_speed_now;
  int16_t right_speed_now;
  static int16_t speed_left_SetPoint = 0.0f;
  static int16_t speed_right_SetPoint = 0.0f;
//  PID_LocTypeDef left_speed_PID = {115,0.0,0.0,0.0,0.0,0.0};
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  delay_init(72);						   /* 锟斤拷始锟斤拷锟斤拷时锟斤拷锟斤�??? */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FSMC_Init();
  MX_TIM4_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM6_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
//  BSP_MPU6050_Init();
//  BSP_MPU6050_UpdateSensors();
  IMU_SensorData_Raw_Structer IMU_SensorData_Raw;


  lcd_init();/* 锟斤拷始锟斤拷LCD */
//  lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
//  lcd_show_string(30, 70, 200, 16, 16, "REMOTE TEST", RED);
//  lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
//  lcd_show_string(30, 110, 200, 16, 16, "KEYVAL:", RED);
//  lcd_show_string(30, 130, 200, 16, 16, "KEYCNT:", RED);/* 锟斤拷示LCD ID */

  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);//锟斤�???
    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);//锟斤�???
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);//锟斤�???
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);//锟斤�???

    __HAL_TIM_CLEAR_FLAG(&htim6,TIM_FLAG_UPDATE);
    HAL_TIM_Base_Start_IT(&htim6);

  g_point_color = RED;
//  sprintf((char *)lcd_id, "LCD ID:%04X", lcddev.id);  /* 锟斤拷LCD ID锟斤拷印锟斤拷lcd_id锟斤拷锟斤拷 */
char a="asdsadas";

//  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 100);
//  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 100);
//  HAL_GPIO_WritePin(INA1_GPIO_Port, INA1_Pin,GPIO_PIN_RESET);
//  HAL_GPIO_WritePin(INA2_GPIO_Port, INA2_Pin,GPIO_PIN_SET);
//  HAL_GPIO_WritePin(INB1_GPIO_Port, INB1_Pin,GPIO_PIN_RESET);
//  HAL_GPIO_WritePin(INB2_GPIO_Port, INB2_Pin,GPIO_PIN_SET);

PID_IncTypeDef motorPID = {
    .Kp = 65.2,
    .Ki = 0.05,
    .Kd = 0.5,
    .Ek = 0,
    .Ek1 = 0,
    .Ek2 = 0,
    .LastOutput = 0
};

PID_IncTypeDef motorPID1 = {
    .Kp = 65.2,
    .Ki =0.2,
    .Kd = 0.05,
    .Ek = 0,
    .Ek1 = 0,
    .Ek2 = 0,
    .LastOutput = 0
};




  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//      user_main_printf("hello world!");

//    char  ch="hel";
//          HAL_UART_Transmit(&huart1,&ch,1,0);
//      Send_USART_String(&huart3, "Hello, STM32!\n");
      char str1[20]; // 存储转换后的字符�???
      char str2[20];
      char str3[20];
      char str4[20];
      char str5[len + 1];
        memset(str5, 0, sizeof(str5));
         // 使用 sprintf 将整数转换为字符�???
int value;

//      int aq=123456;
//        sprintf(str1, "%d", aq);
//      printf("%s\r\n",str1);
//      printf(132);






//      HAL_UART_Transmit(&huart1, "qwe\r\n", 30, HAL_MAX_DELAY);
      Get_Motor_Speed(&leftSpeed,&rightSpeed);
//      printf("%d \n",leftSpeed);

//      left_speed_now = Get_Left_Motor_Speed(left_speed_now);
      left_speed_now =Get_Right_Motor_Speed(left_speed_now);
      right_speed_now=-Get_Left_Motor_Speed(right_speed_now);
//                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, PID_Loc(abs(speed_left_SetPoint), abs(left_speed_now),&left_speed_PID));
//      float left_pid_output = PID_Loc(speed_left_SetPoint,left_speed_now, &left_speed_PID);
//float right_pid_output=PID_Loc(speed_right_SetPoint,right_speed_now, &left_speed_PID);
      float left_pid_output = PID_Loc(left_speed_now,speed_left_SetPoint,&motorPID);
  float right_pid_output=PID_Loc1(right_speed_now,speed_right_SetPoint, &motorPID1);

//
//      if (left_pid_output < 0) {
//	   Right_Motor_Reverse();
//
//      } else {
//	  Right_Motor_Forward();
//
//      }
//
//       if (right_pid_output > 0) {
////		  Left_Motor_Reverse();
//      	 Left_Motor_Forward();
////      	Left_Motor_Reverse();
//            } else {
//      	 Left_Motor_Reverse();
////      	Left_Motor_Forward();
//            }
  if (left_pid_output > 0) {
   Right_Motor_Reverse();

  } else {
  Right_Motor_Forward();

  }

   if (right_pid_output < 0) {
//		  Left_Motor_Reverse();
  	 Left_Motor_Forward();
//      	Left_Motor_Reverse();
        } else {
  	 Left_Motor_Reverse();
//      	Left_Motor_Forward();
        }



     __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, abs(right_pid_output));
     __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, abs(left_pid_output));
//                lcd_show_num(100, 210,abs(left_pid_output), 10, 16, RED);
//                lcd_show_string(10, 210, 240, 16, 16, "pidout", RED);
//                lcd_show_string(10, 230, 240, 16, 16, "Kp", RED);
//                lcd_show_num(100, 230,left_speed_PID.Kp, 10, 16, RED);
//                lcd_show_string(10, 250, 240, 16, 16, "Ki", RED);
//                lcd_show_num(100, 250,left_speed_PID.Ki, 10, 16, RED);
     lcd_show_num(100, 210,abs(left_pid_output), 10, 16, RED);
                     lcd_show_string(10, 210, 240, 16, 16, "pidout", RED);
                     lcd_show_string(10, 230, 240, 16, 16, "Kp", RED);
                     lcd_show_num(100, 230,motorPID.Kp, 10, 16, RED);
                     lcd_show_string(10, 250, 240, 16, 16, "Ki", RED);
                     lcd_show_num(100, 250,motorPID.Ki, 10, 16, RED);
//                printf("%d,%d",&left_speed_now, &pid_output);
//                printf("%lf,%lf,%lf\r\n",left_speed_PID.Kp,left_speed_PID.Ki,left_speed_PID.Kd);
                sprintf(str1, "%d", left_speed_now);
                sprintf(str2, "%d", speed_left_SetPoint);
                sprintf(str3, "%d", right_speed_now);




//                 printf("%s,%s,%s,%lf,%lf \r\n",str1,str3,str2,left_speed_PID.Kp,left_speed_PID.Ki);

//                printf("%s  %s %s\r\n",str1,str3,str2);
//                printf("ALIENTEK\r\n\r\n\r\n");
//                if (g_usart_rx_sta & 0x8000)        /* 接收到了数据? */
//               		  {
//               			  len = g_usart_rx_sta & 0x3fff;  /* 得到此次接收到的数据长度 */
////               			  printf("\r\THE code is:\r\n");
//
//               			  HAL_UART_Transmit(&huart1,(uint8_t*)g_usart_rx_buf, len, 1000);    /* 发送接收到的数据 */
//
//
//               			  while(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) != SET);          /* 等待发送结束 */
//
//               			  printf("\r\n\r\n");             /* 插入换行 */
//               			  g_usart_rx_sta = 0;
//               		  }
//
//                else
//                	{
////                	printf("***%s %s %s &&&\r\n",str1,str3,str2);
////                	HAL_Delay(20);
//                	for (uint16_t i = 0; i < len; i++) {
//               						      str5[i] = g_usart_rx_buf[i];
//               						  }
//               						  str5[len] = '\0';
//               						   value = atoi(str5);
//               						printf("***%s %s %s %d&&&\r\n",str1,str3,str2,value);
//               						                	HAL_Delay(20);
////               						  printf("\r\ndouble: %d\n", value);
//                	}
//
                if (g_usart_rx_sta & 0x8000)        /* 接收到了数据? */
                {
                    len = g_usart_rx_sta & 0x3fff;  /* 得到此次接收到的数据长度 */
                    HAL_UART_Transmit(&huart1, (uint8_t*)g_usart_rx_buf, len, 1000);    /* 发送接收到的数据 */
                    while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) != SET);          /* 等待发送结束 */
                    printf("\r\n\r\n");             /* 插入换行 */

                    g_usart_rx_sta = 0;
                }
                else
                {
                    // 判断数据的第一个字节是 0x01 还是 0x02
//                    if (g_usart_rx_buf[0] == 0x01) {
//                        // 主机命令
////                        printf("*** Host command received: 0x01 ***\r\n");
//                        // 这里可以添加主机命令的处理逻辑
//                        HAL_Delay(20);
//                    } else if (g_usart_rx_buf[0] == 0x02) {
//                        // yaw 数据
////                        printf("*** Yaw data received: 0x02 ***\r\n");
//                        // 这里可以添加 yaw 数据的处理逻辑
//                        HAL_Delay(20);
//                    }
                    // 清空接收缓冲区参数
                    for (uint16_t i = 0; i < len; i++) {
                        str5[i] = g_usart_rx_buf[i];
                    }
                    str5[len] = '\0';
                    value = atoi(str5);

//                    parse_data(str5);
                    if (extractData(str5, data1, data2) == 0) {
                            printf("\r\nData1: %s\n", data1); // 输出 Apple
                            printf("\r\nData2: %s\n", data2); // 输出 Orange
                        } else {
//                            printf("提取失败：格式错误\n");
                        }

                    value2 = atoi(data1);
                    value3 = atoi(data2);
                    lcd_show_num(100, 270,value1, 10, 16, BLUE);
                    lcd_show_num(100, 290,value2, 10, 16, BLUE);
                    lcd_show_num(10, 300,value3, 10, 16, BLUE);
                    if (extractZXData(str5, &number) == 0) {
                            printf("\r\n%.2f\n", number); // 输出 45.20
                        } else {
//                            printf("提取失败：无效格式\n");
                        }
                    value1=number;
                      printf("***%s %s %s&&&\r\n", str1, str3, str2);
                }
                speed_right_SetPoint=value3;
                      speed_left_SetPoint=value2;

//                if (g_usart_rx_sta & 0x8000) {
//                    len = g_usart_rx_sta & 0x3fff;
//                    extract_data(g_usart_rx_buf, len);
//                    g_usart_rx_sta = 0;
//                }

//int *a123=*(uint8_t*)g_usart_rx_buf;
//                uint8_t value = *((uint8_t*)g_usart_rx_buf);
//                sprintf(str4, "%d", value);




//                if (value<0){value=abs(value)+180;}
//lcd_show_num(100, 270,value, 10, 16, BLUE);
//                lcd_show_string(10, 270, 240, 16, 16, "yaw", RED);
//
//
//                if(value!=0){
//
//
//                              if(value>angle+5 ){
//                            	 speed_right_SetPoint=5;
//                            	 speed_left_SetPoint=-5;
//                //                	Left_Motor_Forward();
//                //                	Right_Motor_Reverse();
//                //                	 __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 100);
//                //                	      __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 100);
//                             }
//
//                             else if(value<angle-5) {
//                            	 speed_right_SetPoint=-5;
//                            	 speed_left_SetPoint=5;
//
////                             	Right_Motor_Forward();
////                             	                	Left_Motor_Reverse();
//                //                	                	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 100);
//                //                	                	                	      __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 100);
//                             }
//                             else{
//
//                            	  speed_right_SetPoint=0;
//                            	 speed_left_SetPoint=0;
//                            	 left_pid_output=0;
//                            	 right_pid_output=0;
//
//                             	 __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
//                             	                	      __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
//                             }
//                }





              //                printf("%lf %lf %lf\n",&left_speed_PID.Kp,&left_speed_PID.Ki,&left_speed_PID.Kd);
//printf("Hello, STM32!\r\n");

//      BSP_MPU6050_UpdateSensors();
//  float a=IMU_SensorData_Raw.ACC_X;
//     lcd_show_num(5, 230, IMU_SensorData_Raw.ACC_X, 10, 16,  BLUE);
//     lcd_show_num(15, 210, IMU_SensorData_Raw.ACC_Y, 10, 16,  BLUE);
//     lcd_show_num(15, 190, IMU_SensorData_Raw.ACC_Z, 10, 16,  BLUE);
//     lcd_show_num(20, 170, IMU_SensorData_Raw.GYR_X, 10, 16,  BLUE);
//     lcd_show_num(25, 150, IMU_SensorData_Raw.GYR_Y, 10, 16,  BLUE);
//     lcd_show_num(30, 130, IMU_SensorData_Raw.GYR_Z, 10, 16,  BLUE);
//lcd_show_num(10, 250, key, 3, 16, BLUE);
      key = remote_scan();
           if (key)
           {
           lcd_show_num(10, 270, key, 3, 16, BLUE);
           lcd_show_num(10, 290, g_remote_cnt, 3, 16, BLUE);
           switch (key)
           {
           case 0:
           str = "ERROR";
           break;
           case 69:
           str = "POWER";
           HAL_GPIO_WritePin(INA1_GPIO_Port, INA1_Pin,GPIO_PIN_RESET);
           HAL_GPIO_WritePin(INA2_GPIO_Port, INA2_Pin,GPIO_PIN_RESET);
           HAL_GPIO_WritePin(INB1_GPIO_Port, INB1_Pin,GPIO_PIN_RESET);
           HAL_GPIO_WritePin(INB2_GPIO_Port, INB2_Pin,GPIO_PIN_RESET);
           break;
           case 70:
                    str = "UP";
                    speed_left_SetPoint = 50.0f;
                    speed_right_SetPoint = 50.0f;
                    break;
                    case 64:
                    str = "PLAY";
                    speed_left_SetPoint = 0.0f;
                              speed_right_SetPoint = 0.0f;
                    break;
                    case 71:
                    str = "ALIENTEK";
                    break;
                    case 67:
                    str = "RIGHT";
                    speed_left_SetPoint = -50.0f;
                              speed_right_SetPoint = 50.0f;
                    break;
                    case 68:
                    str = "LEFT";
                    speed_left_SetPoint = 50.0f;
                   speed_right_SetPoint = -50.0f;
                   break;
                    case 7:
                    str = "VOL-";
                    speed_left_SetPoint = 0.0f;
                              speed_right_SetPoint = 0.0f;
                    break;
                    case 21:
                    str = "DOWN";
                    speed_left_SetPoint = -50.0f;
                              speed_right_SetPoint = -50.0f;
                              break;
           case 9:
           str = "VOL+";
           break;
           case 22:
           str = "1";
//           HAL_GPIO_WritePin(INA1_GPIO_Port, INA1_Pin,GPIO_PIN_RESET);
//           HAL_GPIO_WritePin(INA2_GPIO_Port, INA2_Pin,GPIO_PIN_SET);
//           HAL_GPIO_WritePin(INB1_GPIO_Port, INB1_Pin,GPIO_PIN_RESET);
//           HAL_GPIO_WritePin(INB2_GPIO_Port, INB2_Pin,GPIO_PIN_SET);
           break;
           case 25:
           str = "2";
//           HAL_GPIO_WritePin(INA1_GPIO_Port, INA1_Pin,GPIO_PIN_RESET);
//           HAL_GPIO_WritePin(INA2_GPIO_Port, INA2_Pin,GPIO_PIN_SET);
//           HAL_GPIO_WritePin(INB1_GPIO_Port, INB1_Pin,GPIO_PIN_RESET);
//           HAL_GPIO_WritePin(INB2_GPIO_Port, INB2_Pin,GPIO_PIN_SET);
           break;
           case 13:
           str = "3";
           case 12:
           str = "4";
           speed_left_SetPoint=-20;
//           HAL_GPIO_WritePin(INA1_GPIO_Port, INA1_Pin,GPIO_PIN_SET);
//           HAL_GPIO_WritePin(INA2_GPIO_Port, INA2_Pin,GPIO_PIN_RESET);
//           HAL_GPIO_WritePin(INB1_GPIO_Port, INB1_Pin,GPIO_PIN_SET);
//           HAL_GPIO_WritePin(INB2_GPIO_Port, INB2_Pin,GPIO_PIN_RESET);
           break;
           case 24:
           str = "5";
//           left_speed_PID.Kp+=0.5;
           break;
           case 94:
           str = "6";
//           left_speed_PID.Kp-=0.5;
//           int left_speed_now;
//           left_speed_now = Get_Left_Motor_Speed(left_speed_now);
//                     __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, PID_Loc(abs(speed_left_SetPoint), abs(left_speed_now),&left_speed_PID));
//                     lcd_show_num(100, 210, PID_Loc(abs(speed_left_SetPoint), abs(left_speed_now),&left_speed_PID), 10, 16, RED);
           break;
           case 8:
           str = "7";
//           speed_left_SetPoint=20;
           break;
           case 28:
           str = "8";
//           left_speed_PID.Ki+=0.1;
           break;
           case 90:
           str = "9";
//           left_speed_PID.Ki-=0.1;
           break;
           case 66:
           str = "0";
           speed_left_SetPoint=50;
           break;
           case 74:
           str = "DELETE";
//           left_speed_PID.Ki+=5;
           break;
           }
//           lcd_fill(86, 150, 116 + 8 * 8, 170 + 16, WHITE);
//           lcd_show_string(86, 150, 200, 16, 16, str, BLUE);
           }
           else
           {
           delay_ms(10);
           } t ++;
           if (t == 20)
             {
             t = 0;
             LED0_TOGGLE(); /* LED0 锟斤拷烁 */
             }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int getTIMx_DetaCnt(TIM_HandleTypeDef *htim)
{
    int cnt;
    cnt = htim->Instance->CNT - 0x7FFF;
    htim->Instance->CNT = 0x7FFF;
    lcd_show_string(10, 50, 240, 16, 16, "CNT", RED);
    lcd_show_num(10, 70, cnt, 10, 16, RED);
    return cnt;
}

int16_t Get_Right_Motor_Speed(int16_t leftSpeed)
{
			static int leftWheelEncoderNow    = 0;
			static int leftWheelEncoderLast   = 0;

			//锟斤拷录锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�???
			leftWheelEncoderNow +=  getTIMx_DetaCnt(&htim3);;

			//5ms锟斤拷锟斤拷
			leftSpeed   =  20 * (leftWheelEncoderNow - leftWheelEncoderLast)*10 / 1000;  //锟劫讹拷为cm/s
			//锟斤拷录锟较次憋拷锟斤拷锟斤拷锟斤拷锟斤�???
			leftWheelEncoderLast  = leftWheelEncoderNow;
      return leftSpeed;

}
int16_t Get_Left_Motor_Speed(int16_t rightSpeed)
{
			static int rightWheelEncoderNow   = 0;
			static int rightWheelEncoderLast  = 0;

			//锟斤拷录锟斤拷锟斤拷锟揭憋拷锟斤拷锟斤拷锟斤拷锟斤拷
			rightWheelEncoderNow+= getTIMx_DetaCnt(&htim2);


			rightSpeed  =  20.7 * (rightWheelEncoderNow - rightWheelEncoderLast)*10/ 1000;

			//锟斤拷录锟较次憋拷锟斤拷锟斤拷锟斤拷锟斤�???
			rightWheelEncoderLast = rightWheelEncoderNow;
      return rightSpeed;
}


void Get_Motor_Speed(int *leftSpeed, int *rightSpeed)
{
  char str[1000];
//  lcd_show_num(10, 250,6666, 4, 16, BLUE);
//  printf("\r\n6666\r\n");
//  lcd_clear(BLACK);
  lcd_show_string(10, 20, 240, 16, 16, "rightSpeed", RED);
  lcd_show_string(100, 20, 240, 16, 16, "lefttSpeed", BLUE);

//  *rightSpeed = getTIMx_DetaCnt(&htim3); *leftSpeed = getTIMx_DetaCnt(&htim2);
  *rightSpeed =Get_Right_Motor_Speed(rightSpeed);
  *leftSpeed = Get_Left_Motor_Speed(leftSpeed);
    lcd_show_string(10, 90, 240, 16, 16, "+turn", RED);
    lcd_show_string(10, 130, 240, 16, 16, "-turn", RED);
    if( *rightSpeed<0  )

      {

//	 intToString(*rightSpeed, str);
//	 lcd_show_string(10, 150, 240, 16, 16, str, RED);
//	lcd_show_num(10, 150,abs(*rightSpeed/4) , 10, 16, RED);
	lcd_show_num(10, 150,abs(*rightSpeed) , 10, 16, RED);

      }
    else  {
//lcd_show_num(10, 110, *rightSpeed/4, 10, 16, RED);
lcd_show_num(10, 110, *rightSpeed, 10, 16, RED);

    }

if (*leftSpeed>=0){
//    intToString(*leftSpeed*2, str);
//  lcd_show_string(90, 150, 240, 16, 16, str, BLUE);}
//     lcd_show_num(92, 150, *leftSpeed/4, 10, 16,  BLUE);}
lcd_show_num(92, 150, *leftSpeed, 10, 16,  BLUE);}
else
//  lcd_show_num(90, 110,abs(*leftSpeed/4) , 10, 16, BLUE);
  lcd_show_num(90, 110,abs(*leftSpeed) , 10, 16, BLUE);

//start();
//lcd_show_num(10, 260, g_remote_cnt, 3, 16, BLUE);

//
//if (g_remote_sta & 0x80)      /* 锟较达拷锟斤拷锟斤拷锟捷憋拷锟斤拷锟秸碉拷锟斤拷 */
//          {
//              g_remote_sta &= ~0X10;    /* 取锟斤拷锟斤拷锟斤拷锟斤拷锟窖撅拷锟斤拷锟斤拷锟斤拷锟斤�??? */
//
//              if ((g_remote_sta & 0X0F) == 0X00)
//              {
//                  g_remote_sta |= 1 << 6; /* 锟斤拷锟斤拷丫锟斤拷锟斤拷一锟轿帮拷锟斤拷锟侥硷拷�?�锟斤拷息锟缴硷�??? */
//              }
//
//              if ((g_remote_sta & 0X0F) < 14)
//              {
//                  g_remote_sta++;
//              }
//              else
//              {
//                  g_remote_sta &= ~(1 << 7);    /* 锟斤拷锟斤拷锟斤拷锟斤拷锟绞�??? */
//                  g_remote_sta &= 0XF0;         /* 锟斤拷占锟斤拷锟斤拷锟� */
//              }
//          }



}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{


    if (htim == &htim4){
      if (htim->Instance == REMOTE_IN_TIMX)
      {
          if (g_remote_sta & 0x80)      /* 锟较达拷锟斤拷锟斤拷锟捷憋拷锟斤拷锟秸碉拷锟斤拷 */
          {
              g_remote_sta &= ~0X10;    /* 取锟斤拷锟斤拷锟斤拷锟斤拷锟窖撅拷锟斤拷锟斤拷锟斤拷锟斤�??? */

              if ((g_remote_sta & 0X0F) == 0X00)
              {
                  g_remote_sta |= 1 << 6; /* 锟斤拷锟斤拷丫锟斤拷锟斤拷一锟轿帮拷锟斤拷锟侥硷拷�?�锟斤拷息锟缴硷�??? */
              }

              if ((g_remote_sta & 0X0F) < 14)
              {
                  g_remote_sta++;
              }
              else
              {
                  g_remote_sta &= ~(1 << 7);    /* 锟斤拷锟斤拷锟斤拷锟斤拷锟绞�??? */
                  g_remote_sta &= 0XF0;         /* 锟斤拷占锟斤拷锟斤拷锟� */
              }
          }
      }
    }



//    else   if (htim == &htim6)
//    {
//        Get_Motor_Speed(&leftSpeed,&rightSpeed);
//
//    }


}
//
//void start(){
////  lcd_show_num(10, 250, key, 3, 16, BLUE);
//
//  uint8_t x = 0;
//   uint8_t lcd_id[12];
//   uint8_t key;
//   uint8_t t = 0;
//   char *str = 0;
//
//        key = remote_scan();
//             if (key)
//             {
//             lcd_show_num(10, 250, key, 3, 16, BLUE);
//             lcd_show_num(10, 260, g_remote_cnt, 3, 16, BLUE);
//             switch (key)
//             {
//             case 0:
//             str = "ERROR";
//             break;
//             case 69:
//             str = "POWER";
//             break;
//             case 70:
//             str = "UP";
//             break;
//             case 64:
//             str = "PLAY";
//             break;
//             case 71:
//             str = "ALIENTEK";
//             break;
//             case 67:
//             str = "RIGHT";
//             break;
//             case 68:
//             str = "LEFT";
//             break;
//             case 7:
//             str = "VOL-";
//             break;
//             case 21:
//             str = "DOWN";
//             break;
//             case 9:
//             str = "VOL+";
//             break;
//             case 22:
//             str = "1";
//             HAL_GPIO_WritePin(INA1_GPIO_Port, INA1_Pin,GPIO_PIN_RESET);
//             HAL_GPIO_WritePin(INA2_GPIO_Port, INA2_Pin,GPIO_PIN_SET);
//             HAL_GPIO_WritePin(INB1_GPIO_Port, INB1_Pin,GPIO_PIN_RESET);
//             HAL_GPIO_WritePin(INB2_GPIO_Port, INB2_Pin,GPIO_PIN_SET);
//             break;
//             case 25:
//             str = "2";
//             HAL_GPIO_WritePin(INA1_GPIO_Port, INA1_Pin,GPIO_PIN_RESET);
//             HAL_GPIO_WritePin(INA2_GPIO_Port, INA2_Pin,GPIO_PIN_SET);
//             HAL_GPIO_WritePin(INB1_GPIO_Port, INB1_Pin,GPIO_PIN_RESET);
//             HAL_GPIO_WritePin(INB2_GPIO_Port, INB2_Pin,GPIO_PIN_SET);
//             break;
//             case 13:
//             str = "3";
//             break;
//             case 12:
//             str = "4";
//             HAL_GPIO_WritePin(INA1_GPIO_Port, INA1_Pin,GPIO_PIN_SET);
//             HAL_GPIO_WritePin(INA2_GPIO_Port, INA2_Pin,GPIO_PIN_RESET);
//             HAL_GPIO_WritePin(INB1_GPIO_Port, INB1_Pin,GPIO_PIN_SET);
//             HAL_GPIO_WritePin(INB2_GPIO_Port, INB2_Pin,GPIO_PIN_RESET);
//             break;
//             case 24:
//             str = "5";
//             break;
//             case 94:
//             str = "6";
//             break;
//             case 8:
//             str = "7";
//             break;
//             case 28:
//             str = "8";
//             break;
//             case 90:
//             str = "9";
//             break;
//             case 66:
//             str = "0";
//             break;
//             case 74:
//             str = "DELETE";
//             break;
//             }
//             lcd_fill(86, 150, 116 + 8 * 8, 170 + 16, WHITE);
//             lcd_show_string(86, 150, 200, 16, 16, str, BLUE);
//             }
//             else
//             {
//             delay_ms(10);
//             } t ++;
//             if (t == 20)
//               {
//               t = 0;
//               LED0_TOGGLE(); /* LED0 锟斤拷烁 */
//               }
//
//}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
