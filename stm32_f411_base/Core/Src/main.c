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
#include "timers.h"
#include <limits.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
	GPIO_TypeDef* GPIO_puerto;
	uint16_t GPIO_pin;
	uint32_t delay;
}Led_Param_t;

typedef enum {
    STATE_INACTIVE,
    STATE_ARMING,
    STATE_ARMED
} AlarmState_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define LED_PRE_ARMADO     0
#define LED_SISTEMA_ARMADO 1

#define BUTTON_SIGNAL           (1 << 0)
#define ONESHOT_TIMER_SIGNAL    (1 << 1)

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
Led_Param_t leds_param[4] = {{GPIOD, GPIO_PIN_12, 100},	{GPIOD, GPIO_PIN_13, 100},
							{GPIOD, GPIO_PIN_14, 100}, {GPIOD, GPIO_PIN_15, 100}};

//SemaphoreHandle_t xSemButton = NULL;

TimerHandle_t xAutoReloadAlarmTimer;
TimerHandle_t xOneShotAlarmTimer;

TaskHandle_t xArmingTaskHandle;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void prvAutoReloadAlarmCallback(TimerHandle_t xTimer);
void prvOneShotAlarmCallback(TimerHandle_t xTimer);

void vArmingAlarmTask(void *pvParameters);

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

  xAutoReloadAlarmTimer = xTimerCreate(
		  "Armado Alarma",
		  pdMS_TO_TICKS(100),
		  pdTRUE,
		  (void *)0,
		  prvAutoReloadAlarmCallback);

  xOneShotAlarmTimer = xTimerCreate(
		  "Armado Alarma",
		  pdMS_TO_TICKS(10000),
		  pdFALSE,
		  (void *)0,
		  prvOneShotAlarmCallback);

  xTaskCreate(vArmingAlarmTask, "Gestion alarma", 100, NULL, 1, &xArmingTaskHandle);

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

void prvAutoReloadAlarmCallback(TimerHandle_t xTimer){
	HAL_GPIO_TogglePin(leds_param[LED_PRE_ARMADO].GPIO_puerto, leds_param[LED_PRE_ARMADO].GPIO_pin);
}

void prvOneShotAlarmCallback(TimerHandle_t xTimer){
	xTimerStop(xAutoReloadAlarmTimer, 0);
	xTaskNotify(xArmingTaskHandle, ONESHOT_TIMER_SIGNAL, eSetBits);

}

void vArmingAlarmTask(void *pvParameters){
    AlarmState_t currentState = STATE_INACTIVE;
    uint32_t signal;

    while(1){
        xTaskNotifyWait(0, ULONG_MAX, &signal, portMAX_DELAY);

        // Evaluamos QUÉ HACER dependiendo del ESTADO ACTUAL
        switch (currentState) {

            case STATE_INACTIVE:
            case STATE_ARMED:
            	if(signal == BUTTON_SIGNAL){
					xTimerStart(xOneShotAlarmTimer, 0);
					xTimerStart(xAutoReloadAlarmTimer, 0);

					HAL_GPIO_WritePin(leds_param[LED_SISTEMA_ARMADO].GPIO_puerto,
									  leds_param[LED_SISTEMA_ARMADO].GPIO_pin, GPIO_PIN_RESET);

					currentState = STATE_ARMING;
            	}
                break;

            case STATE_ARMING:
            	// Cubrimos en la tarea los eventos posibles de BOTON o de ONE SHOT TIMER
            	if(signal == BUTTON_SIGNAL){
					xTimerStop(xAutoReloadAlarmTimer, 0);
					xTimerStop(xOneShotAlarmTimer, 0);

					HAL_GPIO_WritePin(leds_param[LED_PRE_ARMADO].GPIO_puerto,
									  leds_param[LED_PRE_ARMADO].GPIO_pin, GPIO_PIN_RESET);

					currentState = STATE_INACTIVE;
            	} else if(signal == ONESHOT_TIMER_SIGNAL){

					xTimerStop(xAutoReloadAlarmTimer, 0);
					HAL_GPIO_WritePin(leds_param[LED_SISTEMA_ARMADO].GPIO_puerto,
									  leds_param[LED_SISTEMA_ARMADO].GPIO_pin, GPIO_PIN_SET);

					HAL_GPIO_WritePin(leds_param[LED_PRE_ARMADO].GPIO_puerto,
									  leds_param[LED_PRE_ARMADO].GPIO_pin, GPIO_PIN_RESET);
					currentState = STATE_ARMED;
            	}
                break;
        }
    }
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	if(GPIO_Pin == GPIO_PIN_0){
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		xTaskNotifyFromISR(xArmingTaskHandle, BUTTON_SIGNAL, eSetBits, &xHigherPriorityTaskWoken);

		//xSemaphoreGiveFromISR(xSemButton, &xHigherPriorityTaskWoken);

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
