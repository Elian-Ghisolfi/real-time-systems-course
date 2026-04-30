# real-time-systems-course
practical exercises and projects of the real-time systems course

## Desafio 1

Preguntas: Todas las tareas se ejecutan? Hay concurrencia entre tareas? Porque?. Los
tiempos de conmutación de los leds son precisos? (Medirlos de ser posible utilizando
periodos de conmutacion mas pequños: 20ms, 40ms, 60ms y 80ms)

### Analisis: 

Para este desafio realizamos 4 prototipos de tareas para dejar en claro una *Mala practica* y no utilizar la posibilidad de pasarle parametros a las mismas.

```
void vTaskLed1(void * pvParameters);
void vTaskLed2(void * pvParameters);
void vTaskLed3(void * pvParameters);
void vTaskLed4(void * pvParameters);

void vTaskLed1(void * pvParameters){
	uint32_t Delay = 200;

	while(1){
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12); // Green

		HAL_Delay(Delay);
	}
}

main(){
    xTaskCreate(vTaskLed1, "Task1", 100, NULL, 0, NULL);
    xTaskCreate(vTaskLed2, "Task2", 100, NULL, 0, NULL);
    xTaskCreate(vTaskLed3, "Task3", 100, NULL, 0, NULL);
    xTaskCreate(vTaskLed4, "Task4", 100, NULL, 0, NULL);
}
```
En cada una declaramos el valor de delay necesario cuando podriamos haberlo pasado por parametro.
Este programa utiliza el servicio `HAL_Delay()` que es una funcion que simula un loop requiriendo tiempo de procesamiento del micro hasta que termine el mismo.
Lo que vemos como resultado de tener las 4  tareas creadas con la misma prioridad y tratando de correr generando una sensacion de concurrencia pero esto es producto de la configuracion de nuestro micro precisamente al Time Slicing: Si hay dos tareas de la misma prioridad listas, el scheduler alternará entre ellas en cada iteración del Tick Interrupt (Round Robin).

Los tiempos que vemos no son precisos ya que al tener todas la misma prioridad el micro le da una porcion de procesamiento a cada una y ninguna termina su loop al tiempo que determinamos. 