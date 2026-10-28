#include "stm32f10x.h"                  // Device header
#include "PWM.h"
#include "OLED.h"
void Motor_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能APB2 peripheral--GPIOA clock
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;	//推挽输出
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;//依次为(inA1,inA2，inB1,inB2)
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);			//初始化GPIO,然后可以使用GPIO函数
	
	// 初始化控制引脚为低电平
    GPIO_ResetBits(GPIOA, GPIO_Pin_2);
    GPIO_ResetBits(GPIOA, GPIO_Pin_3);
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    GPIO_ResetBits(GPIOA, GPIO_Pin_5);
	PWM_Init();
}

void Motor_SetSpeed(int8_t motorSpeedL,int8_t motorSpeedR)
{
	if(motorSpeedL>0)//左轮正转
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_3);
		GPIO_ResetBits(GPIOA,GPIO_Pin_2);
		PWM_SetCompare1(motorSpeedL);
	}
	if(motorSpeedL<0)//左轮反转
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_2);
		GPIO_ResetBits(GPIOA,GPIO_Pin_3);
		PWM_SetCompare1(-motorSpeedL);
	}
	if(motorSpeedL==0)//左轮停止
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_2);
		GPIO_ResetBits(GPIOA,GPIO_Pin_3);
		PWM_SetCompare1(0);
	}
	
	
	if(motorSpeedR>0)//右轮正转
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_4);
		GPIO_ResetBits(GPIOA,GPIO_Pin_5);
		PWM_SetCompare2(motorSpeedR);
	}
	if(motorSpeedR<0)//右轮反转
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_5);
		GPIO_ResetBits(GPIOA,GPIO_Pin_4);
		PWM_SetCompare2(-motorSpeedR);
	}
	
	if(motorSpeedR==0)//右轮停止
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_4);
		GPIO_ResetBits(GPIOA,GPIO_Pin_5);
		PWM_SetCompare2(0);
	}
}