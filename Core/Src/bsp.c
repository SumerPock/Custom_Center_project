#include "bsp.h"

/**
 * @brief Calculates the factorial of a non-negative integer.
 *
 * This function computes the factorial of a non-negative integer n.
 *
 * @param n The non-negative integer for which to calculate the factorial.
 * @return The factorial of n.
 *
 * @note The function returns 1 for n = 0 and 1 to indicate an error for negative values of n.
 */



#define START_SYMBOL    0xEA
#define END_SYMBOL      0xED
#define CLARITY_SYMBOL  0xEB

void HandleEndSymbol(void);
void HandleStartSymbol(void);
void HandleValidData(unsigned char inputCharacter);
void UART_Inbuff(unsigned char inputCharacter);

unsigned char UART_RemoteCMD_Process(void);


/*
*********************************************************************************************************
*	函 数 名: bsp_RunPer10ms
*	功能说明: 该函数每隔10ms被Systick中断调用1次。详见 bsp_timer.c的定时中断服务程序。一些处理时间要求不严格的
*			任务可以放在此函数。比如：按键扫描、蜂鸣器鸣叫控制等。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_RunPer10ms(void)
{
	bsp_KeyScan10ms();
}

/*
*********************************************************************************************************
*	函 数 名: bsp_RunPer1ms
*	功能说明: 该函数每隔1ms被Systick中断调用1次。详见 bsp_timer.c的定时中断服务程序。一些需要周期性处理的事务
*			 可以放在此函数。比如：触摸坐标扫描。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_RunPer1ms(void)
{
	
}

/*
*********************************************************************************************************
*	函 数 名: bsp_Idle
*	功能说明: 空闲时执行的函数。一般主程序在for和while循环程序体中需要插入 CPU_IDLE() 宏来调用本函数。
*			 本函数缺省为空操作。用户可以添加喂狗、设置CPU进入休眠模式的功能。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_Idle(void)
{
	/* --- 喂狗 */

	/* --- 让CPU进入休眠，由Systick定时中断唤醒或者其他中断唤醒 */

	/* 例如 emWin 图形库，可以插入图形库需要的轮询函数 */
	//GUI_Exec();

	/* 例如 uIP 协议，可以插入uip轮询函数 */
	//TOUCH_CapScan();
}

/**
 * @brief 简要描述函数的作用
 *
 * 这里可以提供关于函数的高级概述，包括它的输入和输出。
 *
 * @param[in]  参数1  参数1的描述，包括数据类型和含义
 * @param[out] 参数2  参数2的描述，包括数据类型和含义
 *
 * @return 函数返回值的描述，包括数据类型和含义，如果适用的话。
 *
 * @note 任何特殊注意事项或用法示例都可以在这里添加。
 */
void UART_RemoteDecode(unsigned char inputCharacter)
{
    if (RemoteDataPacket.Start_Flag == 1)
    {
        if (RemoteDataPacket.Clarity_Flag == 1)
        {
            if (inputCharacter == START_SYMBOL || inputCharacter == CLARITY_SYMBOL || inputCharacter == END_SYMBOL)
            {
                UART_Inbuff(inputCharacter);
            }
            else
            {
                RemoteDataPacket.Start_Flag = 0;
                RemoteDataPacket.Clarity_Flag = 0;
                RemoteDataPacket.RX_BUF[0] = 0;
                // 添加错误处理代码
            }
        }
        else if (inputCharacter == CLARITY_SYMBOL)
        {
            RemoteDataPacket.Clarity_Flag = 1;
        }
        else if (inputCharacter == END_SYMBOL)
        {
            HandleEndSymbol();
        }
        else if (inputCharacter == START_SYMBOL)
        {
            HandleStartSymbol();
        }
        else
        {
            HandleValidData(inputCharacter);
        }
    }
    else if (inputCharacter == START_SYMBOL)
    {
        HandleStartSymbol();
    }
    // 最后的 Else，未启动且不是启始符，判定该数据不是有效数据，忽略它不执行任何操作
}


/**
 * @brief 简要描述函数的作用
 *
 * 这里可以提供关于函数的高级概述，包括它的输入和输出。
 *
 * @param[in]  参数1  参数1的描述，包括数据类型和含义
 * @param[out] 参数2  参数2的描述，包括数据类型和含义
 *
 * @return 函数返回值的描述，包括数据类型和含义，如果适用的话。
 *
 * @note 任何特殊注意事项或用法示例都可以在这里添加。
 */
void HandleStartSymbol()
{
    RemoteDataPacket.Start_Flag = 1;
    RemoteDataPacket.RX_BUF[0] = 0;
}


/**
 * @brief 简要描述函数的作用
 *
 * 这里可以提供关于函数的高级概述，包括它的输入和输出。
 *
 * @param[in]  参数1  参数1的描述，包括数据类型和含义
 * @param[out] 参数2  参数2的描述，包括数据类型和含义
 *
 * @return 函数返回值的描述，包括数据类型和含义，如果适用的话。
 *
 * @note 任何特殊注意事项或用法示例都可以在这里添加。
 */
void HandleEndSymbol()
{
    RemoteDataPacket.Start_Flag = 0;
    RemoteDataPacket.Clarity_Flag = 0;
    RemoteDataPacket.End_Flag = 1;
		UART_RemoteCMD_Process(); 				// 数据包接收完毕，插入数据处理代码
    RemoteDataPacket.End_Flag = 0;    // 处理完毕，End标记清零!!!
}

//
unsigned char UART_RemoteCMD_Process(void)
{
	unsigned char Temp_u8 = 0, CheckSum = 0;
	unsigned short i = 0 , Temp_int16 = 0;
	unsigned int *p_u32 = 0;
	static unsigned short  	RandomIndex = 0;
	static unsigned char 		RandomRxError = 0;	// 伪随机编码接收时的出错计数器
	
	unsigned char ucreturn = 0;
	for(i = 1 ; i < RemoteDataPacket.RX_BUF[0] ; i++)
	{ //RemoteDataPacket.RX_BUF[0]代表数据长度，从1开始
		CheckSum = CheckSum + RemoteDataPacket.RX_BUF[i];
	}
	if(CheckSum == RemoteDataPacket.RX_BUF[RemoteDataPacket.RX_BUF[0]])
	{		
		switch(RemoteDataPacket.RX_BUF[2])
		{
			case 0x00:
			break;
			
			case 0x01:	//驱动器设置
			if(RemoteDataPacket.RX_BUF[3] == 0x01)
			{
				SetMotot_control.stop_flag = 0x01;
				if(RemoteDataPacket.RX_BUF[4] == 0x01)
				{
					conditions[CONDITION_UP_COMMAND] 		= 1;
					conditions[CONDITION_DOWN_COMMAND] 	= 0;
					conditions[CONDITION_STOP_COMMAND]  = 1;
				}
				else if(RemoteDataPacket.RX_BUF[4] == 0x00)
				{
					conditions[CONDITION_UP_COMMAND] 		= 0;
					conditions[CONDITION_DOWN_COMMAND] 	= 1;
					conditions[CONDITION_STOP_COMMAND] = 1;
				}
			}
			else if(RemoteDataPacket.RX_BUF[3] == 0x00)	//关闭时不考虑正负，无意义
			{
				conditions[CONDITION_DOWN_COMMAND] = 0;
				conditions[CONDITION_UP_COMMAND] = 0;
				conditions[CONDITION_STOP_COMMAND] = 0;
			}
			break;
			
			case 0x02:
			break;
			
			case 0x03:
			break;			
			default:
			break;
		}
	}
	else
	{
		ucreturn = 1;	//退出，报告校验和错误
	}	
	return ucreturn;
}
/**
 * @brief 简要描述函数的作用
 *
 * 这里可以提供关于函数的高级概述，包括它的输入和输出。
 *
 * @param[in]  参数1  参数1的描述，包括数据类型和含义
 * @param[out] 参数2  参数2的描述，包括数据类型和含义
 *
 * @return 函数返回值的描述，包括数据类型和含义，如果适用的话。
 *
 * @note 任何特殊注意事项或用法示例都可以在这里添加。
 */
void HandleValidData(unsigned char inputCharacter)
{
    UART_Inbuff(inputCharacter);
}


/**
 * @brief 简要描述函数的作用
 *
 * 这里可以提供关于函数的高级概述，包括它的输入和输出。
 *
 * @param[in]  参数1  参数1的描述，包括数据类型和含义
 * @param[out] 参数2  参数2的描述，包括数据类型和含义
 *
 * @return 函数返回值的描述，包括数据类型和含义，如果适用的话。
 *
 * @note 任何特殊注意事项或用法示例都可以在这里添加。
 */
void HandleClaritySymbol(unsigned char inputCharacter)
{
    RemoteDataPacket.Clarity_Flag = 1;
}


/**
 * @brief 简要描述函数的作用
 *
 * 这里可以提供关于函数的高级概述，包括它的输入和输出。
 *
 * @param[in]  参数1  参数1的描述，包括数据类型和含义
 * @param[out] 参数2  参数2的描述，包括数据类型和含义
 *
 * @return 函数返回值的描述，包括数据类型和含义，如果适用的话。
 *
 * @note 任何特殊注意事项或用法示例都可以在这里添加。
 */
void UART_Inbuff(unsigned char inputCharacter)
{
    if (RemoteDataPacket.RX_BUF[0] > 10)
    {
        RemoteDataPacket.RX_BUF[0] = 0;
        RemoteDataPacket.Start_Flag = 0;
        RemoteDataPacket.Clarity_Flag = 0;
        RemoteDataPacket.End_Flag = 0;
    }
    else
    {
        RemoteDataPacket.RX_BUF[0]++;
        RemoteDataPacket.RX_BUF[RemoteDataPacket.RX_BUF[0]] = inputCharacter;
        RemoteDataPacket.Clarity_Flag = 0;
    }
}

// 函数用于按位修改指定位的值
unsigned char modifyBit(unsigned char data, int bitIndex, int bitValue) {
    if (bitIndex < 0 || bitIndex > 7) {
        // 位索引超出范围
        return data; // 返回原始数据，不做任何修改
    }

    // 创建一个掩码，只有指定位是1，其他位是0
    unsigned char mask = 1 << bitIndex;

    if (bitValue) {
        // 将指定位设置为1
        data |= mask;
    } else {
        // 将指定位设置为0
        data &= ~mask;
    }

    return data;
}


// 定义全局数组，用于存储打包后的数据
unsigned char globalPackedData[256]; // 根据实际需求定义数组大小

// 定义全局变量，用于存储打包后的数据长度
int globalPackedDataLength = 0;

// 函数用于将数组内容进行二次打包并存储在全局数组中
void doublePackAndStore(const unsigned char *data, int length) {
    if (data == NULL || length <= 0) {
        // 输入参数无效，不执行任何操作
        return;
    }

    // 透明符
    unsigned char transparentChar = 0xEB;
    // 起始符
    unsigned char startChar = 0xEA;
    // 停止符
    unsigned char stopChar = 0xED;
    // 序号符，每次使用后+1
    static unsigned char sequenceChar = 0x00;

    // 将起始符存储在全局数组中
    globalPackedData[globalPackedDataLength++] = startChar;
    // 更新序号符，每次+1
    sequenceChar++;

    // 存储新的序号符，如果超出255，则重新开始
    if (sequenceChar == 0) {
        sequenceChar = 1;
    }

    // 存储序号符
    globalPackedData[globalPackedDataLength++] = sequenceChar;
    for (int i = 0; i < length; i++) {
        if (data[i] == transparentChar || data[i] == startChar || data[i] == stopChar || data[i] == sequenceChar) {
            // 数据包含关键字，先插入透明符
            globalPackedData[globalPackedDataLength++] = transparentChar;
        }
        // 存储数据字节
        globalPackedData[globalPackedDataLength++] = data[i];
    }

    // 存储停止符
    globalPackedData[globalPackedDataLength++] = stopChar;


}


