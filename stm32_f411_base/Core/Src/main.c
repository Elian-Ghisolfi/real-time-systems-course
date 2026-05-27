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
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include <stdlib.h> // Para la función rand()
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
	GPIO_TypeDef* GPIO_puerto;
	uint16_t GPIO_pin;
	uint32_t delay;
}Led_Param_t;

typedef struct{
	uint16_t led_count;
}Led_counter_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define ITEM_SIZE_GLOBAL_QUEUE sizeof( Led_counter_t )
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
Led_Param_t leds_param[4] = {{GPIOD, GPIO_PIN_12, 5000},	{GPIOD, GPIO_PIN_13, 5000},
							{GPIOD, GPIO_PIN_14, 5000}, {GPIOD, GPIO_PIN_15, 5000}};

QueueSetHandle_t xQueueSet;
QueueHandle_t xQueueLed1;
QueueHandle_t xQueueLed2;

QueueHandle_t xGlobalQueue;

SemaphoreHandle_t xSemButton = NULL;

SemaphoreHandle_t xSemControl1 = NULL;
SemaphoreHandle_t xSemControl2 = NULL;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void vProducerCountTask(void *pvParameters);
void vProcessLed1Task(void *pvParameters);
void vProcessLed2Task(void *pvParameters);
void vTrashCountTask(void *pvParameters);

void vPattern_Leds(void *pvParameters);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  /* USER CODE BEGIN 2 */

  xSemButton = xSemaphoreCreateBinary();
  xSemControl1 = xSemaphoreCreateBinary();
  xSemControl2 = xSemaphoreCreateBinary();

  xQueueLed1 = xQueueCreate(5, ITEM_SIZE_GLOBAL_QUEUE);
  xQueueLed2 = xQueueCreate(5, ITEM_SIZE_GLOBAL_QUEUE);
  xGlobalQueue = xQueueCreate(1, ITEM_SIZE_GLOBAL_QUEUE);

  xQueueSet = xQueueCreateSet((UBaseType_t) 10);
  xQueueAddToSet(xQueueLed1, xQueueSet);
  xQueueAddToSet(xQueueLed2, xQueueSet);


  xTaskCreate(vProducerCountTask, "Producer Count", 100, NULL, 2, NULL);

  xTaskCreate(vProcessLed1Task, "Process Led1", 100, NULL, 1, NULL);
  xTaskCreate(vProcessLed2Task, "Process Led2", 100, NULL, 1, NULL);

  xTaskCreate(vTrashCountTask, "Task Recolect", 100, NULL, 1, NULL);
  xTaskCreate(vPattern_Leds, "Leds", 100, NULL, 1, NULL);

  /* Start scheduler */
  vTaskStartScheduler();

  /* USER CODE END 2 */

  /* Init scheduler */
  /* Start scheduler */
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
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void vProducerCountTask(void * pvParameters){
	uint16_t aux_count = 0;

	while(1){

		if(xSemaphoreTake(xSemButton, portMAX_DELAY) == pdPASS){
			aux_count = aux_count + 1;
			xQueueOverwrite(xGlobalQueue, &aux_count);
			// Bloqueamos la tarea 50ms para ignorar ruidos mecánicos.
			vTaskDelay(pdMS_TO_TICKS(300));
			// "Limpiamos" el semáforo por si la interrupción se disparó
			// durante los 200ms de delay y dejó el semáforo en verde de forma errónea.
			xSemaphoreTake(xSemButton, 0);
		}
	}
}
void vProcessLed1Task(void *pvParameters){
	uint16_t count_peek = 0;
	uint16_t led_pos = 0;

	while(1){
		xQueuePeek(xGlobalQueue, &count_peek, portMAX_DELAY);
		led_pos = count_peek % 2;
		if(led_pos == 1 ){
			xQueueSend(xQueueLed1, &led_pos, 0);
		}
		xSemaphoreGive(xSemControl1);

		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

void vProcessLed2Task(void *pvParameters){
	uint16_t count_peek = 0;
	uint16_t led_pos = 0;

	while(1){
		xQueuePeek(xGlobalQueue, &count_peek, portMAX_DELAY);
		led_pos = count_peek % 2;
		if(led_pos == 0 ){
			xQueueSend(xQueueLed2, &led_pos, 0);
		}
		xSemaphoreGive(xSemControl2);

		vTaskDelay(pdMS_TO_TICKS(100));
	}
}
void vTrashCountTask(void *pvParameters){
	uint16_t aux_trash_count;

	while(1){
		xSemaphoreTake(xSemControl1, portMAX_DELAY);
		xSemaphoreTake(xSemControl2, portMAX_DELAY);

		xQueueReceive(xGlobalQueue, &aux_trash_count, 0);
	}
}

void vPattern_Leds(void * pvParameters){
	int8_t led_pos = 3;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	QueueSetMemberHandle_t xActivatedMember;

	while(1){

		xActivatedMember = xQueueSelectFromSet(xQueueSet, portMAX_DELAY);
		if(xActivatedMember == xQueueLed1){
			xQueueReceive(xQueueLed1, &led_pos, 0);
		}else if(xActivatedMember == xQueueLed2){
			xQueueReceive(xQueueLed2, &led_pos, 0);
		}

		// Apagar todos los LEDs
		for (int var = 0; var < 4; ++var) {
			HAL_GPIO_WritePin(leds_param[var].GPIO_puerto, leds_param[var].GPIO_pin, GPIO_PIN_RESET);
		}

		// Encender el correspondiente
		if(led_pos == 0) HAL_GPIO_WritePin(leds_param[0].GPIO_puerto, leds_param[0].GPIO_pin, GPIO_PIN_SET);
		if(led_pos == 1) HAL_GPIO_WritePin(leds_param[2].GPIO_puerto, leds_param[2].GPIO_pin, GPIO_PIN_SET);

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
	}
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	if(GPIO_Pin == GPIO_PIN_0){
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		xSemaphoreGiveFromISR(xSemButton, &xHigherPriorityTaskWoken);

		/* 5. Si xHigherPriorityTaskWoken se puso en pdTRUE, forzamos un cambio de contexto
		* para que al salir de la interrupción entremos directo a la tarea del botón. */
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}

}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM9 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM9)
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
