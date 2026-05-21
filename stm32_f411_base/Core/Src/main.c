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
	TickType_t delay;
	char mode_L_or_R;
}Led_pattern_t;

typedef char Mode_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define ITEM_SIZE_QUEUE_SPEED sizeof( TickType_t )
#define ITEM_SIZE_QUEUE_MODE sizeof( Mode_t )
#define INITIAL_SPEED pdMS_TO_TICKS(10)
#define INITIAL_MODE 'D'
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
Led_Param_t leds_param[4] = {{GPIOD, GPIO_PIN_12, 500},	{GPIOD, GPIO_PIN_13, 500},
							{GPIOD, GPIO_PIN_14, 500}, {GPIOD, GPIO_PIN_15, 500}};

QueueSetHandle_t xqueueSet;
QueueHandle_t queue_speed = NULL;
QueueHandle_t queue_mode = NULL;
QueueHandle_t queue_leds = NULL;
SemaphoreHandle_t sem_button = NULL;

Mode_t initial_mode;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

void vProducer_Speed_Task(void * pvParameters);
void vProducer_Mode_Task(void * pvParameters);
void vProcessTask(void * pvParameters);
void vPattern_Leds(void * pvParameters);

/* USER CODE BEGIN PFP */

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

  queue_speed = xQueueCreate(4, ITEM_SIZE_QUEUE_SPEED);
  queue_mode = xQueueCreate(4, ITEM_SIZE_QUEUE_MODE);
  queue_leds = xQueueCreate(4, sizeof(Led_pattern_t));
  sem_button = xSemaphoreCreateBinary();

  xqueueSet = xQueueCreateSet((UBaseType_t) 8);

  xQueueAddToSet(queue_mode, xqueueSet);
  xQueueAddToSet(queue_speed, xqueueSet);


  xTaskCreate(vProducer_Mode_Task, "Modo", 128, NULL, 2, NULL);
  xTaskCreate(vProducer_Speed_Task, "Velocidad", 128, NULL, 2, NULL);
  xTaskCreate(vProcessTask, "Core", 256, NULL, 1, NULL); // Prioridad baja para consumir

  xTaskCreate(vPattern_Leds, "Leds", 128, NULL, 1, NULL);

  /* Start scheduler */
  vTaskStartScheduler();

  /* USER CODE END 2 */

  /* Init scheduler */


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

void vProducer_Speed_Task(void * pvParameters){
	TickType_t random_speed;

	while(1){
		random_speed = pdMS_TO_TICKS(100 + rand() % 901);
		xQueueSend(queue_speed, &random_speed, portMAX_DELAY);

		vTaskDelay(pdMS_TO_TICKS(3000));
	}
}

void vProducer_Mode_Task(void * pvParameters){
	Mode_t pMode = INITIAL_MODE;

	while(1){

		if(xSemaphoreTake(sem_button, portMAX_DELAY)){
			if(pMode == 'D'){
				pMode = 'I';
				xQueueSend(queue_mode, &pMode, portMAX_DELAY);
			}else{
				pMode = 'D';
				xQueueSend(queue_mode, &pMode, portMAX_DELAY);
			}
		}
	}
}

void vProcessTask(void * pvParameters){
	TickType_t speed = INITIAL_SPEED;
	Mode_t mode = INITIAL_MODE;
	Led_pattern_t pattern;

	QueueSetMemberHandle_t xActivatedMember;

	while(1){
		xActivatedMember = xQueueSelectFromSet(xqueueSet, portMAX_DELAY);

		if(xActivatedMember == queue_speed){

			xQueueReceive(queue_speed, &speed, 0);
			pattern.delay = speed;
			pattern.mode_L_or_R = mode;
			xQueueSend(queue_leds, &pattern, 0);

		}else if(xActivatedMember == queue_mode){

			xQueueReceive(queue_mode, &mode, 0);
			pattern.delay = speed;
			pattern.mode_L_or_R = mode;
			xQueueSend(queue_leds, &pattern, 0);
		}
	}
}

void vPattern_Leds(void * pvParameters){
	Led_pattern_t led_Pattern;
	Led_pattern_t aux;
	led_Pattern.delay = INITIAL_SPEED;
	led_Pattern.mode_L_or_R = initial_mode;
	int8_t led_pos = 0;
	TickType_t xLastWakeTime = xTaskGetTickCount();

	while(1){

		if(xQueueReceive(queue_leds, &aux, 0) == pdPASS){
			led_Pattern.delay = aux.delay;
			led_Pattern.mode_L_or_R = aux.mode_L_or_R;
		}



		// Lógica de desplazamiento
		if(led_Pattern.mode_L_or_R == 'D') {
			led_pos++;
			if(led_pos > 3) led_pos = 0;
		} else {
			led_pos--;
			if(led_pos < 0) led_pos = 3;
		}

		// Apagar todos los LEDs
		for (int var = 0; var < 4; ++var) {
			HAL_GPIO_WritePin(leds_param[var].GPIO_puerto, leds_param[var].GPIO_pin, GPIO_PIN_RESET);
		}

		// Encender el correspondiente
		if(led_pos == 0) HAL_GPIO_WritePin(leds_param[0].GPIO_puerto, leds_param[0].GPIO_pin, GPIO_PIN_SET);
		if(led_pos == 1) HAL_GPIO_WritePin(leds_param[1].GPIO_puerto, leds_param[1].GPIO_pin, GPIO_PIN_SET);
		if(led_pos == 2) HAL_GPIO_WritePin(leds_param[2].GPIO_puerto, leds_param[2].GPIO_pin, GPIO_PIN_SET);
		if(led_pos == 3) HAL_GPIO_WritePin(leds_param[3].GPIO_puerto, leds_param[3].GPIO_pin, GPIO_PIN_SET);

		vTaskDelayUntil(&xLastWakeTime ,led_Pattern.delay);
	}
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	if(GPIO_Pin == GPIO_PIN_0){
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		xSemaphoreGiveFromISR(sem_button, &xHigherPriorityTaskWoken);

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
