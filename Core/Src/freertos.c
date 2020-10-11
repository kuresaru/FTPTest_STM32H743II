/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ftp.h"
#include "lwip.h"
#include "usart.h"
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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId ftpTaskHandle;
osThreadId printThreadHandle;
osMessageQId printQueueHandle;
uint8_t printQueueBuffer[ 64 * sizeof( uint8_t ) ];
osStaticMessageQDef_t printQueueControlBlock;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartFtpTask(void const * argument);
void PrintTask(void const * argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

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
  /* definition and creation of printQueue */
  osMessageQStaticDef(printQueue, 64, uint8_t, printQueueBuffer, &printQueueControlBlock);
  printQueueHandle = osMessageCreate(osMessageQ(printQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 1024);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of ftpTask */
  osThreadDef(ftpTask, StartFtpTask, osPriorityNormal, 0, 1024);
  ftpTaskHandle = osThreadCreate(osThread(ftpTask), NULL);

  /* definition and creation of printThread */
  osThreadDef(printThread, PrintTask, osPriorityLow, 0, 256);
  printThreadHandle = osThreadCreate(osThread(printThread), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartDefaultTask */
  printf("\033[2J\033[0;0HStarting...\r\n");
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartFtpTask */
/**
* @brief Function implementing the ftpTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartFtpTask */
void StartFtpTask(void const * argument)
{
  /* USER CODE BEGIN StartFtpTask */
  while (!netif_is_up(&gnetif))
  {
    osDelay(1);
  }
  ftp_init();
  while (ftp_status != NOOP)
  {
    osDelay(1);
  }
  char buf[512];
  uint16_t read;
  ftp_getfile("test.txt", buf, 128, &read);
  while (ftp_status != NOOP)
  {
    osDelay(1);
  }
  buf[read] = '\0';
  printf("%s\r\n", buf);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartFtpTask */
}

/* USER CODE BEGIN Header_PrintTask */
/**
* @brief Function implementing the printThread thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_PrintTask */
void PrintTask(void const * argument)
{
  /* USER CODE BEGIN PrintTask */
  osEvent status;
  /* Infinite loop */
  for(;;)
  {
    status = osMessageGet(printQueueHandle, osWaitForever);
    if (status.status == osEventMessage)
    {
      uint8_t c = (uint8_t)status.value.v;
      while (HAL_UART_GetState(&huart1) == HAL_UART_STATE_BUSY_TX)
      {
      }
      HAL_UART_Transmit(&huart1, &c, 1, 1000);
    }
  }
  /* USER CODE END PrintTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
