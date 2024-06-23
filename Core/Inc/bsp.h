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


#define  Enable_EventRecorder  0		/* Ĭ���ǹر�״̬ */

#define UART_RX_MODE_DMA						/* �������ڽ��� ��С��������󳤶�rxSize */	

/* ����ȫ���жϵĺ� */
#define ENABLE_INT()	__set_PRIMASK(0)	/* ʹ��ȫ���ж� */
#define DISABLE_INT()	__set_PRIMASK(1)	/* ��ֹȫ���ж� */
/* ���������ڵ��Խ׶��Ŵ� */
#define BSP_Printf		printf

// ����״̬ö��
typedef enum {
    STATE_STOP = 0,
    STATE_UP = 1,
    STATE_DOWN = 2
} State;

// ��������ö��
typedef enum {
    CONDITION_UP_COMMAND = 0,				//��ָ��
    CONDITION_DOWN_COMMAND = 1,			//��ָ��
    CONDITION_UPPER_LIMIT = 2,			//����λ
    CONDITION_LOWER_LIMIT = 3,			//����λ
    CONDITION_STOP_COMMAND = 4			//ָֹͣ��
} Condition;

//extern Condition;
extern int conditions[5];


/*	�����־λ	*/
struct MOTOT_CONTROL
{
	unsigned char control_flag; //���Ʊ�־λ
	unsigned char stop_flag;
	unsigned char error_flag;   //�����־λ
	unsigned char limit_flag;		//��λ��־λ
	
	unsigned char sport_flag;		//�˶�״̬
};


/*	͸��������Э��	*/
struct Uart_DataPacket
{
    unsigned char Start_Flag;    // ��ʼ�����
    unsigned char Clarity_Flag;  // ͸�������
    unsigned char End_Flag;      // ��ֹ�����
    unsigned char RX_BUF[512];    // ���ջ���,���15���ֽڣ���һ���ռ�����Ч���ݸ���
    unsigned char TX_BUF[512];    // ���ͻ���,���15���ֽڣ���һ���ռ�����Ч���ݸ���
};

typedef struct
{
	unsigned char *pRxBuf;
	unsigned char rxFlag;
	unsigned char rxLen;
//	const unsigned char rxSize;	/*const �����ڽṹ���������ʱ��ֵ */
	const unsigned int rxSize;	/*const �����ڽṹ���������ʱ��ֵ */
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