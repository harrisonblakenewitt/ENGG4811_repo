 /** 
 **************************************************************
 * @file uart.h
 * @author HBN - 45300747
 * @date 30062022
 * @brief Header file for the uart driver. 
 *************************************************************** 
 */

#ifndef UART_H
#define UART_H

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "meas.h"

// GPIO pin number declarations
#define GPIO0 0
#define GPIO1 1

// Number of milliseconds in one second. 
#define SEC_TO_MILLI 1000

// Queues used for passing data between measurement tasks and the UART
// controlling task. 
extern QueueHandle_t readings_queue_1;
extern QueueHandle_t readings_queue_2;

// Semaphores used to request readings from the measurement controlling
// tasks. When semaphore is given by UART controlling task, the measurement 
// controlling task puts data in its respective queue for the UART task. 
extern SemaphoreHandle_t request_tank_1_height_sem;
extern SemaphoreHandle_t request_tank_2_height_sem;

// Struct used for passing data between UART and measurement tasks via queue. 
struct packet {
    uint8_t tank;
    float height;
};

// Function prototypes
void uart_task(void *param);
void uart_task_init(void);

#endif