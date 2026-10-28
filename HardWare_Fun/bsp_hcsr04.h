#ifndef __BSP_HCSR04_H
#define __BSP_HCSR04_H

#include "stm32f10x.h"

#define  HCSR04_RCC				RCC_APB2Periph_GPIOA
#define  HCSR04_GPIO_PORT		GPIOA	
#define  HCSR04_TX_PIN  			GPIO_Pin_7	//	trigger/SCL
#define  HCSR04_RX_PIN			GPIO_Pin_6	//	echo/SDA

void HCSR04Init(void);
float Hcsr04GetLength(void);

#endif /*__BSP_HCSR04_H*/
