/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*͸��������Э��*/
struct Uart_DataPacket RemoteDataPacket = {0 , 0 , 0 , {0} , {0}};
struct MOTOT_CONTROL SetMotot_control = {0};
int conditions[5] = {0};

// �������˶�����
#define UPWARD 0
#define DOWNWARD 1
#define STOP 2

// ����ԭ��
void MotorControl(uint8_t direction);
bool IsLimitSensor1Active();
bool IsLimitSensor2Active();

//Condition
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint8_t ucKeyCode;		/* �������� */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
	bsp_InitGpio();
	bsp_GpioOn(DIRECTION);	//�ѻ�
	bsp_GpioOn(OFFLINE);		//����

//	bsp_GpioOff(DIRECTION);	//�ѻ�
//	bsp_Gpio0ff(OFFLINE);		//����	
	bsp_GpioOff(LED_NUM0);
	bsp_GpioOff(LED_NUM1);

	bsp_GpioOn(MOTORPOW);
	bsp_GpioOn(FILLLIGHT);
	bsp_GpioOn(SWITCHNF);

	bsp_GpioOff(OFFLINE);
	
	MX_DMA_Init();

	/*����λ������*/
	MX_USART3_UART_Init();
	/**UART DMA config**/
	//HAL_UART_Transmit(&husart1, (uint8_t *)"this is uart1 send data\r\n", 24, 1000);	
	__HAL_UART_ENABLE_IT(&husart3 , UART_IT_IDLE);	/* �������ڿ����ж� */

	MX_USART1_UART_Init();
	/**UART DMA config**/
	//HAL_UART_Transmit(&husart1, (uint8_t *)"this is uart1 send data\r\n", 24, 1000);	
	__HAL_UART_ENABLE_IT(&husart1 , UART_IT_IDLE);	/* �������ڿ����ж� */
	
	
	#ifdef UART_RX_MODE_DMA
	HAL_UART_Receive_DMA(&husart3 , g_tBspUsart3.pRxBuf , g_tBspUsart3.rxSize);
	HAL_UART_Receive_DMA(&husart1 , g_tBspUsart1.pRxBuf , g_tBspUsart1.rxSize);
	#else
	HAL_UART_Receive_IT(&husart1, g_tBspUart1.pRxBuf, g_tBspUart1.rxSize);
	#endif
	unsigned char buff[5] = {0};
	BSP_UART_Transmit_DMA(&husart3,  &buff[0], 5);
	printf("this is project is begin");
	
//	bsp_InitDWT();
	bsp_InitKey();
	bsp_InitTimer();  	/* ��ʼ���δ�ʱ�� */
	/* 
	 Event Recorder��
	 - �����ڴ���ִ��ʱ�������MDK5.25�������ϰ汾��֧�֣�IAR��֧�֡�
	 - Ĭ�ϲ����������Ҫʹ�ܴ�ѡ���ؿ�V5�������û��ֲ��8��
	*/
	#if Enable_EventRecorder == 1  
		/* ��ʼ��EventRecorder������ */
		EventRecorderInitialize(EventRecordAll, 1U);
		EventRecorderStart();
	#endif	
	
	//bsp_StartAutoTimer(0, 50000); /* ����1��100ms���Զ���װ�Ķ�ʱ�� */

	uint8_t motorDirection = STOP; // ��ʼ����˶�����Ϊֹͣ
	bool sensor1Active = false;
	bool sensor2Active = false;
//	MotorControl(DOWNWARD);
	MotorControl(STOP);
	static unsigned int loopdata = 0;
	
	short splace = 0;
	char cplace = 1; //Ĭ������
	
	bsp_GpioOn(LED_NUM1);
	bsp_GpioOff(LED_NUM0);
  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//		if(g_tBspUsart3.rxFlag == 1)
		if(g_tBspUsart1.rxFlag == 1)
		{
			short spped = 0;
			short place = 0;
//			g_tBspUsart3.rxFlag = 0;
			g_tBspUsart1.rxFlag = 0;
//			if(g_tBspUsart3.pRxBuf[0] == 0x55 && g_tBspUsart3.pRxBuf[1] == 0x06 && g_tBspUsart3.pRxBuf[2] == 0x01)
			if(g_tBspUsart1.pRxBuf[0] == 0x55 && g_tBspUsart1.pRxBuf[1] == 0x06 && g_tBspUsart1.pRxBuf[2] == 0x01)
			{
//				switch(g_tBspUsart3.pRxBuf[3])
				switch(g_tBspUsart1.pRxBuf[3])
				{
					case 1://����
						HAL_GPIO_DeInit(GPIOD, GPIO_PIN_12);
						bsp_SetTIMOutPWM(GPIOD, GPIO_PIN_12,  TIM4,  1, 1000, 5000); //����ģʽ
//						spped = g_tBspUsart3.pRxBuf[4]<<8 | g_tBspUsart3.pRxBuf[5];
						spped = g_tBspUsart1.pRxBuf[4]<<8 | g_tBspUsart1.pRxBuf[5];
						switch(spped)
						{
							case 1: //˳
								/*�ȸߺ��*/
 								MotorControl(UPWARD);
								bsp_SetTIMOutPWM(GPIOD, GPIO_PIN_12,  TIM4,  1, 1000, 5000);
								break;
							
							case 2:
								MotorControl(UPWARD);
								bsp_SetTIMOutPWM(GPIOD, GPIO_PIN_12,  TIM4,  1, 5000, 5000);
								break;
							
							case 3:
								MotorControl(UPWARD);
								bsp_SetTIMOutPWM(GPIOD, GPIO_PIN_12,  TIM4,  1, 10000, 5000);
								break;
							
							case -1://�� 00 01 ����1 , 00 01 ȡ�� ���� FF FE + 1 = FF FF
								MotorControl(DOWNWARD);
								bsp_SetTIMOutPWM(GPIOD, GPIO_PIN_12,  TIM4,  1, 1000, 5000);
								break;

							case -2:
								MotorControl(DOWNWARD);
								bsp_SetTIMOutPWM(GPIOD, GPIO_PIN_12,  TIM4,  1, 5000, 5000);
								break;

							case -3:
								MotorControl(DOWNWARD);
								bsp_SetTIMOutPWM(GPIOD, GPIO_PIN_12,  TIM4,  1, 10000, 5000);
								break;

							default:
								break;
						}
						break;
					
					case 2://λ��
//						place = g_tBspUsart3.pRxBuf[4]<<8 | g_tBspUsart3.pRxBuf[5];
						place = g_tBspUsart1.pRxBuf[4]<<8 | g_tBspUsart1.pRxBuf[5];
						//������������
					  if(place > 0)
						{
							//place = place / 100;
							splace = place;
							cplace = 1;
							printf("+");
							Pules_InitGpio();
							bsp_StartAutoTimer(1, 1);
						}
						else if(place < 0)
						{
							//place = place / 100;
							splace = place;
							cplace = 2;
							printf("-");
							Pules_InitGpio();
							bsp_StartAutoTimer(1, 1);
						}
						else if(place == 0)
						{
							printf("0");
						}
						break;
					
					case 3://ֹͣ
						MotorControl(STOP);
						break;
					
					default:
						break;
				}
			}
//			 memset(g_tBspUsart3.pRxBuf, 0, sizeof((unsigned int)g_tBspUsart3.pRxBuf));
			memset(g_tBspUsart1.pRxBuf, 0, sizeof((unsigned int)g_tBspUsart1.pRxBuf));
		}	
		if(bsp_CheckTimer(1))
		{
			if(cplace == 1)
			{
				unsigned int data = 32 * splace;
				if(loopdata < data)
				{
					MotorControl(UPWARD);
					loopdata++;
					HAL_GPIO_TogglePin(GPIOD , GPIO_PIN_12);
				}
				else
				{
					loopdata = 0;
					bsp_StopTimer(1);
				}
			}
			else if(cplace == 2)
			{
				unsigned int data = 32 * (-splace);
				if(loopdata < data)
				{
					MotorControl(DOWNWARD);
					loopdata++;
					HAL_GPIO_TogglePin(GPIOD , GPIO_PIN_12);
				}
				else
				{
					loopdata = 0;
					bsp_StopTimer(1);
				}
			}
		}
		
//		if (bsp_CheckTimer(0))	
//		{
//			bsp_GpioToggle(LED_NUM1);	
//			bsp_GpioToggle(LED_NUM0);		
//		}			
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}


// ������ƺ���
void MotorControl(uint8_t direction) 
{
		// ���� direction ���Ƶ�����˶�����
		// ����������Ҫʵ�־���ĵ�������߼�
		// ���� direction ���Ƶ�����˶�����
		// ����ʾ��Ϊ��������������˶�����
    if (direction == UPWARD) 
		{
			// ������������˶�
			bsp_GpioOff(DIRECTION);	//������,���ʹ��
			bsp_GpioOff(OFFLINE); 	// �������		
    } 
		else if (direction == DOWNWARD) 
		{
			// ������������˶�
			bsp_GpioOn(DIRECTION);	//������
			bsp_GpioOff(OFFLINE); 	// �������
    } 
		else if (direction == STOP)
		{
			// ֹͣ���
			bsp_GpioOn(OFFLINE);	// ������ִ��ֹͣ״̬���߼�
    }
}

// ��ȡ�¼��޴�����״̬
bool IsLimitSensor1Active() {
    // ��ȡ�ϼ��޴�������״̬������
	if(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_1) == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// ��ȡ�ϼ��޴�����״̬
bool IsLimitSensor2Active() {
	    // ��ȡ�¼��޴�������״̬������
	if(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_0) == 1)
	{
		//return true;
		return false;
	}
	else
	{
		return true;
		
	}
		//GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
