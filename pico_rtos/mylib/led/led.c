 /** 
 **************************************************************
 * @file led.c
 * @author HBN - 45300747
 * @date 30062022
 * @brief LED driver file. This file handles functionality specific to 
 *        toggling the Raspberry Pi Pico onboard LED. 
 *************************************************************** 
 */

#include "led.h"

/**
 * @brief LED task. This task handles toggling the Raspberry Pi Pico onboard 
 *        green LED. 
 * @param param Value passed upon task creation. 
 * @retval None. 
 */
void led_task(void *param) {
    // Initialise the LED pin
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (1) {
        // Toggle the onboard LED every 100 ticks
        gpio_put(LED_PIN, 1);
        vTaskDelay(100);
        gpio_put(LED_PIN, 0);
        vTaskDelay(100);
    }
}

/**
 * @brief LED controlling task creation helper function. This function 
 *        creates the LED controlling task. 
 * @param None. 
 * @retval None. 
 */
void led_task_init(void) {
    xTaskCreate((void *)&led_task, 
        (const signed char *)"LED_Task", 256, NULL, 1, NULL);
}