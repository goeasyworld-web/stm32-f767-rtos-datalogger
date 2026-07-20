/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "cmsis_os.h"
#include "lwip.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "ringbuffer.h"
#include <stdbool.h>
#include "sensor_msg.h"
#include "mempool.h"
#include <string.h>
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
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim5;

UART_HandleTypeDef huart3;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for blinkTask1 */
osThreadId_t blinkTask1Handle;
const osThreadAttr_t blinkTask1_attributes = {
  .name = "blinkTask1",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for blinkTask3 */
osThreadId_t blinkTask3Handle;
const osThreadAttr_t blinkTask3_attributes = {
  .name = "blinkTask3",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for sensor */
osMessageQueueId_t sensorHandle;
const osMessageQueueAttr_t sensor_attributes = {
  .name = "sensor"
};
/* Definitions for halfReadyQueue */
osMessageQueueId_t halfReadyQueueHandle;
const osMessageQueueAttr_t halfReadyQueue_attributes = {
  .name = "halfReadyQueue"
};
/* Definitions for myRecursiveMutex01 */
osMutexId_t myRecursiveMutex01Handle;
const osMutexAttr_t myRecursiveMutex01_attributes = {
  .name = "myRecursiveMutex01",
  .attr_bits = osMutexRecursive,
};
/* USER CODE BEGIN PV */
osThreadId_t ringbufferTaskHandle;
const osThreadAttr_t ringbuffertask_attributes = {
		.name = "ringbuffertask",
		.stack_size = 512*4,
		.priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t bmeSensorReadTaskHandle;
const osThreadAttr_t bmeSensorReadTaskHandle_attributes = {
		.name = "bmeSensorRead",
		.stack_size = 512*4,
		.priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t adcSmaplingTaskHandle;
const osThreadAttr_t adcSmaplingTaskHandle_attributes = {
		.name = "adcSampling",
		.stack_size = 512*4,
		.priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t sensoraggregatoreTaskHandle;
const osThreadAttr_t sensoraggregatoreTaskHandle_attributes = {
		.name = "sesnorAggregater",
		.stack_size = 512*4,
		.priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
static void MX_TIM5_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);

/* USER CODE BEGIN PFP */
void sensorAggregator(void *arguments);
extern void bmeSensorRead(void *arguments);
extern bool bme280init();
extern bool bme28ReadRaw(int32_t *raw_t);
void adcSampling(void *arguments);
void StartUartRxTask(void *argument);
extern int32_t bme280_compensate_T(int32_t adc_T);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define BATCH_SAMPLES 240
char statsbuf[512];
ringbuf_t uart_ringbuffer;
uint8_t byte_received;
uint8_t length = 1;
volatile uint32_t rx_dropped =0;
volatile uint32_t queue_rx_dropped =0;
uint16_t adc_buff[512];
volatile uint8_t adc_half_ready = 0;
volatile uint32_t pool_alloc_fail = 0;
volatile uint32_t half_ready_drop = 0;
extern struct netif gnetif;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

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
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_TIM5_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  if(HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buff, 512) != HAL_OK)
  {
	  Error_Handler();
  }
  if(!mempool_init())
  {
	  Error_Handler();
  }
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* Create the recursive mutex(es) */
  /* creation of myRecursiveMutex01 */
  myRecursiveMutex01Handle = osMutexNew(&myRecursiveMutex01_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of sensor */
  sensorHandle = osMessageQueueNew (16, 16, &sensor_attributes);

  /* creation of halfReadyQueue */
  halfReadyQueueHandle = osMessageQueueNew (4, sizeof(uint16_t), &halfReadyQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of blinkTask1 */
  blinkTask1Handle = osThreadNew(StartTask02, NULL, &blinkTask1_attributes);

  /* creation of blinkTask3 */
  blinkTask3Handle = osThreadNew(StartTask03, NULL, &blinkTask3_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  ringbufferTaskHandle = osThreadNew(StartUartRxTask, NULL, &ringbuffertask_attributes);
  //adcSmaplingTaskHandle = osThreadNew(adcSampling, NULL, &adcSmaplingTaskHandle_attributes);
  bmeSensorReadTaskHandle = osThreadNew(bmeSensorRead, NULL, &bmeSensorReadTaskHandle_attributes);
  sensoraggregatoreTaskHandle = osThreadNew(sensorAggregator, NULL, &sensoraggregatoreTaskHandle_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  hi2c1.Init.Timing = 0x20303E5D;
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
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 10799;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 4294967295;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
int _write(int file, char *ptr, int len)
{
	osMutexAcquire(myRecursiveMutex01Handle, osWaitForever);
	if (myRecursiveMutex01Handle == NULL)
	{
	    Error_Handler();
	}
  HAL_UART_Transmit(&huart3, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  osMutexRelease(myRecursiveMutex01Handle);

  return len;
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART3)
	{
		if(!rb_store(&uart_ringbuffer, byte_received))
		{
			rx_dropped++;
		}
		HAL_UART_Receive_IT(&huart3, &byte_received, length);
	}

}

void sensorAggregator(void *arguments)
{
	sensor_msg_t msg;
	osStatus_t xStatus;
	//printf("size of = %u \r\n", (unsigned)(sizeof(sensor_msg_t)));

	for(;;)
	{
		xStatus = osMessageQueueGet(sensorHandle, &msg, NULL, osWaitForever);
		if(xStatus == osOK)
		{
			switch(msg.source)
			{
				case SRC_ADC:
					if(msg.payload!=NULL)
					{
						adc_batch_t *b  = (adc_batch_t*)msg.payload;
					    printf("[%lu] ADC avg:%u min:%u max:%u\r\n",msg.timestamp, b->avg, b->min, b->max);
					    mempool_free(msg.payload);
					}
						break;
				case 2: printf("[%lu] BME temp-----: %ld.%02ld C\r\n", msg.timestamp, msg.value / 100, msg.value % 100); break;

				default: printf("[%lu] ?? src=%u val=%ld\r\n",   msg.timestamp, msg.source, msg.value);
			}
		}
	}
}

void bmeSensorRead(void *arguments)
{
	int32_t raw_t;
	sensor_msg_t bme_msg;
	osStatus_t xStatus;


    if (!bme280init())
    {
        printf("BME280 init FAILED\r\n");
        for(;;) osDelay(1000);          /* park forever — no sensor, nothing to do */
    }
	for(;;)
	{
		if(bme28ReadRaw(&raw_t))
		{
	        int32_t T = bme280_compensate_T(raw_t);          /* raw -> hundredths of °C */
	        //printf("T = %ld.%02ld C  (raw %ld)\r\n", T / 100, T % 100, raw_t);
	        bme_msg.source = SRC_BME;
	        bme_msg.timestamp = osKernelGetTickCount();
	        bme_msg.value = T;
	        xStatus = osMessageQueuePut(sensorHandle, &bme_msg, 0, 0);
	        if(xStatus!=osOK)
	        {
	        	printf("Failed to push into the queue \r\n");
	        }
		}
        else
        {
            printf("BME280 read FAILED\r\n");
        }
		osDelay(2000);
	}
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
	if(hadc->Instance == ADC1)
	{
		uint8_t which = 1;
		if(osMessageQueuePut(halfReadyQueueHandle, &which, 0, 0) != osOK)
		{
			half_ready_drop++;
		}
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if(hadc->Instance == ADC1)
	{
		uint8_t which = 2;
		if(osMessageQueuePut(halfReadyQueueHandle, &which, 0, 0) != osOK)
		{
			half_ready_drop++;
		}
	}
}

/***************************************ADC SAMPLING PINGPONG BUFFER****************************************************/
void adcSampling(void *arguments)
{
	static uint32_t batch_seq;

	uint16_t min = 0;
	uint16_t max = 0;
	uint8_t which;
	uint32_t adcSum = 0;
	sensor_msg_t adc_msg;
	osStatus_t xStatus;
	uint16_t start;
	uint16_t i = 0;
	for(;;)
	{
		osMessageQueueGet(halfReadyQueueHandle, &which, 0, osWaitForever);

		start = (which ==1)? 0:256;

		adc_batch_t *batch = (adc_batch_t*)mempool_alloc();
		if(batch == NULL)
		{
			//printf("Allocation failed %lu\r\n",pool_alloc_fail++);
			continue;
		}
		adcSum = 0;
		min = adc_buff[start];
		max = adc_buff[start];
		for(i = 0; i<BATCH_SAMPLES; i++)
		{
			adcSum = adc_buff[start+i] + adcSum;
			batch->samples[i] = adc_buff[start + i];
			if(adc_buff[start+i] < min)
			{
				min = adc_buff[i];
			}
			if(adc_buff[start + i] > max)
			{
				max = adc_buff[i];
			}
		}
		batch->max = max;
		batch->min = min;
		batch->count = BATCH_SAMPLES;
		//printf(" [count %ld] h1  %ld \r\n", osKernelGetTickCount(), (adcSum/256));
		adc_msg.source = SRC_ADC;
		adc_msg.timestamp = osKernelGetTickCount();
		//adc_msg.value = ++batch_seq;
		adc_msg.value = (uint16_t)(adcSum/BATCH_SAMPLES);
		batch->avg = adc_msg.value;
		adc_msg.payload = batch;
		xStatus = osMessageQueuePut(sensorHandle, &adc_msg, 0, 0);
		if(xStatus !=osOK)
		{
			queue_rx_dropped++;
			mempool_free(batch);
		}
	}
}
/***************************************ADC SAMPLING****************************************************/

/*******************************************UART RX CIRCULAR BUFFER****************************************************/
void StartUartRxTask(void *argument)
{

	uint8_t byte;
	char line[64];
	uint8_t index = 0;
	rb_init(&uart_ringbuffer);
	HAL_UART_Receive_IT(&huart3, &byte_received, length);

	for(;;)
	{
		if(rb_receive(&uart_ringbuffer, &byte))
		{

			if(byte == '\r' || byte == '\n')
			{
				line[index] = '\0';
				if(strcmp(line, "LED ON") == 0)
				{
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
				}
				else if(strcmp(line, "LED OFF") == 0)
				{
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
				}
				else if(strcmp(line, "status") == 0)
				{
                    uint32_t freeblocks = mempool_free_count();
                    osMutexAcquire(myRecursiveMutex01Handle, osWaitForever);
                	//if (myRecursiveMutex01Handle == NULL)
                	{
                	    //Error_Handler();
                	}
                    printf("\r\n--- STATUS ---\r\n");
                    printf("uptime         : %lu s\r\n", osKernelGetTickCount() / 1000);
                    printf("pool free      : %lu / %u\r\n", freeblocks, POOL_BLOCKS);

                    if (freeblocks == 0)
                        printf("WARNING: pool exhausted (leak or consumer stall?)\r\n");

                    printf("pool alloc fail: %lu\r\n", pool_alloc_fail);
                    printf("queue drops    : %lu\r\n", queue_rx_dropped);
                    printf("half drops    : %lu\r\n", half_ready_drop);
                    printf("uart rx drops  : %lu\r\n", rx_dropped);

                    vTaskGetRunTimeStats(statsbuf);
                    printf("%s\r\n", statsbuf);
                    osMutexRelease(myRecursiveMutex01Handle);
				}
				else if(index>0)
				{
					printf("Unknown command\r\n", line);
				}

				index = 0;
			}
			else
			{
				if(index < sizeof(line) -1)
				{
					line[index] = (char)byte;
					index++;
				}

			}
		}
		else
		{
			osDelay(1000);
		}
	}
}
/*******************************************UART RX CIRCULAR BUFFER****************************************************/
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN 5 */

	for(;;)
	{
		MX_LWIP_Process();
		sys_check_timeouts();
		printf("netif up: %d, dhcp state accessible: yes\r\n", netif_is_up(&gnetif));
		ethernet_link_check_state(&gnetif);
		if(netif_is_link_up(&gnetif))
		{
			printf("link : UP, IP: %s\r\n", ip4addr_ntoa(netif_ip4_addr(&gnetif)));
		}
		else
		{
			printf("link:Down \r\n");
		}
	osDelay(10);
	}
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the blinkTask1 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
  for(;;)
  {
	//HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
    osDelay(150);
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the blinkTask3 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void *argument)
{
  /* USER CODE BEGIN StartTask03 */
  /* Infinite loop */
  for(;;)
  {
	 //HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
    osDelay(150);
  }
  /* USER CODE END StartTask03 */
}

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
#ifdef USE_FULL_ASSERT
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
