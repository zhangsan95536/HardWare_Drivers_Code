#include "stm32f10x.h"                  // Device header

/**
  *	PWM公式：
	   72MHz
	f=CK_PSC/(PSC+1)/(1+ARR)
	占空比=CCR/(1+ARR)
	分辨率=1/(1+ARR)
  */

void PWM_Init()
{
	//1.RCC开启时钟，打开TIM、GPIO外设始终
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
		//配置GPIO--复用推挽输出
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能APB2 peripheral--GPIOA clock
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1;		//PWMA,PWMB
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);			//初始化GPIO,然后可以使用GPIO函数
	//2.为时基单元选择时钟源(内部)
	TIM_InternalClockConfig(TIM2);  
	//3.配置时基单元
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period=100-1;		//周期，ARR自动重装器的值
	TIM_TimeBaseInitStruct.TIM_Prescaler=36-1;		//预分频器PSC的值
	
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter=0;	//重复计数器的值,0表示不使用
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStruct);
	//4.配置输出比较单元 
	TIM_OCInitTypeDef TIM_OCInitStruct;
	TIM_OCStructInit(&TIM_OCInitStruct);		//给TIM_OCInitStruct结构体赋初始值
	TIM_OCInitStruct.TIM_OCMode=TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OCPolarity=TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse=0;				//CCR的值
	
	TIM_OC1Init(TIM2,&TIM_OCInitStruct);
	TIM_OC2Init(TIM2,&TIM_OCInitStruct);
	//5.运行控制，启动计数器
	TIM_Cmd(TIM2,ENABLE);
	
}
void PWM_SetCompare1(uint16_t compare)
{
	TIM_SetCompare1(TIM2,compare);
}
void PWM_SetCompare2(uint16_t compare)
{
	TIM_SetCompare2(TIM2,compare);
}