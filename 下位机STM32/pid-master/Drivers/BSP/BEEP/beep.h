

#ifndef BEEP_BEEP_H_
#define BEEP_BEEP_H_
#include "../../SYSTEM/sys/sys.h"

/******************************************************************************************/

/* ·äÃùÆ÷¿ØÖÆ */
#define BEEP(x)         do{ x ? \
                            HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET) : \
                            HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET); \
                        }while(0)

/* BEEP×´Ì¬·­×ª */
#define BEEP_TOGGLE()   do{ HAL_GPIO_TogglePin(BEEP_GPIO_Port, BEEP_Pin); }while(0)     /* BEEP = !BEEP */


#endif /* BEEP_BEEP_H_ */
