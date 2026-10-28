#include "stm32f10x.h"                  // Device header

void Buzzer_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//使能APB2 peripheral--GPIOB clock
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;	//推挽输出
	//端口默认低电平所以灯会亮
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_12;		//GPIOB的12号引脚
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);			//初始化GPIO,然后可以使用GPIO函数
	
	GPIO_SetBits(GPIOB,GPIO_Pin_12);//将GPIOA的0、1号引脚设置为1
}
void Buzzer_On()
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);//将GPIOA的0号引脚设置为0--点亮LED
}

void Buzzer_Off()
{
	GPIO_SetBits(GPIOB,GPIO_Pin_12);//将GPIOA的0号引脚设置为1--熄灭LED
}
void Buzzer_Turn()
{
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==0)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_12);
	}
	else
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	}
}
