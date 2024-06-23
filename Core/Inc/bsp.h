#ifndef __BSP_H
#define __BSP_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "main.h"
#include "bsp_gpio.h"
#include "timer_pwm.h"
#include "bsp_dma.h"
#include "bsp_uart.h"
#include "bsp_dwt.h"
#include "bsp_key.h"
#include "bsp_timer.h"


#include "EventRecorder.h"
#include "EventRecorderConf.h"


#define  Enable_EventRecorder  0		/* 默认是关闭状态 */

#define UART_RX_MODE_DMA						/* 开启串口接收 大小缓冲区最大长度rxSize */	

/* 开关全局中断的宏 */
#define ENABLE_INT()	__set_PRIMASK(0)	/* 使能全局中断 */
#define DISABLE_INT()	__set_PRIMASK(1)	/* 禁止全局中断 */
/* 这个宏仅用于调试阶段排错 */
#define BSP_Printf		printf

// 定义状态枚举
typedef enum {
    STATE_STOP = 0,
    STATE_UP = 1,
    STATE_DOWN = 2
} State;

// 定义条件枚举
typedef enum {
    CONDITION_UP_COMMAND = 0,				//上指令
    CONDITION_DOWN_COMMAND = 1,			//下指令
    CONDITION_UPPER_LIMIT = 2,			//上限位
    CONDITION_LOWER_LIMIT = 3,			//下限位
    CONDITION_STOP_COMMAND = 4			//停止指令
} Condition;

//extern Condition;
extern int conditions[5];


/*	电机标志位	*/
struct MOTOT_CONTROL
{
	unsigned char control_flag; //控制标志位
	unsigned char stop_flag;
	unsigned char error_flag;   //错误标志位
	unsigned char limit_flag;		//限位标志位
	
	unsigned char sport_flag;		//运动状态
};


/*	透明符传输协议	*/
struct Uart_DataPacket
{
    unsigned char Start_Flag;    // 启始符标记
    unsigned char Clarity_Flag;  // 透明符标记
    unsigned char End_Flag;      // 终止符标记
    unsigned char RX_BUF[512];    // 接收缓冲,最大15个字节，第一个空间存放有效数据个数
    unsigned char TX_BUF[512];    // 发送缓冲,最大15个字节，第一个空间存放有效数据个数
};

typedef struct
{
	unsigned char *pRxBuf;
	unsigned char rxFlag;
	unsigned char rxLen;
//	const unsigned char rxSize;	/*const 必须在结构体变量定义时赋值 */
	const unsigned int rxSize;	/*const 必须在结构体变量定义时赋值 */
}BspUart_t;


enum
{
	DIRECTION = 2,
	OFFLINE = 1,
	LED_NUM0 = 3,
	LED_NUM1 = 4,
	
	MOTORPOW = 5,
	FILLLIGHT = 6,
	SWITCHNF = 7
};


extern BspUart_t g_tBspUsart1;
extern BspUart_t g_tBspUart4;
extern BspUart_t g_tBspUart5;
extern BspUart_t g_tBspUsart2;
extern BspUart_t g_tBspUsart3;
extern BspUart_t g_tBspUsart6;

extern unsigned char globalPackedData[256];
extern int globalPackedDataLength;

extern struct Uart_DataPacket RemoteDataPacket;
extern struct MOTOT_CONTROL SetMotot_control;

extern void UART_RemoteDecode(uint8_t inputCharacter);
extern unsigned char modifyBit(unsigned char data, int bitIndex, int bitValue);

extern void doublePackAndStore(const unsigned char *data, int length);

#endif /* __MAIN_H */