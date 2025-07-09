

#ifndef LED_H_
#define LED_H_
#include "../../SYSTEM/sys/sys.h"

/******************************************************************************************/
/* LED端口定义 */
#define LED0(x)   do{ x ? \
                      HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET); \
                  }while(0)      /* LED0翻转 */

#define LED1(x)   do{ x ? \
                      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET); \
                  }while(0)      /* LED1翻转 */

/* LED取反定义 */
#define LED0_TOGGLE()   do{ HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin); }while(0)        /* 翻转LED0 */
#define LED1_TOGGLE()   do{ HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin); }while(0)        /* 翻转LED1 */

#endif /* LED_H_ */
