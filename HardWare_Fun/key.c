#include "stm32f10x.h"                  // Device header
#include "delay.h"
void Key_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	//GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IPU;//上拉输入 默认高电平,使按键按下为0，松手为1
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IPD;//下拉输入 默认低电平,使按键按下为1，松手为0
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_12;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
}

uint8_t Key_GetKeynum()
{
	uint8_t keyNum=0;
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==0)//如果按键A8按下
	{
		Delay_ms(20);//按键按下机械抖动
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==0);
		Delay_ms(20);//按键松手抖动
		keyNum=12;
	}
//	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)==0)//如果按键B11按下
//	{
//		Delay_ms(20);//按键按下机械抖动
//		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)==0);
//		Delay_ms(20);//按键松手抖动*//		keyNum=11;
//	}
	return keyNum;
}