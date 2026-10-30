#ifndef __DRIVER_MPU6050_H
#define __DRIVER_MPU6050_H

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f4xx_hal.h"
#define MPU6050_QUEUE_LEN 10
struct mpu6050_acc {
	
	float ax,ay,az;
};
struct mpu6050_gyr {
	
	float gx,gy,gz;
};
struct mpu6050_Angle {
	float roll,pitch,yaw;
};
struct mpu6050_data {
	struct mpu6050_acc acc;			//加速度计初始值
	struct mpu6050_gyr gyr;			//陀螺仪计初始值
	float temp;						//温度
	struct mpu6050_Angle angle;		//欧拉角
};



void MPU6050_Init(void);
int MPU6050_GetID(void);

int MPU6050_ParseData();
struct mpu6050_data MPU6050_GetData();
void MPU6050_Test(void);
void MPU6050_Task(void *params);


//HAL_StatusTypeDef MPU6050_Init1(void);
//void MPU6050_Test1(void);
#endif /* __DRIVER_OLED_H */

