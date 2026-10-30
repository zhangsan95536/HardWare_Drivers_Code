#include "mpu6050.h"
#include "OLED.h"
#include "timer.h"
#include <math.h>
#include <i2c.h>
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core
#include "event_groups.h"               // ARM.FreeRTOS::RTOS:Event Groups

// 定义MPU6050内部地址
#define	MPU6050_ADDRESS			0xD0
#define	MPU6050_PWR_MGMT_1		0x6B //电源管理，典型值：0x00(正常启用)
#define MPU6050_TIMEOUT     	500
#define	MPU6050_SMPLRT_DIV		0x19  // 陀螺仪采样率
#define	MPU6050_ACCEL_CONFIG	0x1C  // 加速计自检、量程及高通滤波频
#define	MPU6050_GYRO_CONFIG		0x1b  // 陀螺仪自检、量程

#if 1
	#define	MPU6050_ACCEL_XOUT_H	0x3B
	#define	MPU6050_ACCEL_XOUT_L	0x3C
	#define	MPU6050_ACCEL_YOUT_H	0x3D
	#define	MPU6050_ACCEL_YOUT_L	0x3E
	#define	MPU6050_ACCEL_ZOUT_H	0x3F
	#define	MPU6050_ACCEL_ZOUT_L	0x40

	#define	MPU6050_TEMP_OUT_H		0x41
	#define	MPU6050_TEMP_OUT_L		0x42

	#define	MPU6050_GYRO_XOUT_H		0x43
	#define	MPU6050_GYRO_XOUT_L		0x44
	#define	MPU6050_GYRO_YOUT_H		0x45
	#define	MPU6050_GYRO_YOUT_L		0x46
	#define	MPU6050_GYRO_ZOUT_H		0x47
	#define	MPU6050_GYRO_ZOUT_L		0x48

	#define	MPU6050_CONFIG			0x1A  // 低通滤波频率，典型值：0x06(5Hz)
	#define MPU6050_MOT_THR 0x1f
	#define MPU6050_MOT_DUR 0x20
	#define	MPU6050_INT_PIN_CFG     0x37
	#define	MPU6050_INT_ENABLE      0x38
	#define	MPU6050_PWR_MGMT_2		0x6C
	#define	MPU6050_WHO_AM_I		0x75 //IIC地址寄存器(默认数值0x68，只读)
#endif

struct mpu6050_data data;
static QueueHandle_t g_xQueueMPU6050;
static EventGroupHandle_t g_xEventGroupMPU6050;

void MPU6050_WriteData(uint8_t reg,uint8_t data)
{
	uint8_t buf[2];
	buf[0]=	reg;//寄存器地址
	buf[1]=	data;//写入的值
	HAL_I2C_Master_Transmit(&hi2c1,MPU6050_ADDRESS,buf,2,MPU6050_TIMEOUT);
}

void MPU6050_WriteDatas(uint8_t reg,uint8_t data)
{
	HAL_I2C_Mem_Write(&hi2c1,MPU6050_ADDRESS,reg,
		I2C_MEMADD_SIZE_8BIT,&data,sizeof(data),MPU6050_TIMEOUT);
}

void MPU6050_WriteData1(uint8_t *Data, uint8_t Count)
{   
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDRESS,0x40,
        I2C_MEMADD_SIZE_8BIT,Data, Count, MPU6050_TIMEOUT);
}

HAL_StatusTypeDef MPU6050_ReadData(uint8_t reg , uint8_t *data)
{
	//告诉要读哪个寄存器
	HAL_I2C_Master_Transmit(&hi2c1,MPU6050_ADDRESS,&reg,1,MPU6050_TIMEOUT);
	//读出来的值放在data里
	return HAL_I2C_Master_Receive(&hi2c1,MPU6050_ADDRESS,data,1,MPU6050_TIMEOUT);
}

HAL_StatusTypeDef MPU6050_ReadDatas(uint8_t reg,uint8_t *data,uint8_t len)
{
	return HAL_I2C_Mem_Read(&hi2c1,MPU6050_ADDRESS,reg,
		I2C_MEMADD_SIZE_8BIT,data,len,MPU6050_TIMEOUT);
}





static float gz_bias = 0.0f;
static uint32_t last_ms;
static uint8_t time_valid = 0;

/* 启动时保持静止，采样约2秒计算Z轴零偏 */
static int MPU6050_CalibrateZ(void)
{
    uint8_t buf[2];
    float sum = 0.0f;

    vTaskDelay(pdMS_TO_TICKS(500));

    for (int i = 0; i < 200; i++)
    {
        if (MPU6050_ReadDatas(MPU6050_GYRO_ZOUT_H, buf, 2)
            != HAL_OK)
            return 1;

        sum += (int16_t)((buf[0] << 8) | buf[1]) / 16.4f;
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    gz_bias = sum / 200.0f;
    data.angle.yaw = 0.0f;
    time_valid = 0;
    return 0;
}






void MPU6050_Init()
{
	//1、初始化IIC接口
	hi2c1.Instance=I2C1;
	hi2c1.Init.ClockSpeed=400000;
	hi2c1.Init.DutyCycle=I2C_DUTYCYCLE_2;
	
	//2、设置复位
	MPU6050_WriteData(MPU6050_PWR_MGMT_1,0x80);
	mdelay(100);
	
	//3、唤醒
	MPU6050_WriteData(MPU6050_PWR_MGMT_1,0x00);
	vTaskDelay(pdMS_TO_TICKS(100));  // 新增：唤醒后等待稳定
	
	//4、设置采样率=陀螺仪输出频率/（SMPLRT_DIV+1）,此处设采样位200Hz，即5ms采样一次
	MPU6050_WriteData(MPU6050_SMPLRT_DIV,0x04);
	
	//5、设置加速度计量程和陀螺仪量程
	MPU6050_WriteData(MPU6050_ACCEL_CONFIG,0x00);
	MPU6050_WriteData(MPU6050_GYRO_CONFIG,0x18);
}

int MPU6050_GetID(void)
{
    uint8_t id;
	if(HAL_OK == MPU6050_ReadData(MPU6050_WHO_AM_I, &id))
        return id;
    else
        return -1;
}
//1--解析失败，0--成功
int MPU6050_ParseData()
{

	uint8_t buf[14];
	vTaskDelay(pdMS_TO_TICKS(5));
	float roll_a,pitch_a,yaw_g,roll_g,pitch_g;
	if(HAL_OK != MPU6050_ReadDatas(MPU6050_ACCEL_XOUT_H,buf,14))
		return 1;
	
	
	
	/* 新增：两次成功采样之间的时间，单位秒 */
	uint32_t now_ms = HAL_GetTick();
	float dt = time_valid
			 ? (uint32_t)(now_ms - last_ms) * 0.001f
			 : 0.0f;

	last_ms = now_ms;
	time_valid = 1;
	
	
	data.acc.ax=(int16_t)((buf[0]<<8)|buf[1])/16384.0f;
	data.acc.ay=(int16_t)((buf[2]<<8)|buf[3])/16384.0f;
	data.acc.az=(int16_t)((buf[4]<<8)|buf[5])/16384.0f;
	
	data.temp=(int16_t)((buf[6]<<8)|buf[7])/340.0f+36.53f;
	
	data.gyr.gx=(int16_t)((buf[8]<<8)|buf[9])/16.4f;
	data.gyr.gy=(int16_t)((buf[10]<<8)|buf[11])/16.4f;
//	data.gyr.gz=(int16_t)((buf[12]<<8)|buf[13])/16.4f;
	
	data.gyr.gz =(int16_t)((buf[12] << 8) | buf[13]) / 16.4f - gz_bias;
	
	roll_a=atan2(data.acc.ay,data.acc.az)/3.1415926*180;
	pitch_a=-atan2(data.acc.ax,data.acc.az)/3.1415926*180;
	
//	roll_g = data.angle.roll+ 0.005 * data.gyr.gx;
//	pitch_g = data.angle.pitch+ 0.005 * data.gyr.gy;
//	yaw_g = data.angle.yaw+  0.005 * data.gyr.gz;
//	
//	float a =0.1/(0.1+0.005);
//	
//	data.angle.yaw=yaw_g-0.003;
//	data.angle.roll=a*roll_g+(1-a)*roll_a;
//	data.angle.pitch=a*pitch_g+(1-a)*pitch_a;

	roll_g  = data.angle.roll  + dt * data.gyr.gx;
	pitch_g = data.angle.pitch + dt * data.gyr.gy;
	yaw_g   = data.angle.yaw   + dt * data.gyr.gz;

	float a = 0.1f / (0.1f + dt);

	data.angle.roll  = a * roll_g  + (1.0f - a) * roll_a;
	data.angle.pitch = a * pitch_g + (1.0f - a) * pitch_a;
	data.angle.yaw   = yaw_g;
	return 0;
}

struct mpu6050_data MPU6050_GetData()
{
	return data;
}

void MPU6050_Test(void)
{
    int id,ret;
	struct mpu6050_data result;

    MPU6050_Init();
	
	
    if (MPU6050_CalibrateZ() != 0)
		return;
	
	
    id = MPU6050_GetID();
    OLED_ShowString(0, 0, "MPU6050 ID:",OLED_8X16);
    OLED_ShowHexNum(13*8, 0, id, 3,OLED_8X16);
    OLED_Update();
    mdelay(1000);
	
	extern void GetI2C(void);
	extern void PutI2C(void);
    while (1)
    {    
		GetI2C();
		ret=MPU6050_ParseData();
		PutI2C();
		if(!ret)
		{
			result = MPU6050_GetData();
			OLED_ShowString(0, 16, "AngX:",OLED_8X16);
			OLED_ShowFloatNum(6*8,16,result.angle.roll,3,1,OLED_8X16);
			
			OLED_ShowString(0, 32, "AngY:",OLED_8X16);
			OLED_ShowFloatNum(6*8,32,result.angle.pitch,3,1,OLED_8X16);
			
			OLED_ShowString(0, 48, "AngZ:",OLED_8X16);
			OLED_ShowFloatNum(6*8,48,result.angle.yaw,3,1,OLED_8X16);
			
			
		}else OLED_ShowString(0, 48, "ERR:",OLED_8X16);
		OLED_Update();
		
		vTaskDelay(20);
//		vTaskDelay(pdMS_TO_TICKS(100));
//		mdelay(100);
//        OLED_Clear();
    }
}

void MPU6050_Task(void *params)
{	
    int16_t AccX;
	struct mpu6050_data result;
	int ret;
	//extern volatile int bInUsed;
	extern void GetI2C(void);
	extern void PutI2C(void);

    while (1)
    {    
		/* 等待事件:bit0 */
		xEventGroupWaitBits(g_xEventGroupMPU6050, (1<<0), pdTRUE, pdFALSE, portMAX_DELAY);		
		do 
		{
			/* 读数据 */
			GetI2C();
			ret = MPU6050_ParseData();
			PutI2C();
			if (0 == ret)
			{
				/* 解析数据 */
				result = MPU6050_GetData();

				/* 写队列 */
				xQueueSend(g_xQueueMPU6050, &result, 0);
				
				if (result.angle.roll != 90)
					vTaskDelay(50);
			}
		} while (!ret && (result.angle.roll != 90));
		
		/* delay */
		vTaskDelay(20);
	}
}

//void EXTI9_5_IRQHandler()
void MPU6050_Callback(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	
	/* 设置事件组: bit0 */
	xEventGroupSetBitsFromISR(g_xEventGroupMPU6050, (1<<0), &xHigherPriorityTaskWoken);
	
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}



///* 返回 HAL_OK 表示初始化写入全部成功 */
//HAL_StatusTypeDef MPU6050_Init1(void)
//{
//    HAL_StatusTypeDef status;
//    uint8_t value;

//    /* I2C 已由 MX_I2C1_Init() 初始化，这里不再修改 hi2c1 */

//    value = 0x80;  // 复位
//    status = HAL_I2C_Mem_Write(
//        &hi2c1, MPU6050_ADDRESS, MPU6050_PWR_MGMT_1,
//        I2C_MEMADD_SIZE_8BIT, &value, 1, 100);
//    if (status != HAL_OK)
//        return status;

//    vTaskDelay(pdMS_TO_TICKS(100));

//    value = 0x00;  // 解除休眠
//    status = HAL_I2C_Mem_Write(
//        &hi2c1, MPU6050_ADDRESS, MPU6050_PWR_MGMT_1,
//        I2C_MEMADD_SIZE_8BIT, &value, 1, 100);
//    if (status != HAL_OK)
//        return status;

//    vTaskDelay(pdMS_TO_TICKS(100));

//    value = 0x00;  // 启用全部加速度计、陀螺仪轴
//    status = HAL_I2C_Mem_Write(
//        &hi2c1, MPU6050_ADDRESS, MPU6050_PWR_MGMT_2,
//        I2C_MEMADD_SIZE_8BIT, &value, 1, 100);
//    if (status != HAL_OK)
//        return status;

//    value = 0x00;  // 加速度计 ±2g，对应 16384 LSB/g
//    status = HAL_I2C_Mem_Write(
//        &hi2c1, MPU6050_ADDRESS, MPU6050_ACCEL_CONFIG,
//        I2C_MEMADD_SIZE_8BIT, &value, 1, 100);
//    if (status != HAL_OK)
//        return status;

//    value = 0x18;  // 陀螺仪 ±2000°/s，对应 16.4 LSB/(°/s)
//    return HAL_I2C_Mem_Write(
//        &hi2c1, MPU6050_ADDRESS, MPU6050_GYRO_CONFIG,
//        I2C_MEMADD_SIZE_8BIT, &value, 1, 100);
//}

//void MPU6050_Test1(void)
//{
//    int id, ret;
//    struct mpu6050_data result;

//    /* 替换原来的 MPU6050_Init(); */
//    if (MPU6050_Init1() != HAL_OK)
//    {
//        OLED_Clear();
//        OLED_ShowString(0, 0, "MPU INIT ERROR", OLED_8X16);
//        OLED_Update();

//        for (;;)
//        {
//            vTaskDelay(pdMS_TO_TICKS(1000));
//        }
//    }

//    /* 初始化成功，继续执行 */
//    id = MPU6050_GetID();

//    OLED_Clear();
//    OLED_ShowString(0, 0, "MPU6050 ID:", OLED_8X16);
//    OLED_ShowHexNum(104, 0, id, 2, OLED_8X16);
//    OLED_Update();

//    while (1)
//    {
//        /* 保留你的采集、解析和角度显示代码 */
//		GetI2C();
//		ret=MPU6050_ParseData();
//		PutI2C();
//		if(!ret)
//		{
//			result = MPU6050_GetData();
//			OLED_ShowString(0, 16, "AngX:",OLED_8X16);
//			OLED_ShowFloatNum(6*8,16,result.angle.roll,3,1,OLED_8X16);
//			
//			OLED_ShowString(0, 32, "AngY:",OLED_8X16);
//			OLED_ShowFloatNum(6*8,32,result.angle.pitch,3,1,OLED_8X16);
//			
//			OLED_ShowString(0, 48, "AngZ:",OLED_8X16);
//			OLED_ShowFloatNum(6*8,48,result.angle.yaw,3,1,OLED_8X16);
//			
//			
//		}else OLED_ShowString(0, 48, "ERR:",OLED_8X16);
//		OLED_Update();
//		
//		vTaskDelay(20);
//    }
//}


