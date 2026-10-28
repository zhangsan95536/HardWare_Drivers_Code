#include "stm32f10x.h"                  // Device header

void LED_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能APB2 peripheral--GPIOA clock
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;	//推挽输出
	//端口默认低电平所以灯会亮
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1;		//GPIOA的0、1号引脚
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);			//初始化GPIO,然后可以使用GPIO函数
	
	GPIO_SetBits(GPIOA,GPIO_Pin_0|GPIO_Pin_1);//将GPIOA的0、1号引脚设置为1
}
void LED0_On()
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_0);//将GPIOA的0号引脚设置为0--点亮LED
}

void LED0_Off()
{
	GPIO_SetBits(GPIOA,GPIO_Pin_0);//将GPIOA的0号引脚设置为1--熄灭LED
}
void LED0_Turn()
{
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)==0)
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_0);
	}
	else
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_0);
	}
}
void LED1_On()
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_1);//将GPIOA的1号引脚设置为0--点亮LED
}

void LED1_Off()
{
	GPIO_SetBits(GPIOA,GPIO_Pin_1);//将GPIOA的1号引脚设置为1--熄灭LED
}
void LED1_Turn()
{
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)==0)
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_1);
	}
	else
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_1);
	}
}
