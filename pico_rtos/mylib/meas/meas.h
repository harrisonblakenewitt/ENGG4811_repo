 /** 
 **************************************************************
 * @file meas.h
 * @author HBN - 45300747
 * @date 30062022
 * @brief Header file for the water tank level measurement driver. 
 *************************************************************** 
 */

#ifndef MEAS_H
#define MEAS_H

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "uart.h"
#include "ctrl.h"

#define VREF 3.0            // ADC reference voltage
#define RES_LEVELS 4095     // ADC resolution levels (12-bit)

// GPIO pin number declarations
#define GPIO26 26
#define GPIO27 27
#define GPIO28 28

// ADC channel number declarations
#define CHANNEL_0 0
#define CHANNEL_1 1
#define CHANNEL_2 2

// Time between pressure sensor samples for each tank (in sec)
#define T1_SAMPLE_PERIOD 1
#define T2_SAMPLE_PERIOD 1

// Tank number declarations
#define TANK_1 1
#define TANK_2 2

// Critical water tank levels (i.e., levels which cause tank filling/draining
// to be initiated). 
#define TANK_1_MAX_FILL_LEVEL 60.0
#define TANK_2_MAX_FILL_LEVEL 60.0
#define TANK_1_MIN_FILL_LEVEL 10.0
#define TANK_2_MIN_FILL_LEVEL 10.0

// Safe water levels for fill and drain (i.e., when filling, tank will fill
// to the fill to level, when draining, tank will drain to the drain 
// to level). 
#define TANK_1_FILL_TO_LEVEL 20.0
#define TANK_2_FILL_TO_LEVEL 20.0
#define TANK_1_DRAIN_TO_LEVEL 50.0
#define TANK_2_DRAIN_TO_LEVEL 50.0

// Water height offsets for the usable water level range. Everything
// below these measurements (which are in cm) will be considered as 
// "empty". 
#define TANK_1_USABLE_HEIGHT_OFFSET 4.0
#define TANK_2_USABLE_HEIGHT_OFFSET 2.0

// Pressure sensor zero offsets (determined via experimentation). 
#define TANK_1_ZERO_PRESSURE_OFFSET 140.183
#define TANK_2_ZERO_PRESSURE_OFFSET 221.583

// Width of averaging window being used to smooth pressure readings. 
#define AVG_WINDOW_WIDTH 20
#define AVG_WINDOW_WIDTH_FLOAT 20.0

// Function prototypes 
void init_t1_adc_pins(void);
void init_t2_adc_pins(void);
float calc_pressure(uint16_t pressure_channel_raw, uint16_t offset_channel_raw);
void check_ctrl_requirements(bool *filling, bool *draining, float height, uint8_t tank);
void t1_meas_task(void *param);
void t2_meas_task(void *param);
void t1_meas_task_init(void);
void t2_meas_task_init(void);

#endif