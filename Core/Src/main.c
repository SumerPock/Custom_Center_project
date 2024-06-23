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

/*透明符传输协议*/
struct Uart_DataPacket RemoteDataPacket = {0 , 0 , 0 , {0} , {0}};
struct MOTOT_CONTROL SetMotot_control = {0};
int conditions[5] = {0};

// 定义电机运动方向
#define UPWARD 0
#define DOWNWARD 1
#define STOP 2

// 函数原型
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
	uint8_t ucKeyCode;		/* 按键代码 */
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
	bsp_GpioOn(DIRECTION);	//脱机
	bsp_GpioOn(OFFLINE);		//方向

//	bsp_GpioOff(DIRECTION);	//脱机
//	bsp_Gpio0ff(OFFLINE);		//方向	
	bsp_GpioOff(LED_NUM0);
	bsp_GpioOff(LED_NUM1);

	bsp_GpioOn(MOTORPOW);
	bsp_GpioOn(FILLLIGHT);
	bsp_GpioOn(SWITCHNF);

	bsp_GpioOff(OFFLINE);
	
	MX_DMA_Init();

	/*与上位机链接*/
	MX_USART3_UART_Init();
	/**UART DMA config**/
	//HAL_UART_Transmit(&husart1, (uint8_t *)"this is uart1 send data\r\n", 24, 1000);	
	__HAL_UART_ENABLE_IT(&husart3 , UART_IT_IDLE);	/* 开启串口空闲中断 */

	MX_USART1_UART_Init();
	/**UART DMA config**/
	//HAL_UART_Transmit(&husart1, (uint8_t *)"this is uart1 send data\r\n", 24, 1000);	
	__HAL_UART_ENABLE_IT(&husart1 , UART_IT_IDLE);	/* 开启串口空闲中断 */
	
	
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
	bsp_InitTimer();  	/* 初始化滴答定时器 */
	/* 
	 Event Recorder：
	 - 可用于代码执行时间测量，MDK5.25及其以上版本才支持，IAR不支持。
	 - 默认不开启，如果要使能此选项，务必看V5开发板用户手册第8章
	*/
	#if Enable_EventRecorder == 1  
		/* 初始化EventRecorder并开启 */
		EventRecorderInitialize(EventRecordAll, 1U);
		EventRecorderStart();
	#endif	
	
	//bsp_StartAutoTimer(0, 50000); /* 启动1个100ms的自动重装的定时器 */

	uint8_t motorDirection = STOP; // 初始电机运动方向为停止
	bool sensor1Active = false;
	bool sensor2Active = false;
//	MotorControl(DOWNWARD);
	MotorControl(STOP);
	static unsigned int loopdata = 0;
	
	short splace = 0;
	char cplace = 1; //默认正数
	
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
					case 1://定速
						HAL_GPIO_DeInit(GPIOD, GPIO_PIN_12);
						bsp_SetTIMOutPWM(GPIOD, GPIO_PIN_12,  TIM4,  1, 1000, 5000); //定速模式
//						spped = g_tBspUsart3.pRxBuf[4]<<8 | g_tBspUsart3.pRxBuf[5];
						spped = g_tBspUsart1.pRxBuf[4]<<8 | g_tBspUsart1.pRxBuf[5];
						switch(spped)
						{
							case 1: //顺
								/*先高后低*/
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
							
							case -1://逆 00 01 代表1 , 00 01 取反 代表 FF FE + 1 = FF FF
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
					
					case 2://位置
//						place = g_tBspUsart3.pRxBuf[4]<<8 | g_tBspUsart3.pRxBuf[5];
						place = g_tBspUsart1.pRxBuf[4]<<8 | g_tBspUsart1.pRxBuf[5];
						//区分下正负数
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
					
					case 3://停止
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


// 电机控制函数
void MotorControl(uint8_t direction) 
{
		// 根据 direction 控制电机的运动方向
		// 在这里你需要实现具体的电机控制逻辑
		// 根据 direction 控制电机的运动方向
		// 这里示例为启动电机并设置运动方向
    if (direction == UPWARD) 
		{
			// 启动电机向上运动
			bsp_GpioOff(DIRECTION);	//正方向,电机使能
			bsp_GpioOff(OFFLINE); 	// 电机开启		
    } 
		else if (direction == DOWNWARD) 
		{
			// 启动电机向下运动
			bsp_GpioOn(DIRECTION);	//正方向
			bsp_GpioOff(OFFLINE); 	// 电机开启
    } 
		else if (direction == STOP)
		{
			// 停止电机
			bsp_GpioOn(OFFLINE);	// 在这里执行停止状态的逻辑
    }
}

// 读取下极限传感器状态
bool IsLimitSensor1Active() {
    // 读取上极限传感器的状态并返回
	if(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_1) == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// 读取上极限传感器状态
bool IsLimitSensor2Active() {
	    // 读取下极限传感器的状态并返回
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
