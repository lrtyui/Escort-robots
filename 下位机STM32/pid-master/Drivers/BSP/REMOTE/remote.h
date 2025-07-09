

#ifndef __REMOTE_H
#define __REMOTE_H

#include "../../SYSTEM/sys/sys.h"
#include "tim.h"

/******************************************************************************************/
/* 红外输入引脚及定时器 定义 */

#define REMOTE_IN_TIMX                          TIM4                       
#define REMOTE_IN_TIMX_IRQn                     TIM4_IRQn
#define REMOTE_IN_TIMX_CHY                      TIM_CHANNEL_4                               /* 通道Y,  1<= Y <=2*/
#define REMOTE_IN_TIMX_CCRY                     REMOTE_IN_TIMX->CCR4
#define REMOTE_IN_TIMX_CHY_CLK_ENABLE()         do{ __HAL_RCC_TIM4_CLK_ENABLE(); }while(0)  /* TIMX 时钟使能 */

/******************************************************************************************/


#define RDATA           HAL_GPIO_ReadPin(REMOTE_IN_GPIO_Port, REMOTE_IN_Pin)   /* 红外数据输入脚 */


/* 红外遥控识别码(ID),每款遥控器的该值基本都不一样,但也有一样的.
 * 我们选用的遥控器识别码为0
*/
#define REMOTE_ID       0

extern uint8_t g_remote_cnt;    /* 按键按下的次数 */
extern uint8_t g_remote_sta;
uint8_t remote_scan(void);

#endif















