# real-time-systems-course
practical exercises and projects of the real-time systems course.

Este espacio está pensado para mostrar el aprendizaje progresivo sobre los conceptos fundamentales de los sistemas operativos en tiempo real (RTOS), implementados en microcontroladores de la familia STM32.

---

## 📁 Carperta de Reportes

### 📘 [Guía 1: Fundamentos de Tareas, Tiempos y Sincronización Básica](reports/INFORME_GUIA1.md)
En este primer reporte se aborda la creación y gestión del ciclo de vida de las tareas en FreeRTOS. Los principales temas que se analizan son:
* **Gestión de Tareas y Estados:** Creación de tareas (paso de parámetros para reutilizar código) y observación del flujo entre estados (*Running, Blocked, Ready, Suspended*) para liberar el 99.9% de la CPU.
* **Manejo del Tiempo:** Diferencias prácticas y visuales entre usar tiempos relativos (`vTaskDelay`), absolutos (`xTaskDelayUntil`) y demoras bloqueantes (`HAL_Delay`).
* **Prioridades y Starvation:** Manipulación dinámica de prioridades en tiempo de ejecución para evitar la inanición de tareas menores.
* **Interrupciones (ISR) y Procesamiento Diferido:** Migración del ineficiente mecanismo de *Polling* al uso de interrupciones externas (EXTI) que despiertan tareas específicas mediante Semáforos Binarios, logrando un mejor tiempo de respuesta.
* **Sincronización:** Uso de *Mutex* para proteger el acceso a recursos críticos compartidos (como LEDs) y Semáforos para coordinar secuencias de tareas.
* **Eliminación y Hook Idle:** Comprobación de la eliminación exitosa de tareas de la memoria utilizando la Tarea Demonio (Idle) del sistema.

### 📙 [Guía 2: Comunicación por Queue y Software Timers](reports/INFORME_GUIA2.md)
En el segundo reporte, el enfoque se centra en la comunicación segura de datos entre tareas y la gestión optimizada de eventos temporales. Los principales temas incluyen:
* **Colas (Queues):** Gestión de flujos de datos asíncronos entre tareas productoras y tareas consumidoras, evaluando el impacto de distintas velocidades de procesamiento.
* **Optimización de Memoria (RAM):** Análisis del ahorro de espacio y ciclos de CPU al transferir punteros (paso por referencia) a través de las colas, frente al envío de estructuras pesadas completas (paso por valor).
* **Queue Sets y Mailboxes:** Implementación de `Queue Sets` para escuchar de manera bloqueante múltiples fuentes de datos sin gastar CPU, y creación de un esquema de buzón (Mailbox) empleando `xQueuePeek` para que múltiples tareas lean eventos simultáneamente.
* **Software Timers:** Creación de temporizadores periódicos (*Auto-reload*) y de un solo disparo (*One-shot*), implementando funciones avanzadas como un *Watchdog* por software. Además, se detalla por qué usar Timers reduce el consumo de pila (Stack) al delegarse a la `Daemon Task` de FreeRTOS.
* **Máquinas de Estados y Notificaciones:** Construcción de una aplicación estructurada mediante *Task Notifications* con variables de 32 bits, logrando una máquina de estados segura e ininterrumpible.
