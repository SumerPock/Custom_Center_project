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
*	�� �� ��: bsp_RunPer10ms
*	����˵��: �ú���ÿ��10ms��Systick�жϵ���1�Ρ���� bsp_timer.c�Ķ�ʱ�жϷ������һЩ����ʱ��Ҫ���ϸ��
*			������Է��ڴ˺��������磺����ɨ�衢���������п��Ƶȡ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_RunPer10ms(void)
{
	bsp_KeyScan10ms();
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_RunPer1ms
*	����˵��: �ú���ÿ��1ms��Systick�жϵ���1�Ρ���� bsp_timer.c�Ķ�ʱ�жϷ������һЩ��Ҫ�����Դ��������
*			 ���Է��ڴ˺��������磺��������ɨ�衣
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_RunPer1ms(void)
{
	
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_Idle
*	����˵��: ����ʱִ�еĺ�����һ����������for��whileѭ������������Ҫ���� CPU_IDLE() �������ñ�������
*			 ������ȱʡΪ�ղ������û��������ι��������CPU��������ģʽ�Ĺ��ܡ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_Idle(void)
{
	/* --- ι�� */

	/* --- ��CPU�������ߣ���Systick��ʱ�жϻ��ѻ��������жϻ��� */

	/* ���� emWin ͼ�ο⣬���Բ���ͼ�ο���Ҫ����ѯ���� */
	//GUI_Exec();

	/* ���� uIP Э�飬���Բ���uip��ѯ���� */
	//TOUCH_CapScan();
}

/**
 * @brief ��Ҫ��������������
 *
 * ��������ṩ���ں����ĸ߼�������������������������
 *
 * @param[in]  ����1  ����1�������������������ͺͺ���
 * @param[out] ����2  ����2�������������������ͺͺ���
 *
 * @return ��������ֵ�������������������ͺͺ��壬������õĻ���
 *
 * @note �κ�����ע��������÷�ʾ����������������ӡ�
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
                // ��Ӵ��������
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
    // ���� Else��δ�����Ҳ�����ʼ�����ж������ݲ�����Ч���ݣ���������ִ���κβ���
}


/**
 * @brief ��Ҫ��������������
 *
 * ��������ṩ���ں����ĸ߼�������������������������
 *
 * @param[in]  ����1  ����1�������������������ͺͺ���
 * @param[out] ����2  ����2�������������������ͺͺ���
 *
 * @return ��������ֵ�������������������ͺͺ��壬������õĻ���
 *
 * @note �κ�����ע��������÷�ʾ����������������ӡ�
 */
void HandleStartSymbol()
{
    RemoteDataPacket.Start_Flag = 1;
    RemoteDataPacket.RX_BUF[0] = 0;
}


/**
 * @brief ��Ҫ��������������
 *
 * ��������ṩ���ں����ĸ߼�������������������������
 *
 * @param[in]  ����1  ����1�������������������ͺͺ���
 * @param[out] ����2  ����2�������������������ͺͺ���
 *
 * @return ��������ֵ�������������������ͺͺ��壬������õĻ���
 *
 * @note �κ�����ע��������÷�ʾ����������������ӡ�
 */
void HandleEndSymbol()
{
    RemoteDataPacket.Start_Flag = 0;
    RemoteDataPacket.Clarity_Flag = 0;
    RemoteDataPacket.End_Flag = 1;
		UART_RemoteCMD_Process(); 				// ���ݰ�������ϣ��������ݴ������
    RemoteDataPacket.End_Flag = 0;    // ������ϣ�End�������!!!
}

//
unsigned char UART_RemoteCMD_Process(void)
{
	unsigned char Temp_u8 = 0, CheckSum = 0;
	unsigned short i = 0 , Temp_int16 = 0;
	unsigned int *p_u32 = 0;
	static unsigned short  	RandomIndex = 0;
	static unsigned char 		RandomRxError = 0;	// α����������ʱ�ĳ��������
	
	unsigned char ucreturn = 0;
	for(i = 1 ; i < RemoteDataPacket.RX_BUF[0] ; i++)
	{ //RemoteDataPacket.RX_BUF[0]�������ݳ��ȣ���1��ʼ
		CheckSum = CheckSum + RemoteDataPacket.RX_BUF[i];
	}
	if(CheckSum == RemoteDataPacket.RX_BUF[RemoteDataPacket.RX_BUF[0]])
	{		
		switch(RemoteDataPacket.RX_BUF[2])
		{
			case 0x00:
			break;
			
			case 0x01:	//����������
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
			else if(RemoteDataPacket.RX_BUF[3] == 0x00)	//�ر�ʱ������������������
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
		ucreturn = 1;	//�˳�������У��ʹ���
	}	
	return ucreturn;
}
/**
 * @brief ��Ҫ��������������
 *
 * ��������ṩ���ں����ĸ߼�������������������������
 *
 * @param[in]  ����1  ����1�������������������ͺͺ���
 * @param[out] ����2  ����2�������������������ͺͺ���
 *
 * @return ��������ֵ�������������������ͺͺ��壬������õĻ���
 *
 * @note �κ�����ע��������÷�ʾ����������������ӡ�
 */
void HandleValidData(unsigned char inputCharacter)
{
    UART_Inbuff(inputCharacter);
}


/**
 * @brief ��Ҫ��������������
 *
 * ��������ṩ���ں����ĸ߼�������������������������
 *
 * @param[in]  ����1  ����1�������������������ͺͺ���
 * @param[out] ����2  ����2�������������������ͺͺ���
 *
 * @return ��������ֵ�������������������ͺͺ��壬������õĻ���
 *
 * @note �κ�����ע��������÷�ʾ����������������ӡ�
 */
void HandleClaritySymbol(unsigned char inputCharacter)
{
    RemoteDataPacket.Clarity_Flag = 1;
}


/**
 * @brief ��Ҫ��������������
 *
 * ��������ṩ���ں����ĸ߼�������������������������
 *
 * @param[in]  ����1  ����1�������������������ͺͺ���
 * @param[out] ����2  ����2�������������������ͺͺ���
 *
 * @return ��������ֵ�������������������ͺͺ��壬������õĻ���
 *
 * @note �κ�����ע��������÷�ʾ����������������ӡ�
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

// �������ڰ�λ�޸�ָ��λ��ֵ
unsigned char modifyBit(unsigned char data, int bitIndex, int bitValue) {
    if (bitIndex < 0 || bitIndex > 7) {
        // λ����������Χ
        return data; // ����ԭʼ���ݣ������κ��޸�
    }

    // ����һ�����룬ֻ��ָ��λ��1������λ��0
    unsigned char mask = 1 << bitIndex;

    if (bitValue) {
        // ��ָ��λ����Ϊ1
        data |= mask;
    } else {
        // ��ָ��λ����Ϊ0
        data &= ~mask;
    }

    return data;
}


// ����ȫ�����飬���ڴ洢����������
unsigned char globalPackedData[256]; // ����ʵ�������������С

// ����ȫ�ֱ��������ڴ洢���������ݳ���
int globalPackedDataLength = 0;

// �������ڽ��������ݽ��ж��δ�����洢��ȫ��������
void doublePackAndStore(const unsigned char *data, int length) {
    if (data == NULL || length <= 0) {
        // ���������Ч����ִ���κβ���
        return;
    }

    // ͸����
    unsigned char transparentChar = 0xEB;
    // ��ʼ��
    unsigned char startChar = 0xEA;
    // ֹͣ��
    unsigned char stopChar = 0xED;
    // ��ŷ���ÿ��ʹ�ú�+1
    static unsigned char sequenceChar = 0x00;

    // ����ʼ���洢��ȫ��������
    globalPackedData[globalPackedDataLength++] = startChar;
    // ������ŷ���ÿ��+1
    sequenceChar++;

    // �洢�µ���ŷ����������255�������¿�ʼ
    if (sequenceChar == 0) {
        sequenceChar = 1;
    }

    // �洢��ŷ�
    globalPackedData[globalPackedDataLength++] = sequenceChar;
    for (int i = 0; i < length; i++) {
        if (data[i] == transparentChar || data[i] == startChar || data[i] == stopChar || data[i] == sequenceChar) {
            // ���ݰ����ؼ��֣��Ȳ���͸����
            globalPackedData[globalPackedDataLength++] = transparentChar;
        }
        // �洢�����ֽ�
        globalPackedData[globalPackedDataLength++] = data[i];
    }

    // �洢ֹͣ��
    globalPackedData[globalPackedDataLength++] = stopChar;


}


