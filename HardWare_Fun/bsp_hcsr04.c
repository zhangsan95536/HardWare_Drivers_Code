#include "stm32f10x.h"                  // Device header
#include "bsp_hcsr04.h"
#include "delay.h"


void HCSR04GpioConfig()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(HCSR04_RCC,ENABLE);
	 //	trigger
	GPIO_InitStructure.GPIO_Pin 	= HCSR04_TX_PIN;   
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_Init(HCSR04_GPIO_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(HCSR04_GPIO_PORT,HCSR04_TX_PIN);
	 //	echo
	GPIO_InitStructure.GPIO_Pin 	= HCSR04_RX_PIN;     
//	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_Init(HCSR04_GPIO_PORT, &GPIO_InitStructure);  	
}

void Timer3Init()
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); 
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period 		= 0xffff; 
	TIM_TimeBaseStructure.TIM_Prescaler 	= 72 -1 ; 
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
	TIM_TimeBaseStructure.TIM_CounterMode 	= TIM_CounterMode_Up;  
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	TIM_Cmd(TIM3, ENABLE); 
}

float Hcsr04GetLength(void)
{
	uint16_t timeStart = 0;
	uint16_t timeEnd = 0;
	uint16_t timeLast = 0;
	float length = 0;
	float sum = 0;
	uint16_t i = 0;
	
	uint32_t timeout = 0;
	
	while(i != 5)
	{
		
		// 发送触发脉冲前确保TRIG为低电平
		GPIO_ResetBits(HCSR04_GPIO_PORT, HCSR04_TX_PIN);
		Delay_us(5);
		
		
		//trig拉高信号，发出高电平
		GPIO_SetBits(HCSR04_GPIO_PORT,HCSR04_TX_PIN);
		Delay_us(20);
		GPIO_ResetBits(HCSR04_GPIO_PORT,HCSR04_TX_PIN);
		/*Echo发出信号 等待回响信号*/
		/*输入方波后，模块会自动发射8个40KHz的声波，与此同时回波引脚（echo）端的电平会由0变为1；
		（此时应该启动定时器计时）；当超声波返回被模块接收到时，回波引 脚端的电平会由1变为0；
		（此时应该停止定时器计数），定时器记下的这个时间即为
		超声波由发射到返回的总时长；*/
		//echo等待回响
		
		timeout = 0;
		
//		while(GPIO_ReadInputDataBit(HCSR04_GPIO_PORT,HCSR04_RX_PIN) == 0);
		//记录放送时间T1
		
		while(GPIO_ReadInputDataBit(HCSR04_GPIO_PORT, HCSR04_RX_PIN) == 0)
        {
            timeout++;
            if(timeout > 60000) // 根据实际情况调整超时值
            {
				Delay_ms(100);
                return 1.0; // 超时返回0
            }
        }
		
		timeStart = TIM3->CNT;
		i = i+1; //每收到一次回响信号+1，收到5次就计算均值
//		while(GPIO_ReadInputDataBit(HCSR04_GPIO_PORT,HCSR04_RX_PIN) == 1);
		
		timeout = 0;
        while(GPIO_ReadInputDataBit(HCSR04_GPIO_PORT, HCSR04_RX_PIN) == 1)
        {
            timeout++;
            if(timeout > 30000) // 根据实际情况调整超时值
            {
				Delay_ms(100);
                return 2.0; // 超时返回0
            }
        }
		
		//记录接收时间T3
		timeEnd = TIM3->CNT;
		/*获取Echo高电平时间时间T3*/
		timeLast = (timeEnd > timeStart) ? (timeEnd - timeStart) : (0xffff - timeStart + timeEnd);
		//单位时cm
		length = (float)timeLast / 58;
		
		
		// 过滤异常值
		if(length > 400.0f || length < 2.0f)
		{
			i--; // 不计入有效测量
			continue;
		}
		
		sum += length;		
		// 重要：每次测量后等待至少60ms，让模块恢复
		Delay_ms(70);
	}
	if(i == 0) return 0.0f; // 没有有效测量
	length = sum / i;
	
	return length;
}

void HCSR04Init()
{
	HCSR04GpioConfig();
	Timer3Init();
}