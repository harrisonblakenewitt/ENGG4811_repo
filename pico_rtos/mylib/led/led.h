 /** 
 **************************************************************
 * @file led.h
 * @author HBN - 45300747
 * @date 30062022
 * @brief Header file for the LED driver. 
 *************************************************************** 
 */

#ifndef LED_H
#define LED_H

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Function prototypes
void led_task(void *param);
void led_task_init(void);

#endif