 /** 
 **************************************************************
 * @file main.c
 * @author HBN - 45300747
 * @date 30062022
 * @brief Main file for water tank level monitoring & level
 *        control project. 
 *************************************************************** 
 */

#include "main.h"

/**
 * @brief main
 */
void main(void) {
    // Initialise all present stdio types that are linked into the binary
    // (as per ADC example from RP2040 SDK documentation at 
    // https://raspberrypi.github.io/pico-sdk-doxygen/group__hardware__adc.html#adc_example)
    stdio_init_all();

    // Initialise level measurement controlling tasks
    t1_meas_task_init();
    t2_meas_task_init();

    // Initialise control enable controlling task
    level_ctrl_enable_task_init();

    // Initialise LED controlling task
    led_task_init();

    // Initialise UART controlling task
    uart_task_init();

    // Start the RTOS scheduler
    vTaskStartScheduler();
}