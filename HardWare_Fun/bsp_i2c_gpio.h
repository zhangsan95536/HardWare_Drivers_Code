#ifndef _BSP_I2C_GPIO_H
#define _BSP_I2C_GPIO_H


#include <inttypes.h>



#define I2C_ADDR                    0x1e   //语音识别模块地址,模块地址为0x0f，由于最右边一位要做读写位，所以右移一位为0x1e

#define ASR_ADD_WORD_ADDR           0x01   //词条添加地址

#define ASR_MODE_ADDR               0x02   //识别模式设置地址，值为0-2，0:循环识别模式 1:口令模式 ,2:按键模式，默认为循环检测

#define ASR_RGB_ADDR                0x03   //RGB灯设置地址,需要发两位，第一个直接为灯号1：蓝 2:红 3：绿 ,
                                           //第二个字节为亮度0-255，数值越大亮度越高
                                           
#define ASR_REC_GAIN                0x04   //识别灵敏度设置地址，灵敏度可设置为0x00-0x55，值越高越容易检测但是越容易误判，
                                           //建议设置值为0x40-0x55,默认值为0x40
                                           
#define ASR_CLEAR_ADDR              0x05   //清除掉电缓存操作地址，录入信息前均要清除下缓存区信息


#define ASR_KEY_FLAG                0x06   //用于按键模式下，设置启动识别模式

#define ASR_VOICE_FLAG              0x07   //用于设置是否开启识别结果提示音

#define ASR_RESULT                  0x08  //识别结果存放地址

#define ASR_BUZZER                  0x09 //蜂鸣器控制写1开启，写0关闭

#define ASR_NUM_CLECK               0x0a //录入词条数目校验

#define FIRMWARE_VERSION            0x0b //读取固件版本

#define ASR_BUSY                    0x0c//忙闲标志

#define macI2C_WR	0		/* 写控制bit */
#define macI2C_RD	1		/* 读控制bit */


/* 定义I2C总线连接的GPIO端口, 用户只需要修改下面4行代码即可任意改变SCL和SDA的引脚 */
#define macGPIO_PORT_I2C	GPIOA			/* GPIO端口 */
#define macRCC_I2C_PORT 	RCC_APB2Periph_GPIOA		/* GPIO端口时钟 */
#define macI2C_SCL_PIN		GPIO_Pin_8			/* 连接到SCL时钟线的GPIO */
#define macI2C_SDA_PIN		GPIO_Pin_9			/* 连接到SDA数据线的GPIO */


/* 定义读写SCL和SDA的宏，已增加代码的可移植性和可阅读性 */
#if 0	/* 条件编译： 1 选择GPIO的库函数实现IO读写 */
	#define macI2C_SCL_1()  GPIO_SetBits(macGPIO_PORT_I2C, macI2C_SCL_PIN)		/* SCL = 1 */
	#define macI2C_SCL_0()  GPIO_ResetBits(macGPIO_PORT_I2C, macI2C_SCL_PIN)		/* SCL = 0 */
	
	#define macI2C_SDA_1()  GPIO_SetBits(macGPIO_PORT_I2C, macI2C_SDA_PIN)		/* SDA = 1 */
	#define macI2C_SDA_0()  GPIO_ResetBits(macGPIO_PORT_I2C, macI2C_SDA_PIN)		/* SDA = 0 */
	
	#define macI2C_SDA_READ()  GPIO_ReadInputDataBit(macGPIO_PORT_I2C, macI2C_SDA_PIN)	/* 读SDA口线状态 */
#else	/* 这个分支选择直接寄存器操作实现IO读写 */
    /*　注意：如下写法，在IAR最高级别优化时，会被编译器错误优化 */
	#define macI2C_SCL_1()  macGPIO_PORT_I2C->BSRR = macI2C_SCL_PIN				/* SCL = 1 */
	#define macI2C_SCL_0()  macGPIO_PORT_I2C->BRR = macI2C_SCL_PIN				/* SCL = 0 */
	
	#define macI2C_SDA_1()  macGPIO_PORT_I2C->BSRR = macI2C_SDA_PIN				/* SDA = 1 */
	#define macI2C_SDA_0()  macGPIO_PORT_I2C->BRR = macI2C_SDA_PIN				/* SDA = 0 */
	
	#define macI2C_SDA_READ()  ((macGPIO_PORT_I2C->IDR & macI2C_SDA_PIN) != 0)	/* 读SDA口线状态 */
	#define macI2C_SCL_READ()  ((macGPIO_PORT_I2C->IDR & macI2C_SCL_PIN) != 0)	/* 读SDA口线状态 */
#endif


void Start_I2c(void);
void Stop_I2c(void);
void  I2C_SendByte(unsigned char  c);
unsigned char I2C_RcvByte(void);
void i2c_CfgGpio(void);
void Ack_I2c(uint8_t a);

uint8_t I2C_ByteWrite_alone(unsigned char date);
void I2C_ByteWrite(uint8_t reg_addr,uint8_t dat);
void AsrAddWords(uint8_t idNum,uint8_t * words);
void RGB_Set(unsigned char R,unsigned char G,unsigned char B);
unsigned char I2C_BufferRead(unsigned char date);

void BusyWait(void);
void delay(int z);

#endif

