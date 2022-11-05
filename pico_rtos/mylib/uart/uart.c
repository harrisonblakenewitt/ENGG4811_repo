 /** 
 **************************************************************
 * @file uart.c
 * @author HBN - 45300747
 * @date 30062022
 * @brief UART driver file. This file handles functionality specific to 
 *        fetching water tank height readings from the measurement
 *        controlling tasks upon request (requested via UART) by the 
 *        M5StickC Plus, and transmitting the level readings back to the 
 *        M5StickC Plus. 
 *************************************************************** 
 */

#include "uart.h"

// Queues used for passing data between measurement tasks and the UART
// controlling task. 
QueueHandle_t readings_queue_1;
QueueHandle_t readings_queue_2;

// Semaphores used to request readings from the measurement controlling
// tasks. When semaphore is given by UART controlling task, the measurement 
// controlling task puts data in its respective queue for the UART task. 
SemaphoreHandle_t request_tank_1_height_sem;
SemaphoreHandle_t request_tank_2_height_sem;


/**
 * @brief
 * @param param Value passed upon task creation. 
 * @retval None. 
 */
void uart_task(void *param) {
    // Initialise UART 0
    uart_init(uart0, 9600);
 
    // Set UART to be on GPIO0 and GPIO1 (TX and RX respectively)
    gpio_set_function(GPIO0, GPIO_FUNC_UART);
    gpio_set_function(GPIO1, GPIO_FUNC_UART);

    // Local tank height variables
    float tank_1_height = 0.0, tank_2_height = 0.0;

    // Create queues for passing data between measurement controlling tasks
    // and this task. 
    readings_queue_1 = xQueueCreate(10, sizeof(struct packet));
    readings_queue_2 = xQueueCreate(10, sizeof(struct packet));

    // Create semaphores used to request tank height data from measurement
    // controlling tasks. 
    request_tank_1_height_sem = xSemaphoreCreateBinary();
    request_tank_2_height_sem = xSemaphoreCreateBinary();

    while (1) {
        // Read data from UART (will be a request from the M5StickC Plus)
        uint8_t buffer = '\0';
        uart_read_blocking(uart0, (uint8_t *)&buffer, 1);

        // 'R' received on UART denotes a request for recent tank heights
        // from the M5StickC Plus. 
        if (buffer == 'R') {
            // Request new tank height reading from tank 1 measurement 
            // controlling task. 
            if (request_tank_1_height_sem != NULL) {
                xSemaphoreGive(request_tank_1_height_sem);
            }

            // Request new tank height reading from tank 2 measurement 
            // controlling task. 
            if (request_tank_2_height_sem != NULL) {
                xSemaphoreGive(request_tank_2_height_sem);
            }

            // Receive tank 1 level reading via queue
            if (readings_queue_1 != NULL) {
                struct packet reading_packet;
                if (xQueueReceive(readings_queue_1, &reading_packet, (10 * (T1_SAMPLE_PERIOD * SEC_TO_MILLI)))) {
                    tank_1_height = reading_packet.height;
                }
            }

            // Receive tank 2 level reading via queue
            if (readings_queue_2 != NULL) {
                struct packet reading_packet;
                if (xQueueReceive(readings_queue_2, &reading_packet, (10 * (T2_SAMPLE_PERIOD * SEC_TO_MILLI)))) {
                    tank_2_height = reading_packet.height;
                }
            }

            // Format string to send back to M5StickC Plus (agreed format 
            // between the two devices). 
            char uart_str[20] = {'\0'};
            sprintf(uart_str, "T1=%.1fT2=%.1f!", tank_1_height, tank_2_height);

            // Send formatted string to M5StickC Plus, and ensure 
            // transmission won't be interrupted. 
            vTaskSuspendAll();
            uart_puts(uart0, uart_str);
            xTaskResumeAll();
        }

        vTaskDelay(100);
    }
}

/**
 * @brief UART controlling task creation helper function. This function 
 *        creates the UART controlling task. 
 * @param None. 
 * @retval None. 
 */
void uart_task_init(void) {
    xTaskCreate((void *)&uart_task, (const signed char *)"UART_Task", 
            256, NULL, 1, NULL);
}