/*
 * freeRTOS_AVR.c
 *
 * Created: 8/14/2017 11:09:51 AM
 * Author : JThiessen
 */

/******************************************************************************
 * Header file inclusions.
 ******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"

#include <avr/io.h>
#include <avr/sleep.h>

/******************************************************************************
 * Private macro definitions.
 ******************************************************************************/

#define mainLED_TASK_PRIORITY   1
#define mainLUART_TASK_PRIORITY   (tskIDLE_PRIORITY)

#define BAUD 9600UL      // Baudrate

// Berechnungen
#define UBRR_VAL ((F_CPU+BAUD*8)/(BAUD*16)-1)   // clever runden
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))     // Reale Baudrate
#define BAUD_ERROR ((BAUD_REAL*1000)/BAUD) // Fehler in Promille, 1000 = kein Fehler.

#if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
#error Systematischer Fehler der Baudrate grösser 1% und damit zu hoch!
#endif

/******************************************************************************
 * Private function prototypes.
 ******************************************************************************/

static void vBlinkLed(void* pvParameters);
static void vUartTask(void* pvParameters);

static void uart_init(void);
static void uart_putc(char c);
static void uart_puts (char *s);

/******************************************************************************
 * Public function definitions.
 ******************************************************************************/



/**************************************************************************//**
 * \fn int main(void)
 *
 * \brief Main function.
 *
 * \return
 ******************************************************************************/
int main_example(void)
{
	uart_init();
	uart_puts("--- FREE RTOS EXAMPLE ---\n\r");
    // Create task.
    xTaskHandle blink_handle;
	xTaskHandle uart_handle;
    xTaskCreate(vBlinkLed, "blink", configMINIMAL_STACK_SIZE, NULL, mainLED_TASK_PRIORITY, &blink_handle);
	xTaskCreate(vUartTask, "uart", configMINIMAL_STACK_SIZE, NULL, mainLUART_TASK_PRIORITY, &uart_handle);

    // Start scheduler.
    vTaskStartScheduler();

    return 0;
}

/**************************************************************************//**
 * \fn static vApplicationIdleHook(void)
 *
 * \brief
 ******************************************************************************/
void vApplicationIdleHook(void)
{
	uart_puts("--> TASK IDLE...\n\r");
}

/******************************************************************************
 * Private function definitions.
 ******************************************************************************/

/**************************************************************************//**
 * \fn static void vBlinkLed(void* pvParameters)
 *
 * \brief
 *
 * \param[in]   pvParameters
 ******************************************************************************/
static void vBlinkLed(void* pvParameters)
{
    //DDRB |= _BV(PINB5);

	uart_puts("--> TASK vBlinkLed: init\n\r");

    for ( ;; )
    {
		uart_puts("--> TASK vBlinkLed: BLINK...\n\r");
        //PORTB ^= _BV(PINB5);
        vTaskDelay(1000);
    }
}

/**************************************************************************//**
 * \fn static void vUartTask(void* pvParameters)
 *
 * \brief
 *
 * \param[in]   pvParameters
 ******************************************************************************/
static void vUartTask(void* pvParameters)
{
	uart_puts("--> TASK vBlinkLed: init\n\r");
	uint32_t i = 0;

    for ( ;; )
    {
		uart_puts("--> TASK vUartTask: ");
		uart_putc(i++);
		uart_puts("\n\r");
        vTaskDelay(500);
    }
}

