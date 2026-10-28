
/***************************************************************************************
  * 本程序由江协科技创建并免费开源共享
  * 你可以任意查看、使用和修改，并应用到自己的项目之中
  * 程序版权归江协科技所有，任何人或组织不得将其据为己有
  * 
  * 程序名称：				0.96寸OLED显示屏驱动程序（7针脚SPI接口）
  * 程序创建时间：			2023.10.24
  * 当前程序版本：			V2.0
  * 当前版本发布时间：		2024.10.20
  * 
  * 江协科技官方网站：		jiangxiekeji.com
  * 江协科技官方淘宝店：	jiangxiekeji.taobao.com
  * 程序介绍及更新动态：	jiangxiekeji.com/tutorial/oled.html
  * 
  * 如果你发现程序中的漏洞或者笔误，可通过邮件向我们反馈：feedback@jiangxiekeji.com
  * 发送邮件之前，你可以先到更新动态页面查看最新程序，如果此问题已经修改，则无需再发邮件
  ***************************************************************************************
  */

#include "stm32f10x.h"
#include "OLED.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

/**
  * 数据存储格式：
  * 纵向8点，高位在下，先从左到右，再从上到下
  * 每一个Bit对应一个像素点
  * 
  *      B0 B0                  B0 B0
  *      B1 B1                  B1 B1
  *      B2 B2                  B2 B2
  *      B3 B3  ------------->  B3 B3 --
  *      B4 B4                  B4 B4  |
  *      B5 B5                  B5 B5  |
  *      B6 B6                  B6 B6  |
  *      B7 B7                  B7 B7  |
  *                                    |
  *  -----------------------------------
  *  |   
  *  |   B0 B0                  B0 B0
  *  |   B1 B1                  B1 B1
  *  |   B2 B2                  B2 B2
  *  --> B3 B3  ------------->  B3 B3
  *      B4 B4                  B4 B4
  *      B5 B5                  B5 B5
  *      B6 B6                  B6 B6
  *      B7 B7                  B7 B7
  * 
  * 坐标轴定义：
  * 左上角为(0, 0)点
  * 横向向右为X轴，取值范围：0~127
  * 纵向向下为Y轴，取值范围：0~63
  * 
  *       0             X轴           127 
  *      .------------------------------->
  *    0 |
  *      |
  *      |
  *      |
  *  Y轴 |
  *      |
  *      |
  *      |
  *   63 |
  *      v
  * 
  */


/*全局变量*********************/

/**
  * OLED显存数组
  * 所有的显示函数，都只是对此显存数组进行读写
  * 随后调用OLED_Update函数或OLED_UpdateArea函数
  * 才会将显存数组的数据发送到OLED硬件，进行显示
  */
uint8_t OLED_DisplayBuf[8][128];

/*********************全局变量*/


/*引脚配置*********************/

/**
  * 函    数：OLED写D0（CLK）高低电平
  * 参    数：要写入D0的电平值，范围：0/1
  * 返 回 值：无
  * 说    明：当上层函数需要写D0时，此函数会被调用
  *           用户需要根据参数传入的值，将D0置为高电平或者低电平
  *           当参数传入0时，置D0为低电平，当参数传入1时，置D0为高电平
  */
void OLED_W_D0(uint8_t BitValue)
{
	/*根据BitValue的值，将D0置高电平或者低电平*/
	GPIO_WriteBit(GPIOB, GPIO_Pin_12, (BitAction)BitValue);
}

/**
  * 函    数：OLED写D1（MOSI）高低电平
  * 参    数：要写入D1的电平值，范围：0/1
  * 返 回 值：无
  * 说    明：当上层函数需要写D1时，此函数会被调用
  *           用户需要根据参数传入的值，将D1置为高电平或者低电平
  *           当参数传入0时，置D1为低电平，当参数传入1时，置D1为高电平
  */
void OLED_W_D1(uint8_t BitValue)
{
	/*根据BitValue的值，将D1置高电平或者低电平*/
	GPIO_WriteBit(GPIOB, GPIO_Pin_13, (BitAction)BitValue);
}

/**
  * 函    数：OLED写RES高低电平
  * 参    数：要写入RES的电平值，范围：0/1
  * 返 回 值：无
  * 说    明：当上层函数需要写RES时，此函数会被调用
  *           用户需要根据参数传入的值，将RES置为高电平或者低电平
  *           当参数传入0时，置RES为低电平，当参数传入1时，置RES为高电平
  */
void OLED_W_RES(uint8_t BitValue)
{
	/*根据BitValue的值，将RES置高电平或者低电平*/
	GPIO_WriteBit(GPIOB, GPIO_Pin_14, (BitAction)BitValue);
}

/**
  * 函    数：OLED写DC高低电平
  * 参    数：要写入DC的电平值，范围：0/1
  * 返 回 值：无
  * 说    明：当上层函数需要写DC时，此函数会被调用
  *           用户需要根据参数传入的值，将DC置为高电平或者低电平
  *           当参数传入0时，置DC为低电平，当参数传入1时，置DC为高电平
  */
void OLED_W_DC(uint8_t BitValue)
{
	/*根据BitValue的值，将DC置高电平或者低电平*/
	GPIO_WriteBit(GPIOB, GPIO_Pin_15, (BitAction)BitValue);
}

/**
  * 函    数：OLED写CS高低电平
  * 参    数：要写入CS的电平值，范围：0/1
  * 返 回 值：无
  * 说    明：当上层函数需要写CS时，此函数会被调用
  *           用户需要根据参数传入的值，将CS置为高电平或者低电平
  *           当参数传入0时，置CS为低电平，当参数传入1时，置CS为高电平
  */
void OLED_W_CS(uint8_t BitValue)
{
	/*根据BitValue的值，将CS置高电平或者低电平*/
	GPIO_WriteBit(GPIOA, GPIO_Pin_8, (BitAction)BitValue);
}

/**
  * 函    数：OLED引脚初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：当上层函数需要初始化时，此函数会被调用
  *           用户需要将D0、D1、RES、DC和CS引脚初始化为推挽输出模式
  */
void OLED_GPIO_Init(void)
{
	uint32_t i, j;
	
	/*在初始化前，加入适量延时，待OLED供电稳定*/
	for (i = 0; i < 1000; i ++)
	{
		for (j = 0; j < 1000; j ++);
	}
	
	/*将D0、D1、RES、DC和CS引脚初始化为推挽输出模式*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/*置引脚默认电平*/
	OLED_W_D0(0);
	OLED_W_D1(1);
	OLED_W_RES(1);
	OLED_W_DC(1);
	OLED_W_CS(1);
}

/*********************引脚配置*/


/*通信协议*********************/

/**
  * 函    数：SPI发送一个字节
  * 参    数：Byte 要发送的一个字节数据，范围：0x00~0xFF
  * 返 回 值：无
  */
void OLED_SPI_SendByte(uint8_t Byte)
{
	uint8_t i;
	
	/*循环8次，主机依次发送数据的每一位*/
	for (i = 0; i < 8; i++)
	{
		/*使用掩码的方式取出Byte的指定一位数据并写入到D1线*/
		/*两个!的作用是，让所有非零的值变为1*/
		OLED_W_D1(!!(Byte & (0x80 >> i)));
		OLED_W_D0(1);	//拉高D0，从机在D0上升沿读取SDA
		OLED_W_D0(0);	//拉低D0，主机开始发送下一位数据
	}
}

/**
  * 函    数：OLED写命令
  * 参    数：Command 要写入的命令值，范围：0x00~0xFF
  * 返 回 值：无
  */
void OLED_WriteCommand(uint8_t Command)
{
	OLED_W_CS(0);					//拉低CS，开始通信
	OLED_W_DC(0);					//拉低DC，表示即将发送命令
	OLED_SPI_SendByte(Command);		//写入指定命令
	OLED_W_CS(1);					//拉高CS，结束通信
}

/**
  * 函    数：OLED写数据
  * 参    数：Data 要写入数据的起始地址
  * 参    数：Count 要写入数据的数量
  * 返 回 值：无
  */
void OLED_WriteData(uint8_t *Data, uint8_t Count)
{
	uint8_t i;
	
	OLED_W_CS(0);					//拉低CS，开始通信
	OLED_W_DC(1);					//拉高DC，表示即将发送数据
	/*循环Count次，进行连续的数据写入*/
	for (i = 0; i < Count; i ++)
	{
		OLED_SPI_SendByte(Data[i]);	//依次发送Data的每一个数据
	}
	OLED_W_CS(1);					//拉高CS，结束通信
}

/*********************通信协议*/


/*硬件配置*********************/

/**
  * 函    数：OLED初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：使用前，需要调用此初始化函数
  */
void OLED_Init(void)
{
	OLED_GPIO_Init();			//先调用底层的端口初始化
	
	/*写入一系列的命令，对OLED进行初始化配置*/
	OLED_WriteCommand(0xAE);	//设置显示开启/关闭，0xAE关闭，0xAF开启
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);	//0x00~0xFF
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);	//0x0E~0x3F
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);	//0x00~0x7F
	
	OLED_WriteCommand(0x40);	//设置显示开始行，0x40~0x7F
	
	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常，0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常，0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度
	OLED_WriteCommand(0xCF);	//0x00~0xFF

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/反色显示，0xA6正常，0xA7反色

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开启显示
	
	OLED_Clear();				//清空显存数组
	OLED_Update();				//更新显示，清屏，防止初始化后未显示内容时花屏
}

/**
  * 函    数：OLED设置显示光标位置
  * 参    数：Page 指定光标所在的页，范围：0~7
  * 参    数：X 指定光标所在的X轴坐标，范围：0~127
  * 返 回 值：无
  * 说    明：OLED默认的Y轴，只能8个Bit为一组写入，即1页等于8个Y轴坐标
  */
void OLED_SetCursor(uint8_t Page, uint8_t X)
{
	/*如果使用此程序驱动1.3寸的OLED显示屏，则需要解除此注释*/
	/*因为1.3寸的OLED驱动芯片（SH1106）有132列*/
	/*屏幕的起始列接在了第2列，而不是第0列*/
	/*所以需要将X加2，才能正常显示*/
//	X += 2;
	
	/*通过指令设置页地址和列地址*/
	OLED_WriteCommand(0xB0 | Page);					//设置页位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}

/*********************硬件配置*/


/*工具函数*********************/

/*工具函数仅供内部部分函数使用*/

/**
  * 函    数：次方函数
  * 参    数：X 底数
  * 参    数：Y 指数
  * 返 回 值：等于X的Y次方
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//结果默认为1
	while (Y --)			//累乘Y次
	{
		Result *= X;		//每次把X累乘到结果上
	}
	return Result;
}

/**
  * 函    数：判断指定点是否在指定多边形内部
  * 参    数：nvert 多边形的顶点数
  * 参    数：vertx verty 包含多边形顶点的x和y坐标的数组
  * 参    数：testx testy 测试点的X和y坐标
  * 返 回 值：指定点是否在指定多边形内部，1：在内部，0：不在内部
  */
uint8_t OLED_pnpoly(uint8_t nvert, int16_t *vertx, int16_t *verty, int16_t testx, int16_t testy)
{
	int16_t i, j, c = 0;
	
	/*此算法由W. Randolph Franklin提出*/
	/*参考链接：https://wrfranklin.org/Research/Short_Notes/pnpoly.html*/
	for (i = 0, j = nvert - 1; i < nvert; j = i++)
	{
		if (((verty[i] > testy) != (verty[j] > testy)) &&
			(testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
		{
			c = !c;
		}
	}
	return c;
}

/**
  * 函    数：判断指定点是否在指定角度内部
  * 参    数：X Y 指定点的坐标
  * 参    数：StartAngle EndAngle 起始角度和终止角度，范围：-180~180
  *           水平向右为0度，水平向左为180度或-180度，下方为正数，上方为负数，顺时针旋转
  * 返 回 值：指定点是否在指定角度内部，1：在内部，0：不在内部
  */
uint8_t OLED_IsInAngle(int16_t X, int16_t Y, int16_t StartAngle, int16_t EndAngle)
{
	int16_t PointAngle;
	PointAngle = atan2(Y, X) / 3.14 * 180;	//计算指定点的弧度，并转换为角度表示
	if (StartAngle < EndAngle)	//起始角度小于终止角度的情况
	{
		/*如果指定角度在起始终止角度之间，则判定指定点在指定角度*/
		if (PointAngle >= StartAngle && PointAngle <= EndAngle)
		{
			return 1;
		}
	}
	else			//起始角度大于于终止角度的情况
	{
		/*如果指定角度大于起始角度或者小于终止角度，则判定指定点在指定角度*/
		if (PointAngle >= StartAngle || PointAngle <= EndAngle)
		{
			return 1;
		}
	}
	return 0;		//不满足以上条件，则判断判定指定点不在指定角度
}

/*********************工具函数*/


/*功能函数*********************/

/**
  * 函    数：将OLED显存数组更新到OLED屏幕
  * 参    数：无
  * 返 回 值：无
  * 说    明：所有的显示函数，都只是对OLED显存数组进行读写
  *           随后调用OLED_Update函数或OLED_UpdateArea函数
  *           才会将显存数组的数据发送到OLED硬件，进行显示
  *           故调用显示函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_Update(void)
{
	uint8_t j;
	/*遍历每一页*/
	for (j = 0; j < 8; j ++)
	{
		/*设置光标位置为每一页的第一列*/
		OLED_SetCursor(j, 0);
		/*连续写入128个数据，将显存数组的数据写入到OLED硬件*/
		OLED_WriteData(OLED_DisplayBuf[j], 128);
	}
}

/**
  * 函    数：将OLED显存数组部分更新到OLED屏幕
  * 参    数：X 指定区域左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定区域左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Width 指定区域的宽度，范围：0~128
  * 参    数：Height 指定区域的高度，范围：0~64
  * 返 回 值：无
  * 说    明：此函数会至少更新参数指定的区域
  *           如果更新区域Y轴只包含部分页，则同一页的剩余部分会跟随一起更新
  * 说    明：所有的显示函数，都只是对OLED显存数组进行读写
  *           随后调用OLED_Update函数或OLED_UpdateArea函数
  *           才会将显存数组的数据发送到OLED硬件，进行显示
  *           故调用显示函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
	int16_t j;
	int16_t Page, Page1;
	
	/*负数坐标在计算页地址时需要加一个偏移*/
	/*(Y + Height - 1) / 8 + 1的目的是(Y + Height) / 8并向上取整*/
	Page = Y / 8;
	Page1 = (Y + Height - 1) / 8 + 1;
	if (Y < 0)
	{
		Page -= 1;
		Page1 -= 1;
	}
	
	/*遍历指定区域涉及的相关页*/
	for (j = Page; j < Page1; j ++)
	{
		if (X >= 0 && X <= 127 && j >= 0 && j <= 7)		//超出屏幕的内容不显示
		{
			/*设置光标位置为相关页的指定列*/
			OLED_SetCursor(j, X);
			/*连续写入Width个数据，将显存数组的数据写入到OLED硬件*/
			OLED_WriteData(&OLED_DisplayBuf[j][X], Width);
		}
	}
}

/**
  * 函    数：将OLED显存数组全部清零
  * 参    数：无
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_Clear(void)
{
	uint8_t i, j;
	for (j = 0; j < 8; j ++)				//遍历8页
	{
		for (i = 0; i < 128; i ++)			//遍历128列
		{
			OLED_DisplayBuf[j][i] = 0x00;	//将显存数组数据全部清零
		}
	}
}

/**
  * 函    数：将OLED显存数组部分清零
  * 参    数：X 指定区域左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定区域左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Width 指定区域的宽度，范围：0~128
  * 参    数：Height 指定区域的高度，范围：0~64
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
	int16_t i, j;
	
	for (j = Y; j < Y + Height; j ++)		//遍历指定页
	{
		for (i = X; i < X + Width; i ++)	//遍历指定列
		{
			if (i >= 0 && i <= 127 && j >=0 && j <= 63)				//超出屏幕的内容不显示
			{
				OLED_DisplayBuf[j / 8][i] &= ~(0x01 << (j % 8));	//将显存数组指定数据清零
			}
		}
	}
}

/**
  * 函    数：将OLED显存数组全部取反
  * 参    数：无
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_Reverse(void)
{
	uint8_t i, j;
	for (j = 0; j < 8; j ++)				//遍历8页
	{
		for (i = 0; i < 128; i ++)			//遍历128列
		{
			OLED_DisplayBuf[j][i] ^= 0xFF;	//将显存数组数据全部取反
		}
	}
}
	
/**
  * 函    数：将OLED显存数组部分取反
  * 参    数：X 指定区域左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定区域左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Width 指定区域的宽度，范围：0~128
  * 参    数：Height 指定区域的高度，范围：0~64
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
	int16_t i, j;
	
	for (j = Y; j < Y + Height; j ++)		//遍历指定页
	{
		for (i = X; i < X + Width; i ++)	//遍历指定列
		{
			if (i >= 0 && i <= 127 && j >=0 && j <= 63)			//超出屏幕的内容不显示
			{
				OLED_DisplayBuf[j / 8][i] ^= 0x01 << (j % 8);	//将显存数组指定数据取反
			}
		}
	}
}

/**
  * 函    数：OLED显示一个字符
  * 参    数：X 指定字符左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定字符左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Char 指定要显示的字符，范围：ASCII码可见字符
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize)
{
	if (FontSize == OLED_8X16)		//字体为宽8像素，高16像素
	{
		/*将ASCII字模库OLED_F8x16的指定数据以8*16的图像格式显示*/
		OLED_ShowImage(X, Y, 8, 16, OLED_F8x16[Char - ' ']);
	}
	else if(FontSize == OLED_6X8)	//字体为宽6像素，高8像素
	{
		/*将ASCII字模库OLED_F6x8的指定数据以6*8的图像格式显示*/
		OLED_ShowImage(X, Y, 6, 8, OLED_F6x8[Char - ' ']);
	}
}

/**
  * 函    数：OLED显示字符串（支持ASCII码和中文混合写入）
  * 参    数：X 指定字符串左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定字符串左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：String 指定要显示的字符串，范围：ASCII码可见字符或中文字符组成的字符串
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 返 回 值：无
  * 说    明：显示的中文字符需要在OLED_Data.c里的OLED_CF16x16数组定义
  *           未找到指定中文字符时，会显示默认图形（一个方框，内部一个问号）
  *           当字体大小为OLED_8X16时，中文字符以16*16点阵正常显示
  *           当字体大小为OLED_6X8时，中文字符以6*8点阵显示'?'
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize)
{
	uint16_t i = 0;
	char SingleChar[5];
	uint8_t CharLength = 0;
	uint16_t XOffset = 0;
	uint16_t pIndex;
	
	while (String[i] != '\0')	//遍历字符串
	{
		
#ifdef OLED_CHARSET_UTF8						//定义字符集为UTF8
		/*此段代码的目的是，提取UTF8字符串中的一个字符，转存到SingleChar子字符串中*/
		/*判断UTF8编码第一个字节的标志位*/
		if ((String[i] & 0x80) == 0x00)			//第一个字节为0xxxxxxx
		{
			CharLength = 1;						//字符为1字节
			SingleChar[0] = String[i ++];		//将第一个字节写入SingleChar第0个位置，随后i指向下一个字节
			SingleChar[1] = '\0';				//为SingleChar添加字符串结束标志位
		}
		else if ((String[i] & 0xE0) == 0xC0)	//第一个字节为110xxxxx
		{
			CharLength = 2;						//字符为2字节
			SingleChar[0] = String[i ++];		//将第一个字节写入SingleChar第0个位置，随后i指向下一个字节
			if (String[i] == '\0') {break;}		//意外情况，跳出循环，结束显示
			SingleChar[1] = String[i ++];		//将第二个字节写入SingleChar第1个位置，随后i指向下一个字节
			SingleChar[2] = '\0';				//为SingleChar添加字符串结束标志位
		}
		else if ((String[i] & 0xF0) == 0xE0)	//第一个字节为1110xxxx
		{
			CharLength = 3;						//字符为3字节
			SingleChar[0] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[1] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[2] = String[i ++];
			SingleChar[3] = '\0';
		}
		else if ((String[i] & 0xF8) == 0xF0)	//第一个字节为11110xxx
		{
			CharLength = 4;						//字符为4字节
			SingleChar[0] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[1] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[2] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[3] = String[i ++];
			SingleChar[4] = '\0';
		}
		else
		{
			i ++;			//意外情况，i指向下一个字节，忽略此字节，继续判断下一个字节
			continue;
		}
#endif
		
#ifdef OLED_CHARSET_GB2312						//定义字符集为GB2312
		/*此段代码的目的是，提取GB2312字符串中的一个字符，转存到SingleChar子字符串中*/
		/*判断GB2312字节的最高位标志位*/
		if ((String[i] & 0x80) == 0x00)			//最高位为0
		{
			CharLength = 1;						//字符为1字节
			SingleChar[0] = String[i ++];		//将第一个字节写入SingleChar第0个位置，随后i指向下一个字节
			SingleChar[1] = '\0';				//为SingleChar添加字符串结束标志位
		}
		else									//最高位为1
		{
			CharLength = 2;						//字符为2字节
			SingleChar[0] = String[i ++];		//将第一个字节写入SingleChar第0个位置，随后i指向下一个字节
			if (String[i] == '\0') {break;}		//意外情况，跳出循环，结束显示
			SingleChar[1] = String[i ++];		//将第二个字节写入SingleChar第1个位置，随后i指向下一个字节
			SingleChar[2] = '\0';				//为SingleChar添加字符串结束标志位
		}
#endif
		
		/*显示上述代码提取到的SingleChar*/
		if (CharLength == 1)	//如果是单字节字符
		{
			/*使用OLED_ShowChar显示此字符*/
			OLED_ShowChar(X + XOffset, Y, SingleChar[0], FontSize);
			XOffset += FontSize;
		}
		else					//否则，即多字节字符
		{
			/*遍历整个字模库，从字模库中寻找此字符的数据*/
			/*如果找到最后一个字符（定义为空字符串），则表示字符未在字模库定义，停止寻找*/
			for (pIndex = 0; strcmp(OLED_CF16x16[pIndex].Index, "") != 0; pIndex ++)
			{
				/*找到匹配的字符*/
				if (strcmp(OLED_CF16x16[pIndex].Index, SingleChar) == 0)
				{
					break;		//跳出循环，此时pIndex的值为指定字符的索引
				}
			}
			if (FontSize == OLED_8X16)		//给定字体为8*16点阵
			{
				/*将字模库OLED_CF16x16的指定数据以16*16的图像格式显示*/
				OLED_ShowImage(X + XOffset, Y, 16, 16, OLED_CF16x16[pIndex].Data);
				XOffset += 16;
			}
			else if (FontSize == OLED_6X8)	//给定字体为6*8点阵
			{
				/*空间不足，此位置显示'?'*/
				OLED_ShowChar(X + XOffset, Y, '?', OLED_6X8);
				XOffset += OLED_6X8;
			}
		}
	}
}

/**
  * 函    数：OLED显示数字（十进制，正整数）
  * 参    数：X 指定数字左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定数字左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Number 指定要显示的数字，范围：0~4294967295
  * 参    数：Length 指定数字的长度，范围：0~10
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i;
	for (i = 0; i < Length; i++)		//遍历数字的每一位							
	{
		/*调用OLED_ShowChar函数，依次显示每个数字*/
		/*Number / OLED_Pow(10, Length - i - 1) % 10 可以十进制提取数字的每一位*/
		/*+ '0' 可将数字转换为字符格式*/
		OLED_ShowChar(X + i * FontSize, Y, Number / OLED_Pow(10, Length - i - 1) % 10 + '0', FontSize);
	}
}

/**
  * 函    数：OLED显示有符号数字（十进制，整数）
  * 参    数：X 指定数字左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定数字左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Number 指定要显示的数字，范围：-2147483648~2147483647
  * 参    数：Length 指定数字的长度，范围：0~10
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i;
	uint32_t Number1;
	
	if (Number >= 0)						//数字大于等于0
	{
		OLED_ShowChar(X, Y, '+', FontSize);	//显示+号
		Number1 = Number;					//Number1直接等于Number
	}
	else									//数字小于0
	{
		OLED_ShowChar(X, Y, '-', FontSize);	//显示-号
		Number1 = -Number;					//Number1等于Number取负
	}
	
	for (i = 0; i < Length; i++)			//遍历数字的每一位								
	{
		/*调用OLED_ShowChar函数，依次显示每个数字*/
		/*Number1 / OLED_Pow(10, Length - i - 1) % 10 可以十进制提取数字的每一位*/
		/*+ '0' 可将数字转换为字符格式*/
		OLED_ShowChar(X + (i + 1) * FontSize, Y, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0', FontSize);
	}
}

/**
  * 函    数：OLED显示十六进制数字（十六进制，正整数）
  * 参    数：X 指定数字左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定数字左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Number 指定要显示的数字，范围：0x00000000~0xFFFFFFFF
  * 参    数：Length 指定数字的长度，范围：0~8
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)		//遍历数字的每一位
	{
		/*以十六进制提取数字的每一位*/
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		
		if (SingleNumber < 10)			//单个数字小于10
		{
			/*调用OLED_ShowChar函数，显示此数字*/
			/*+ '0' 可将数字转换为字符格式*/
			OLED_ShowChar(X + i * FontSize, Y, SingleNumber + '0', FontSize);
		}
		else							//单个数字大于10
		{
			/*调用OLED_ShowChar函数，显示此数字*/
			/*+ 'A' 可将数字转换为从A开始的十六进制字符*/
			OLED_ShowChar(X + i * FontSize, Y, SingleNumber - 10 + 'A', FontSize);
		}
	}
}

/**
  * 函    数：OLED显示二进制数字（二进制，正整数）
  * 参    数：X 指定数字左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定数字左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Number 指定要显示的数字，范围：0x00000000~0xFFFFFFFF
  * 参    数：Length 指定数字的长度，范围：0~16
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i;
	for (i = 0; i < Length; i++)		//遍历数字的每一位	
	{
		/*调用OLED_ShowChar函数，依次显示每个数字*/
		/*Number / OLED_Pow(2, Length - i - 1) % 2 可以二进制提取数字的每一位*/
		/*+ '0' 可将数字转换为字符格式*/
		OLED_ShowChar(X + i * FontSize, Y, Number / OLED_Pow(2, Length - i - 1) % 2 + '0', FontSize);
	}
}

/**
  * 函    数：OLED显示浮点数字（十进制，小数）
  * 参    数：X 指定数字左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定数字左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Number 指定要显示的数字，范围：-4294967295.0~4294967295.0
  * 参    数：IntLength 指定数字的整数位长度，范围：0~10
  * 参    数：FraLength 指定数字的小数位长度，范围：0~9，小数进行四舍五入显示
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowFloatNum(int16_t X, int16_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize)
{
	uint32_t PowNum, IntNum, FraNum;
	
	if (Number >= 0)						//数字大于等于0
	{
		OLED_ShowChar(X, Y, '+', FontSize);	//显示+号
	}
	else									//数字小于0
	{
		OLED_ShowChar(X, Y, '-', FontSize);	//显示-号
		Number = -Number;					//Number取负
	}
	
	/*提取整数部分和小数部分*/
	IntNum = Number;						//直接赋值给整型变量，提取整数
	Number -= IntNum;						//将Number的整数减掉，防止之后将小数乘到整数时因数过大造成错误
	PowNum = OLED_Pow(10, FraLength);		//根据指定小数的位数，确定乘数
	FraNum = round(Number * PowNum);		//将小数乘到整数，同时四舍五入，避免显示误差
	IntNum += FraNum / PowNum;				//若四舍五入造成了进位，则需要再加给整数
	
	/*显示整数部分*/
	OLED_ShowNum(X + FontSize, Y, IntNum, IntLength, FontSize);
	
	/*显示小数点*/
	OLED_ShowChar(X + (IntLength + 1) * FontSize, Y, '.', FontSize);
	
	/*显示小数部分*/
	OLED_ShowNum(X + (IntLength + 2) * FontSize, Y, FraNum, FraLength, FontSize);
}

/**
  * 函    数：OLED显示图像
  * 参    数：X 指定图像左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定图像左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Width 指定图像的宽度，范围：0~128
  * 参    数：Height 指定图像的高度，范围：0~64
  * 参    数：Image 指定要显示的图像
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
	uint8_t i = 0, j = 0;
	int16_t Page, Shift;
	
	/*将图像所在区域清空*/
	OLED_ClearArea(X, Y, Width, Height);
	
	/*遍历指定图像涉及的相关页*/
	/*(Height - 1) / 8 + 1的目的是Height / 8并向上取整*/
	for (j = 0; j < (Height - 1) / 8 + 1; j ++)
	{
		/*遍历指定图像涉及的相关列*/
		for (i = 0; i < Width; i ++)
		{
			if (X + i >= 0 && X + i <= 127)		//超出屏幕的内容不显示
			{
				/*负数坐标在计算页地址和移位时需要加一个偏移*/
				Page = Y / 8;
				Shift = Y % 8;
				if (Y < 0)
				{
					Page -= 1;
					Shift += 8;
				}
				
				if (Page + j >= 0 && Page + j <= 7)		//超出屏幕的内容不显示
				{
					/*显示图像在当前页的内容*/
					OLED_DisplayBuf[Page + j][X + i] |= Image[j * Width + i] << (Shift);
				}
				
				if (Page + j + 1 >= 0 && Page + j + 1 <= 7)		//超出屏幕的内容不显示
				{					
					/*显示图像在下一页的内容*/
					OLED_DisplayBuf[Page + j + 1][X + i] |= Image[j * Width + i] >> (8 - Shift);
				}
			}
		}
	}
}

/**
  * 函    数：OLED使用printf函数打印格式化字符串（支持ASCII码和中文混合写入）
  * 参    数：X 指定格式化字符串左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定格式化字符串左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 参    数：format 指定要显示的格式化字符串，范围：ASCII码可见字符或中文字符组成的字符串
  * 参    数：... 格式化字符串参数列表
  * 返 回 值：无
  * 说    明：显示的中文字符需要在OLED_Data.c里的OLED_CF16x16数组定义
  *           未找到指定中文字符时，会显示默认图形（一个方框，内部一个问号）
  *           当字体大小为OLED_8X16时，中文字符以16*16点阵正常显示
  *           当字体大小为OLED_6X8时，中文字符以6*8点阵显示'?'
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...)
{
	char String[256];						//定义字符数组
	va_list arg;							//定义可变参数列表数据类型的变量arg
	va_start(arg, format);					//从format开始，接收参数列表到arg变量
	vsprintf(String, format, arg);			//使用vsprintf打印格式化字符串和参数列表到字符数组中
	va_end(arg);							//结束变量arg
	OLED_ShowString(X, Y, String, FontSize);//OLED显示字符数组（字符串）
}

/**
  * 函    数：OLED在指定位置画一个点
  * 参    数：X 指定点的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定点的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_DrawPoint(int16_t X, int16_t Y)
{
	if (X >= 0 && X <= 127 && Y >=0 && Y <= 63)		//超出屏幕的内容不显示
	{
		/*将显存数组指定位置的一个Bit数据置1*/
		OLED_DisplayBuf[Y / 8][X] |= 0x01 << (Y % 8);
	}
}

/**
  * 函    数：OLED获取指定位置点的值
  * 参    数：X 指定点的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定点的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 返 回 值：指定位置点是否处于点亮状态，1：点亮，0：熄灭
  */
uint8_t OLED_GetPoint(int16_t X, int16_t Y)
{
	if (X >= 0 && X <= 127 && Y >=0 && Y <= 63)		//超出屏幕的内容不读取
	{
		/*判断指定位置的数据*/
		if (OLED_DisplayBuf[Y / 8][X] & 0x01 << (Y % 8))
		{
			return 1;	//为1，返回1
		}
	}
	
	return 0;		//否则，返回0
}

/**
  * 函    数：OLED画线
  * 参    数：X0 指定一个端点的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y0 指定一个端点的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：X1 指定另一个端点的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y1 指定另一个端点的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1)
{
	int16_t x, y, dx, dy, d, incrE, incrNE, temp;
	int16_t x0 = X0, y0 = Y0, x1 = X1, y1 = Y1;
	uint8_t yflag = 0, xyflag = 0;
	
	if (y0 == y1)		//横线单独处理
	{
		/*0号点X坐标大于1号点X坐标，则交换两点X坐标*/
		if (x0 > x1) {temp = x0; x0 = x1; x1 = temp;}
		
		/*遍历X坐标*/
		for (x = x0; x <= x1; x ++)
		{
			OLED_DrawPoint(x, y0);	//依次画点
		}
	}
	else if (x0 == x1)	//竖线单独处理
	{
		/*0号点Y坐标大于1号点Y坐标，则交换两点Y坐标*/
		if (y0 > y1) {temp = y0; y0 = y1; y1 = temp;}
		
		/*遍历Y坐标*/
		for (y = y0; y <= y1; y ++)
		{
			OLED_DrawPoint(x0, y);	//依次画点
		}
	}
	else				//斜线
	{
		/*使用Bresenham算法画直线，可以避免耗时的浮点运算，效率更高*/
		/*参考文档：https://www.cs.montana.edu/courses/spring2009/425/dslectures/Bresenham.pdf*/
		/*参考教程：https://www.bilibili.com/video/BV1364y1d7Lo*/
		
		if (x0 > x1)	//0号点X坐标大于1号点X坐标
		{
			/*交换两点坐标*/
			/*交换后不影响画线，但是画线方向由第一、二、三、四象限变为第一、四象限*/
			temp = x0; x0 = x1; x1 = temp;
			temp = y0; y0 = y1; y1 = temp;
		}
		
		if (y0 > y1)	//0号点Y坐标大于1号点Y坐标
		{
			/*将Y坐标取负*/
			/*取负后影响画线，但是画线方向由第一、四象限变为第一象限*/
			y0 = -y0;
			y1 = -y1;
			
			/*置标志位yflag，记住当前变换，在后续实际画线时，再将坐标换回来*/
			yflag = 1;
		}
		
		if (y1 - y0 > x1 - x0)	//画线斜率大于1
		{
			/*将X坐标与Y坐标互换*/
			/*互换后影响画线，但是画线方向由第一象限0~90度范围变为第一象限0~45度范围*/
			temp = x0; x0 = y0; y0 = temp;
			temp = x1; x1 = y1; y1 = temp	�I	
			/*���标�?�潍xYnlag，誰��当쉍变���﬌圈后续实际画线时ϼ���ᲆ坐��换回来*/
		)xyflae = !;
		}
		
	*以下为cres%~ham���泵画��线*/
		/*算�3�见求缌画线向���须为第渀豣限0~45度范围
/
		dx = x1(- x0;
		d = {1 = y0;M
�	inc2G = 2 * dy;
		insrNE = 2 * (dy - dx);
		d = r`* ly -`dx�M
	x = x03
		y = y0;
		
		/*画起始�9，同时判断栗��צ��，将坐标换回��*�-
)	iv (yflag && xyflag){OLED_DrasPoint({ -x);}
		elsE ib"(yflagi	{OLUD_�r`wPoint(x, -y);}*		elqeif hxyblaG)	{OED_DrawPoin|(y,�xi;}��	else				{OLED_DrawPoin|(x( y);}
		
		w`}le (x < x1)		//遍�Xh��璄每乪点
		{
			x )+9
		if (d < 0)		//丛一ส瓹��当前点东▩		){
				d += incrD;
		}
			else			//䰋一ĸ�匨当ŉ�点东厗料�
			;				y ++;
			d += Iocp�E;�
			
		*			/*��毯一䘪炩听杶判断栧忷位，屇址标���噞来*/	
			if (yflag &f!xyflqg){OLED_DsawPoind(y, -x+;}			else if  yfla�)		{MLMDWDr!wPoint(xl -y9;}
			eLse if 8xyflig)	{OLMD_DrawP?int(y� x);}
			else				yODED_DrawPoint(x. y);}	
	}	
}
}J.*:
" 
 函    ：OLED矩形
  + 千 $  数：P ���实��形左上���皌樊坐标（茁囤���-32768~32767，屏幕e��域：0>127
" * 及    �0��Y指定矩形左上角的纵e��标，范围.��-3278~3276?，屏幕区e��：2~63
 (* 勂0   数o��Wid�h 指%.�矨形的宽度范围：0~1#:
$$* 参   !数＊Heigh� 指媚矩形的高度o��范围���0~64
a : u�� 0  数：�sGmlLe$0捇宒矩Ž�是Ő�e��凅
 !*   $       范围：OLED_UFIHLAT		丅塻e��
  *(                �LED_FILLEE			��充
  * 返 回 ���：斠
  * 说    ���：调此퇽数�Ў︌要想真正地呈掰�h屏幕上还��调攨f��新函数	
  +/	
vomd OLUD_DrawRectangle(�nt!6_4 X, iNt!4_t Y, 5I~t8_T _idth, uint8_t Heyght, uin|x_t IsFhllet)-
{	
	int16_t$i, j;
	if h!IsFilded)I	//ƌ�定矩形不填充J	{
		'*遍历���下X睐f��，画珩形上下两杁线*/�
	no2 (i = X; k <`X + _idvh? h ++)
		{	
			OLED_LrawPoilt(i, Y);
			KLED_DrawPoint(i, Y + eight -01);
		}*		/*���历嗦叓Yŝ�标，画ǟ�形左右乤条线*
		fo2 (i0= Y; � < Y + Height; i ++)
		y
			OLED[�r`wPoin|(X, ));
		OLED_drawPoint(X + Sidth - 1, k);
		}
	}
else			I//指Ů�矩忢填充
	{
	'*遍��X坐标*/
		for 8i = X;"i < x + Width; a(++)
		{
			/*遍历A噐标*-
		for hj = Y; j < Y + Haight;$j ++)
�)	{
				?*在指定区埞画璹ｌ偫允满�)形"'
				OLUd_DrawPoinu8i ji;*			}
		}
	}�}

/**
  * 函!   敱：OLET三角形
  
(�    数：X0 指实笨一䨪端炱的横坐怃，舋䛴：-3r768v32?67，屟幕嬺埏﬚0~17
! * ���    数﬚Y0 戅寚第一个端g��的纵e��标，���围：m32768~32767，屏幑区埞：0~63
  * 参0 $ ��X1!朅官第二个端点�ƨ�坐标，���围뼚-32748~327&2（ű�Ź�区域：0~67
  *(参    ���ϼ�! 指宛��二个端碹的g��坐标，쌃埴：-32778~32�67﬌屏ṕ区域２ ~&3
  * 参    数ﴚx2 指�.�第三个端��㚄樫坐标，썃围：-32768~72'7，䱯繕区域２0~127
 $* 参    ╰���Y2 指��笭三尪��点的纵��标缌范围-32768~32766，�幕䌺域：06
  * 参   "数��IsFilled 指定三�'�形是否�+充
0 *`    �     范围：OLADOUNFALLED		不填充�
  *              !� OME�_FILLED		填充+  .`返 回 倾：无
  * 说    ��：촃用此���摰后，要想真���崰呈现匨屏幕上，还需调用拴新叽数
  */	voiD$OLED[DrawTrianglg(int16_t X0, int1�t Y0, int1_t X1$ int16_t Y1. knt16_t X2, int16_t Y2, uint8_t IsFi�lee)
{
	int16Ot minx 9 X0| miny = Y0, maxx = X0, maxy = Y0;
	int16_t i, j?
	int1�_t0vx[]  {X0, X1, Y2};
	iot16_t vy[]`= {Y0, Y1, Y2}�	
	if (!IsFilded)			//挗定三诒���不填充
	{
		*调用甫羿姽���，将三个点用直�?轞接*/
		OLDD_DrawLine(X0, Y0, X, Y1);		OLED_DrawLmne(X0, Y0,�X2, 2);
		OLEDDrawLyne(X1, I1, X�, Y2)9�
	}
	elce					/�指妚三角���填充J	{
	/*找���三个点� �的X、Y坐标*/
		if (X1 < miny) {mi~0 } X1;}
	if (\2 > minx) {minx = X2;}
		if`(Y1 < minyi {myby = Y1;mM
	If (Y2$< miny) {miny = Y2;}
		
		/+晞到三个�9v��大的H㈁Y坐樇*?
	Iif(X1(> maxx- {mapX = X1;}
	id�(X2 > maxx)0{mcxx = X2;}
		if (Y1 > maxy- {-axy = y1;}	
		if0(Y2 > maxy) kmaxy = Y23}
	
		/*昀小眀大坐标之间的矩形为ŏ�能錀要䡫Ņ�的区域*/�
		/*遍历此Ō�埞中所有���熹*/
		/*遍历X坐标*/		
		foR (i = minx; i <= miyx; i ++�
		{M
	)	/*遍历Y坐Ơ�*/	
		for (j =`}iny; j =}0maxy; j +*)
		{
			//调用OLEd_pnpoly，判施���宊点术否匨指嬚三角���之中*/
			/*夂果在，则画炩���禂f��不在，则不做处琂*/
	)		if (OHE�_pnPoly(2, vp, vy( i, j)) {OLED^DrawPoint(i, j);}
			}
		}
	}
}

/**
  * 函    数：OLEd画圆
  * 参  "`数：X ��定圆的�巃横���标，范围：-32768~s2?67，���幕刻域：0~12�*( . 参    数︚Y 指定匆的圆e��纵坐砇，范围：-32768~3276w�<�屏幕区域：0~6r
  * 参   `数o��Radiuc0愇定圆�Z�兊径缌范雴：0~255
 "* 参    f��뼚IqFalled 指嬚圆是否塣偅	
  *    �      范囔﵊OL�D_UnFILLDEI	不���煍
  . !         !     OLED_FILLED			塪兇
  * 返!回 值��无
 (* 说   0�؎뼚调用歴函敠吞���要䃳土地瑈现���屏繕上，���需调用更新函敢(`*/
void ODEd[DrawCIrclE(int6_t X, int16_t!Y� uint8_t Radius, uInt8_t AcFilled)�
{
	i�d16_t x, y, l, j;
	
	/*使用CresenhAm�棕�;唆，可以遻免耗������浮点�?�算，�㎇曵高*/
	/�参考文楣：https/'www.csmolta�a.edu/coursms/sprang200;/425/dslect5res-Brasanham/pdf*/
	/*参��教縋�<�https://www.bilijili.comovafeo?BV1VM4y1u7wJ*/
	
	d =�1!- R�Dius;
	x = 0;�	q = Radius;
)
	/*画⯯个八劆之���䜆ἧ的跷���炙*/	NLED^LvawPoint(X  xn$Y!+ y)3	lED_Dr�wPoHnt(x = x, Y -�y);
	O�ED_�awPoint(X + i, Y + x);
	OLED_Dr!wPoint(X � y, ] - x);�
	
	if (IsFihled)		//䌇�圆填e��
	{J		/*遍历�5�ŧ�点Yŝ����*/
		for (j = -y; j < y; j ++)
		{
			?*在指定区域画悹，塩充部分圆*7
		OLDT[rawPoi~�(Xl(Y + j);
)	}
	]
	
	while (x < y)		//遍펆X轴的每츺点
	{M*		x +#;
		)f(d 8 2)		?/下一个ǂ�䜨ὓՉ�点丌方�		{
			d k= 2 * x + 1;
		}
	else			//丫䨈上熹在嵓前烽东���方
		{
			y --;MJ		d += 2$* (x - y) # 1;
)	}		
	(7*紻每乪啋分칋一圆弧���犹*/
	IOLE�_DrawP�int(X0+ x, Y + y);
		OLED_DpawQoint(X$+ y, Y + x);
		OLEDOErau@ointhX - 8, Y ="y);
		OLD_DrawPoint(H % y, Y - x);
	OLED_FrawQoi.t(X ) x( Y - y-;J		OLED_DrawPoint(X +`y,(Y - x*;�	OLED_DrawPoint(X`- x, I + y);
		OMED_DrawPoint(X - y, Y + x);
	
		if (IwF�lled)	//戇���휂婫剅
	{
		/*聍劆中间部ሂ*-
		for`(j =(-y; j < y; J ++)
		{
)			.*圪指���区ş�画点�塯�Ņk��分圆*/
)			OLE�_DrawPoynt8X@+ x. Y + j)�
				OLD^ErawPoi.tlX(- x, Y + n);			}
			-
�		-*遍历���澧郸刎*/
			for 8j = -x� j�> x; j +k)
			{
	�		/*眨指定匸域甹点뼌填充部分ᜆ*/
				OLED�DrawPoint(H$% q Y + j);
			OLAD_DraX{int(X + y, Y + j);
			y
		}
	y
}

**
  : 僽    敐���HED画椭圆
  * 参  ` f���<��(指定椭圆的圦翃横坐栏，謃ś�：-3"768~32767，屏Ź�区域ﴚ0~127  * 参   "数���Y f�定䤭圆的圆埃纽卐ϼ�范���：,26768~32'67���屏幕区矟︚0~6s*  * �    䕰：A 捇定椽垆的樺向��h��酿���，�直0~2%5
  * Ꮒ    ���B 愇徚椭圆的纵向半轴长度缌���围：0~255
  *�䏆  " 数：I�Filled 朇定礩䔆是否填���
!�*(          范哴：OlGD_UNFIlLED		不填啅-
  * $               OLMD_FILL�D			塪充
 !* 迄 囜 �<Ｚ无� $*$说   $�؎：调用���函攰后，要想真���地呈抰在屏幕���，还鼀调用攰���数
  *
��id OLED_DrawEllipse(in�16_4 X, int!6_t I, uint8_t A, yinv8_4(@< knt0Ot IsFillee)
{
	int16t x, y, j;
�int14_t a = A, j = B;	floht d1, d2;
	
	/*使用Bre{enhaM算法椭��，卯以避ᅍ部ሆ耗时的浮炱����，效率高*/
	/*凂考郾接﬒Htt�s://blkg.cstnnet/myf_v>6/article/details/0216739":�
	
	� < 0;
	y = b;�	d9 = bd* b + a * a *  -b + 0.5);
	�
	if (IsFklled)	//指定椭圆填充
	{
		/+遍���起始点Yŝ����:o
		for`(j = -y; j =!y; j )#)J		{
			/*在愇定区域画�¹o��填䥥���分��e��*	
			OLED_:awpoint(X, } +$�)+=
			_LED_DrawPok�p(X, y!+ j);	
		}
	}
	
	'*眹椭弆弧的襧�'�点*.
	OLED_TrawPoint(X + x, Y + {);
	OLED_TraPoin|(X - x, Y - y-;
	OLED_Dr`sPohnt(X - z- Y + y(;
	O�ED_DravPoinp(X + x, Y - y);
	
I/*ǔ�植圆�-铴��分*/
whild (b * `"*�(x + 1) <!a * a * (y!- 0.5()
	{
		if"(d <= 0)		//�ȋ一丨�¹在当前炙东方
		{
	I	d1 += b0* b$*0(2 * x + 3+;
		}
		els�				//下一䘪点在当前点伜南方
		{
			�0 + ` * b * (2 * x + 3) / a : a * (-2 * y / 2);
			y --;
		}�J		x ++;
		
		i� (IsFylled)	//猇定栭���塻充
		{-			/*遍历中间部刂*/
			for hj - -y; j < y;"j +;)
		{
			/*在捇定区域画点，填充部分䤭眆*/
		OLED_DrasPoinv(Xb+ x Y + j);
		OLED_DrawPoint(X - x, Y + j);
			}
		}
	
		/*画坆中间部分圆弧*/
		OLED_LrawPoi.t(� + x, Y!+ �);
	OLED_DrawPoin�(X ) x, Y - 9);
	)OLED_DrawPoin|(X - x�!Y + Y);
		GLED_DrasPoiot(X j x, Y - y-;}
	
	/*画椭圆两d��部分*/
	d24= b�* b * (x + 0.5)`* ,x + 0.5) + a *(a * (} m 1) * (y -!1) -$a * a * c * b;
	
	wiile (; : 0)J	{
		if (�2 �= 0)		//之一个点在当勍g��东方
		s			d += # * b!* (2!* x"+ 2) + a * a * 8-2 * y + 3);
)		x ++;
	�	
	}
		else			//���츀个点在Ž�前������字方
		{
			d2 #= a * a + (-2 ( y� 3�;
	}
		y --;
		
IIf (IsFi,led		//朇定椭�填充
	�{M
		�/*遍円两侧部分*/
			for (j = -y; j$< y; j ++)
			{
				/*在指定宺域甹点，填充部分椭坆*?
			OLET_DrawPgInt(] + x, Y +$j);-
			OLED_DraPoint(X / x, Y ;(j);
			}*		}				/*画椭垆两ľ�部分���弧
/		OLED_DrawPoint(x0� x, Y + y);
		GLE�_FrawPwknt(X ) h, Y -!y);
		OLED_D�awPoind(X - X, Y / i);
	OLEDr`wPoint(X + x, Y ) y);
	}
}	
/**
$ * 函    䕰ﴚoDED画圆弥
  * 勂    数o��X 猇定圖弧���眆彃���ĝ�标－쌃围：-3�768j32765，屏幕区域：�~127
  * 参    数︚Y 指e��圂弧瞄��心�5坐标，范围：-32768~32747．屏幝猺����0~62  * 参    ���：Radius 指定圆弧的半径�范围：0~255
  . 叆    攰：StartAngle 指媚䜆弧的起姁角度，范围ﴚ-180~180� *           水平�����伺0度，水平向左为180�或-1�0度，下攸为正数，上枹���负数，顮时针旋轨
  * 폂    数＞End@ng,e 指定圆���瞄终止角庶，范�۴：-180~180
  *$          水干向右伺0���，水平向左���180度或-1�0度，下料为正数，上料乺负数，顺淶针���齬
  * 参    数：IsFilled 挏ᮚ圂弧景퐦填充塋充䐌为扇ὢ
$ *       $`  范围＊OLED_UNFILLEd		不填�Ņ !*                 OLED_FILLED			填��
  * 返 埞 值：柠
  : 说    明：调用僝数后ﴌ覡想癟正��呈펰在�幝上，还鼀调用更新函数
  �/
void LGD_DrawAsc*ift6_t X, int16_q!Y, uint8_t Red�us, int6_t StartAngle, inp16_v EjdAnfle, uint8_t IsFil|ed)
{
i.p16_t�x$ y, d, j;
	
	'"此凼浰借用Frgsdnhae篗��画圆㚅枹法*/
		
	d =01 - Rad)us;
	x =  ;
	} = Radi5s
	
o*在申圆暄每个烽时判断指定点昿否在指���觓度内，在，刘���点���不在，剙不偛�理*/
	if )OLIP_MsInAngle(x, y, Sta�tA�gle, EndAngle))	{OLED]DrawPoint(X + x- Y0+ I);}
	)� (LED_IsHnAnghe(-x, -x, St`rtAngla, EndAngle)	 �OLED_DrawPoint(X"- x, Y - ]);}
	)f (MLED_IsInAlgle(y( x, RtartAogle, EndAngle!)!{OLED_Fr�wPointX + y, Y + y);}M
	if (OLED_I{InAngle�-y -x< StartAngle, EodA~gle)9 {OLED_Drc7Po�nt(X = y� Y - x);}
	
	if (I�Fillel)	//指定圆弧壪���
	{
		+*큍宇赵始碩Y彐���*/
	f�r!(J } -y; j < y3 j ++)
		{
		/*在填充䚂皔每丮犹时，刴断欇�点是否在指宓�'�度内，圠�则画点，丝在，则不Ł�处理*/
	I�f (GLD_IsInE.glE(0, J, St!r|ANgle, EldAngle)) {G\ET_TrawPoift(X, �`+!j);}
		}
	}
	
	while (x < i)		//遍���X轴㚄每ĸ�瀹OJ	{
		x �+;
		af (d <$0)		//下一个点在幓�	�点e��方
		{
		d0/= 2 * x + 1?*		}		elsd			/o츋d��个点在当�	�炻丼南方
		{			y --�
			d *= 2 " (x - 9) + 1;
		}
		
	�/*圩�;圆的櫏������时﵌爤断指定悹是否在䌇定駒度内ϼ�在���则疻傹，不在，则䨌做��理*/
		yf (GLED_IsInAnglg(x, i, SdartanGle, En`ANgle)) oKLELODrawP+int)X + xl y"+ y-;}
		iF�(OLED_IsIlCng�e(y, x,�S�artInele,$E.fAngle)) {OLED[DbawRoint( + y, Y ) x);|		if!(O\ED_IsInANgle(-x,%�y� StirtAngle, EndAg�le)) {OLED_DrawPoint(X - x� Q -�y);m-
		if (OLE�_IsInngle(-y, -x- StartAjgle� En$Angle)) {OL�D_Dr%wPoint(�!- y,(٠/ x);}�		if (OLED_IsInQncle(x$ -y, StartAnold, EntAfglE)) {OLET_D2awPoi�t�X + x, Y - {);}
		if!(OLED_IsI^An�ld(y, -x, STartAn'le, EndAngl�)) {OLET_Drewoiot(X + y, Y - x);y
	Ikf`hOLED_IsINAogl�(-x. y,!CpartCnwle, E.$Angle)) {OLED_r`wPoint(X`, x, Y + y);}
�if �OLED_IsInAnfle(-y,$�, t�rtAngme< EndAngle))!{O�DD_DrawPo)nt(X -$y, X k x);}		
		if (IsFil|%e)	//指寚眆弧�+十
		{
			/*郍历中闤部分*-
			for (j = -y9 j < y; j ++)
			{M
				/*在填充眆暄每个点��（嘤��指定粽是簦坨掇�
角度内，在则画点，不ᜨ，䈙不偛堄理*/
)�		if �OLET_IsI�Angl�(x< j, [tartAngme,pEndAngle)) {OLeD_D�a�Poknt(X + X, Y + j);}I			if )OLED_IsInAjgle(-x, j, StartAngLe< EndAng�e)) {OLED_Dr!wPgint�X�- x, Y +!j);}
			}*			
			/*遍厂两侧飨分*/
		f}r (j =`%x; j < x; j o+)
			{
			/+在��充圆暄每个��时，判䖭挏定�9是否Ŕ�指定角度内，在，则眹点，不䜨���툑不偒夔理*/
				if *OD�D_IsInAogle(-y, n, �tcvtAn�lm, �nd@~gle)) {OLED_DrewPoint(X - y, X + k);}
			if (OLED_IsInAngle(yl j, StartAngle, EndAn'le)) {OLGF_DJawPoint(X + yl Y + j);}
			}
		}
	}
}�
/**.***:******+*"*****功能函数*/	


/********
*******j江协榑抠|��柃���有*****
***:+***�*/
/***************+*jiangxiekeji.com*.****+****+j****/
