# real-time-systems-course Guía 3 de Trabajos Prácticos
practical exercises and projects of the real-time systems course.

## Desafío 1

**Funcionamiento:** Tener servicios funcionando para la recepción `RxTask` y la transferencia `TxTask` de datos por puerto serie via Puerto COM Virtual (Virtual COM Port). 

```c
void vTareaBoton(void *pvParameters){
	const char *pcMSG = "[EVENTO] Pulsador accionado\r\n";

	while(1){

		if(xSemaphoreTake(xButton_Sem, portMAX_DELAY) == pdPASS){

            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
			usb_transmit_buffer_safe(pcMSG);

			// Anti Bouncing
			xSemaphoreTake(xButton_Sem, 0);
			vTaskDelay(pdMS_TO_TICKS(100));
		}
	}
}
uint8_t usb_transmit_buffer_safe(const char *pcString){
	uint8_t status;
	uint16_t len = 0U;

	if(xSemaphoreTake(xUSB_Tx_Mutex, portMAX_DELAY) == pdTRUE){

		while(pcString[len] != '\0') len++;
		do {
			status = CDC_Transmit_FS((uint8_t*)pcString, len);

			if(status == USBD_BUSY) {
				// Liberamos el CPU para despues volver a iterar

				vTaskDelay(pdMS_TO_TICKS(1));
			}
		} while(status == USBD_BUSY);

		// Liberamos el Mutex
		xSemaphoreGive(xUSB_Tx_Mutex);
	}
	return USBD_OK;
}
```

## Desafío 1

**Preguntas**: Es correcto el funcionamiento observado? Los caracteres se muestran en el orden que se enviaron? Los mensajes por terminal están corruptos? Porque?

### Análisis:

```c
uint8_t usb_transmit_buffer_safe(const char *pcString){
	uint8_t status;
	uint16_t len = 0U;

	if(xSemaphoreTake(xUSB_Tx_Mutex, portMAX_DELAY) == pdTRUE){

		while(pcString[len] != '\0') len++; // Standard de C
		do {
			status = CDC_Transmit_FS((uint8_t*)pcString, len);

			if(status == USBD_BUSY) {
				// Liberamos el CPU para despues volver a iterar

				vTaskDelay(pdMS_TO_TICKS(1));
			}
		} while(status == USBD_BUSY);

		// Liberamos el Mutex
		xSemaphoreGive(xUSB_Tx_Mutex);
	}
	return USBD_OK;
}

void vTareaProcesadora(void *pvParameters) {
    char rxChar;
    char txBuffer[80]; // Buffer local para armar el mensaje
    UBaseType_t pendingCount; // Variable para almacenar la cantidad 'N'

    while(1) {

        if(xQueueReceive(xUSB_Rx_Queue, &rxChar, portMAX_DELAY) == pdPASS) {

            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
            vTaskDelay(pdMS_TO_TICKS(50));
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);


            pendingCount = uxQueueMessagesWaiting(xUSB_Rx_Queue);

            snprintf(txBuffer, sizeof(txBuffer), "Recibido:'%c' - Caracteres pendientes:%u \r\n", rxChar, (unsigned int)pendingCount);

            usb_transmit_buffer_safe(txBuffer);
        }
    }
}
```