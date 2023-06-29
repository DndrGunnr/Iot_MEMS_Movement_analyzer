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
//EDGE VERSION
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mpu6050.h"
#include "defines_WIFI.h"
#include "ESP8266.h"
#include <string.h>
#include <math.h>
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
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
MPU6050_t MPU6050;
Wifi_t	Wifi;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
static void calibration(OFFSET_ACCEL_t * offset_accel, OFFSET_ACCEL_t * offset_gyro);
void send_data();
//function to calculate moving average
float movingAverage(float[],int,float);
//used to calculate acceleration not directed on a particular axis
float module(float, float, float);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char string[1024]="\0";
//to sum on top of accelerometer measurement
#define OFFX (-0.02)
#define OFFY 0.008
#define OFFZ 0.02
#define NCAMP 10
OFFSET_ACCEL_t offset_accel;
OFFSET_ACCEL_t offset_gyro;


#ifdef TIMING
unsigned long interval = 0;
int measures = 0;
#endif
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  //int wifi comunication with ESP01S
  DWT_Delay_Init();
  Wifi_Restart();
  while(!Wifi_Init())
  {
	  while(!Wifi_Restart());
  }

  HAL_Delay(500);
  while(!Wifi_SetMode(WifiMode_Station));
  while(!Wifi_Station_ConnectToAp(SSID_W,PASSWD_W,NULL));
  HAL_Delay(500);
  Wifi_TcpIp_SetMultiConnection(1);
  Wifi_TcpIp_StartTcpConnection(0,ACCSESS_POINT_IP,PORT_AP,10);

  //initialize MPU6050

  while (MPU6050_Init(&hi2c1) == 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  char temp;
#ifndef TIMING
  HAL_TIM_Base_Start_IT(&htim2);
#endif
  while (1)
  {
#ifdef TIMING
	  send_data();
#else
	  HAL_Delay(500);
#endif
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00300F38;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 32;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(EN_ESP8266_GPIO_Port, EN_ESP8266_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : EN_ESP8266_Pin */
  GPIO_InitStruct.Pin = EN_ESP8266_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(EN_ESP8266_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD3_Pin */
  GPIO_InitStruct.Pin = LD3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD3_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)
{
	Wifi_RxCallBack();
}

#define CALIB_CYCLES 10

void instruct_and_read(char * message, OFFSET_ACCEL_t * avg) {
	int i = 0;
	char command;

	HAL_UART_Transmit(&huart2, message, strlen(message), HAL_MAX_DELAY);
	HAL_UART_Receive(&huart2, &command, 1, HAL_MAX_DELAY);

	avg->offset_x = 0;
	avg->offset_y = 0;
	avg->offset_z = 0;

	for (i = 0; i < CALIB_CYCLES; i++) {
		MPU6050_Read_Accel(&hi2c1, &MPU6050);

		avg->offset_x += MPU6050.Ax;
		avg->offset_y += MPU6050.Ay;
		avg->offset_z += MPU6050.Az;
	}

	avg->offset_x /= CALIB_CYCLES;
	avg->offset_y /= CALIB_CYCLES;
	avg->offset_z /= CALIB_CYCLES;
}

void read_gyro(OFFSET_ACCEL_t * avg) {
	int i;
	int calib = CALIB_CYCLES * 10;
	for (int i = 0; i < calib; i++) {
		MPU6050_Read_Gyro(&hi2c1, &MPU6050);

		avg->offset_x += MPU6050.Gx;
		avg->offset_y += MPU6050.Gy;
		avg->offset_z += MPU6050.Gz;
	}

	avg->offset_x /= calib;
	avg->offset_y /= calib;
	avg->offset_z /= calib;
}

static void calibration(OFFSET_ACCEL_t * offset_accel, OFFSET_ACCEL_t * offset_gyro) {

	OFFSET_ACCEL_t positive;
	OFFSET_ACCEL_t negative;
	OFFSET_ACCEL_t avg;

	char message[] = "Press any button after each instruction to continue...\r\n";
	HAL_UART_Transmit(&huart2, message, strlen(message), HAL_MAX_DELAY);

	instruct_and_read("Keep sensor horizontal and still\r\n", &avg);
	positive.offset_z = 1.0 - avg.offset_z;

	read_gyro(&avg);
	offset_gyro->offset_x = - avg.offset_x;
	offset_gyro->offset_y = - avg.offset_y;
	offset_gyro->offset_z = - avg.offset_z;

	instruct_and_read("Keep sensor upside down\r\n", &avg);
	negative.offset_z = -1.0 - avg.offset_z;

	instruct_and_read("Orient sensor on positive x axis\r\n", &avg);
	positive.offset_x = 1.0 - avg.offset_x;
	instruct_and_read("Orient sensor on negative x axis\r\n", &avg);
	negative.offset_x = -1.0 - avg.offset_x;

	instruct_and_read("Orient sensor on positive y axis\r\n", &avg);
	positive.offset_y = 1.0 - avg.offset_y;
	instruct_and_read("Orient sensor on negative y axis\r\n", &avg);
	negative.offset_y = -1.0 - avg.offset_y;

	if (fabsf(positive.offset_x - negative.offset_x) > 0.05 ||
		fabsf(positive.offset_y - negative.offset_y) > 0.05 ||
		fabsf(positive.offset_z - negative.offset_z) > 0.05) {
		char error[] = "[WARNING] Positive and negative offsets are different!\r\n";
		HAL_UART_Transmit(&huart2, error, strlen(error), HAL_MAX_DELAY);
	}

	offset_accel->offset_x = (positive.offset_x + negative.offset_x) / 2.0;
	offset_accel->offset_y = (positive.offset_y + negative.offset_y) / 2.0;
	offset_accel->offset_z = (positive.offset_z + negative.offset_z) / 2.0;

}

float module(float acc_x, float acc_y, float acc_z){

	return sqrt(pow(acc_x+OFFX,2)+pow(acc_y+OFFY,2)+pow(acc_z+OFFZ,2));
}

float movingAverage(float buffer[],int oldest, float newValue){
	float sum=0;
	buffer[oldest]=newValue;
	oldest++;
	oldest=oldest%NCAMP;
	for(int i=0; i<NCAMP; i++)
		sum+=buffer[i];
	return sum/NCAMP;
}



void send_data() {
#ifdef TIMING
	unsigned int tick = HAL_GetTick();
#endif
	//buffer to hold data from module measurement
	float mAvgBuffer[NCAMP];
	//all elements equal to zero
	for(int i = 0; i<NCAMP; i++)
		mAvgBuffer[i]=0;
	int oldestBufferElement=0;
	float mAvg;
	MPU6050_Read_All(&hi2c1, &MPU6050);
	char mybuffer[1000];
	float mod;

	mod=module(MPU6050.Ax,MPU6050.Ay,MPU6050.Az);

	mAvg=movingAverage(mAvgBuffer,oldestBufferElement, mod);






	if(mAvg>=1.5){
		sprintf(mybuffer, "Attenzione accelerazione >1.5g :  = %fg \n\r", mAvg);
		Wifi_Transmit(0, strlen(mybuffer), mybuffer);
	}
	else{
		sprintf(mybuffer, "Accelerazione nella norma (<1.5g) : =%fg \n\r",mAvg);
		Wifi_Transmit(0, strlen(mybuffer), mybuffer);
	}


#ifdef TIMING
	interval += (HAL_GetTick() - tick);
	if (++measures >= 100) {
		char temp[100];
		sprintf(temp, "Time: %f ms\r\n", ((double)interval) / (double) measures);
		HAL_UART_Transmit(&huart2, temp, strlen(temp), HAL_MAX_DELAY);
		measures = 0;
		interval = 0;
	}
#endif
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * htim) {
	send_data();
}
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
