#include "stm32f10x.h"                  // Device header
#include "PWM.h"

void Servo_Init()
{
	PWM_Init();
}
//0  -- 500
//180-- 2500
void Servo_SetAngle(float angle)
{
	PWM_SetCompare1(angle/180*2000+500);
}