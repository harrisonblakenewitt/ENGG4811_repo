 /** 
 **************************************************************
 * @file ctrl.h
 * @author HBN - 45300747
 * @date 04092022
 * @brief Header file for the water tank level control driver. 
 *************************************************************** 
 */

#ifndef CTRL_H
#define CTRL_H

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// GPIO pin number declarations
#define GPIO2 2
#define GPIO14 14
#define GPIO15 15
#define GPIO16 16
#define GPIO17 17

// Semaphores which are given when a level controlling task must be notified
// that their respective water tank requires filling. 
extern SemaphoreHandle_t fill_t1_sem;
extern SemaphoreHandle_t fill_t2_sem;

// Semaphores which are given when a level controlling task must be notified
// that filling of their respective water tank must stop. 
extern SemaphoreHandle_t stop_fill_t1_sem;
extern SemaphoreHandle_t stop_fill_t2_sem;

// Semaphores which are given when a level controlling task must be notified
// that their respective water tank requires draining. 
extern SemaphoreHandle_t drain_t1_sem;
extern SemaphoreHandle_t drain_t2_sem;

// Semaphores which are given when a level controlling task must be notified
// that draining of their respective water tank must stop. 
extern SemaphoreHandle_t stop_drain_t1_sem;
extern SemaphoreHandle_t stop_drain_t2_sem;

// Semaphores which are given to notify the water tank level measurement
// controlling task for tank 1 whether or not control is enabled for 
// tank 1. 
extern SemaphoreHandle_t ctrl_on_sem_1;
extern SemaphoreHandle_t ctrl_off_sem_1;

// Semaphores which are given to notify the water tank level measurement
// controlling task for tank 2 whether or not control is enabled for 
// tank 2. 
extern SemaphoreHandle_t ctrl_on_sem_2;
extern SemaphoreHandle_t ctrl_off_sem_2;

// Function prototypes
void gpio2_cb(uint gpio, uint32_t events);
void t1_valve_pins_init(void);
void t2_valve_pins_init(void);
void level_ctrl_enable_pin_init(void);
void init_t1_semaphores(void);
void handle_t1_ctrl_pins(bool filling, bool draining, bool deinit);
void deinit_t1_level_ctrl_task(void);
void t1_level_ctrl_task(void *param);
void init_t2_semaphores(void);
void handle_t2_ctrl_pins(bool filling, bool draining, bool deinit);
void deinit_t2_level_ctrl_task(void);
void t2_level_ctrl_task(void *param);
void level_ctrl_enable_task(void *param);
BaseType_t t1_level_ctrl_task_init(void);
BaseType_t t2_level_ctrl_task_init(void);
void level_ctrl_enable_task_init(void);

#endif