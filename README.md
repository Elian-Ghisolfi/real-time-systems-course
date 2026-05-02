# real-time-systems-course
practical exercises and projects of the real-time systems course

## Desafio 1

**Preguntas**: Todas las tareas se ejecutan? Hay concurrencia entre tareas? Porque?. Los
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

## Desafio 2

**Preguntas:** Que diferencias hay en las transiciones de tareas respecto al caso del Desafio 1? (De ser posible medir con fines comparativos utilizando los mismos periodos de conmutacion del caso anterior).

### Analisis:

Para este desafio empezamos a parametrizar las configuraciones y los parametros de los LEDS (en el futuro seran otros perifericos) y empezamos a definir tareas mas generales para solo tener instancias de la misma

```
ESTRUCTURA DE PARAMETROS DE LOS LEDS
typedef struct {
	GPIO_TypeDef* GPIO_puerto;
	uint16_t GPIO_pin;
	uint32_t delay;
}Led_Param_t;

TAREA PARAMETRIZADA
void vTareaParpadeo(void * pvParameters){
	Led_Param_t *pxParam = (Led_Param_t *) pvParameters;

	while(1){

		HAL_GPIO_TogglePin(pxParam->GPIO_puerto, pxParam->GPIO_pin);

		vTaskDelay(pdMS_TO_TICKS(pxParam->delay));
	}

}
```
En el Desafío 1 las tareas pasaban únicamente de Ready a Running consumiendo el 100% de la CPU y desperdiciando recursos. 
Mietras que en el Desafío 2: Las tareas ahora transicionan entre Running, Blocked y Ready. Tardan apenas unos microsegundos en hacer el Toggle del pin y calcular el nuevo vTaskDelay, cediendo inmediatamente la CPU. Ahora, durante el 99.9% del tiempo, las 4 tareas están en estado Blocked. ¿Quién usa la CPU? La Idle Task (prioridad 0), la cual puede aprovecharse para poner el microcontrolador en modo "Sleep" y ahorrar energía.
