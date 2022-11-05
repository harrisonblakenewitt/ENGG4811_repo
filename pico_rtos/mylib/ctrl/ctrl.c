 /** 
 **************************************************************
 * @file ctrl.c
 * @author HBN - 45300747
 * @date 04092022
 * @brief Water tank level control driver file. This file handles functionality
 *        specific to controlling water tank level for both tanks, and 
 *        enabling/disabling the water tank level control feature as determined
 *        by the switch connected to GPIO2. 
 *************************************************************** 
 */

#include "ctrl.h"

// Semaphore which is given when control enable switch is switched on/off, 
// which notifies the level control enable task that a logic state change 
// has occurred on the pin connected to the switch. 
SemaphoreHandle_t ctrl_enable_sem;

// Semaphores that are given when control is switched off, which notifies 
// tank level control tasks to delete themselves. 
SemaphoreHandle_t delete_t1_ctrl_sem;
SemaphoreHandle_t delete_t2_ctrl_sem;

// Semaphores which are given when a level controlling task must be notified
// that their respective water tank requires filling. 
SemaphoreHandle_t fill_t1_sem;
SemaphoreHandle_t fill_t2_sem;

// Semaphores which are given when a level controlling task must be notified
// that filling of their respective water tank must stop. 
SemaphoreHandle_t stop_fill_t1_sem;
SemaphoreHandle_t stop_fill_t2_sem;

// Semaphores which are given when a level controlling task must be notified
// that their respective water tank requires draining. 
SemaphoreHandle_t drain_t1_sem;
SemaphoreHandle_t drain_t2_sem;

// Semaphores which are given when a level controlling task must be notified
// that draining of their respective water tank must stop. 
SemaphoreHandle_t stop_drain_t1_sem;
SemaphoreHandle_t stop_drain_t2_sem;

// Semaphores which are given to notify the water tank level measurement
// controlling task for tank 1 whether or not control is enabled for 
// tank 1. 
SemaphoreHandle_t ctrl_on_sem_1;
SemaphoreHandle_t ctrl_off_sem_1;

// Semaphores which are given to notify the water tank level measurement
// controlling task for tank 2 whether or not control is enabled for 
// tank 2. 
SemaphoreHandle_t ctrl_on_sem_2;
SemaphoreHandle_t ctrl_off_sem_2;

/**
 * @brief GPIO2 interrupt callback. This callback is executed upon rising and 
 *        falling edges on GPIO2. 
 * @param gpio GPIO number. 
 * @param events Events that caused the interrupt to occur. 
 * @retval None. 
 */
void gpio2_cb(uint gpio, uint32_t events) {
    // This will be set to pdTRUE if giving the semaphore causes a task to
    // unblock which has a priority than the task which is currently running. 
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // If ctrl_enable_sem isn't NULL, give the semaphore to notify of an edge
    // change on GPIO2. 
    if (ctrl_enable_sem != NULL) {
        xSemaphoreGiveFromISR(ctrl_enable_sem, &xHigherPriorityTaskWoken);   
    }
}

/**
 * @brief Tank 1 valve controlling pins initialiser function. This function
 *        handles initialisation of GPIO pins which are used to open/close
 *        the tank 1 valves. 
 * @param None. 
 * @retval None. 
 */
void t1_valve_pins_init(void) {
    // Initialise tank 1 fill valve controlling pin. 
    gpio_init(GPIO14);
    gpio_set_dir(GPIO14, GPIO_OUT);
    gpio_pull_down(GPIO14);
    gpio_put(GPIO14, false);

    // Initialise tank 1 drain valve controlling pin. 
    gpio_init(GPIO15);
    gpio_set_dir(GPIO15, GPIO_OUT);
    gpio_pull_down(GPIO15);
    gpio_put(GPIO15, false);
}

/**
 * @brief Tank 2 valve controlling pins initialiser function. This function
 *        handles initialisation of GPIO pins which are used to open/close
 *        the tank 2 valves. 
 * @param None. 
 * @retval None. 
 */
void t2_valve_pins_init(void) {
    // Initialise tank 2 fill valve controlling pin. 
    gpio_init(GPIO16);
    gpio_set_dir(GPIO16, GPIO_OUT);
    gpio_pull_down(GPIO16);
    gpio_put(GPIO16, false);

    // Initialise tank 2 drain valve controlling pin. 
    gpio_init(GPIO17);
    gpio_set_dir(GPIO17, GPIO_OUT);
    gpio_pull_down(GPIO17);
    gpio_put(GPIO17, false);
}

/**
 * @brief Level control enable pin init. This function handles initialisation
 *        of the GPIO pin being used to determine whether to enable or disable 
 *        the water tank level control functionality. 
 * @param None. 
 * @retval None. 
 */
void level_ctrl_enable_pin_init(void) {
    // Initialise control enable pin. 
    gpio_init(GPIO2);
    gpio_set_dir(GPIO2, GPIO_IN);
    gpio_pull_down(GPIO2);

    // Set up interrupt with callback to trigger on rising and falling
    // edge on control enable pin. 
    gpio_set_irq_enabled_with_callback(GPIO2, (GPIO_IRQ_EDGE_FALL 
            | GPIO_IRQ_EDGE_RISE), true, &gpio2_cb);
}

/**
 * @brief Tank 1 control task semaphore initialiser function. This function
 *        handles initialisation of semaphores used by the tank 1 level 
 *        control task. 
 * @param None. 
 * @retval None. 
 */
void init_t1_semaphores(void) {
    delete_t1_ctrl_sem = xSemaphoreCreateBinary();
    fill_t1_sem = xSemaphoreCreateBinary();
    stop_fill_t1_sem = xSemaphoreCreateBinary();
    drain_t1_sem = xSemaphoreCreateBinary();
    stop_drain_t1_sem = xSemaphoreCreateBinary();
}

/**
 * @brief Tank 1 control pin handler. This function handles setting logic 
 *        levels of valve control pins based on whether the tank requires
 *        filling, draining, or neither. 
 * @param filling Boolean value denoting whether or not the tank needs to fill. 
 * @param draining Boolean value denoting whether or not the tank needs to drain. 
 * @param deinit Boolean value denoting whether or not deinitialisation of the 
 *        tank 1 level control task is occurring. 
 * @retval None. 
 */
void handle_t1_ctrl_pins(bool filling, bool draining, bool deinit) {

    /* If deinitialisation isn't occurring, set GPIO pins which control the
    valves to the appropriate states as requested. 
    
    Note that Boolean value corresponds to valve state (i.e., 'true' will 
    open a valve, 'false' will close a valve). */
    if (!deinit) {
        gpio_put(GPIO14, filling);
        gpio_put(GPIO15, draining);
    } else {
        // If deinitialisation is occurring, close both of the tank 1 level 
        // control valves. 
        gpio_put(GPIO14, false);
        gpio_put(GPIO15, false);
    }
}

/**
 * @brief Tank 1 control task deinitialise helper function. This function
 *        handles deinitialisation of semaphores used by the tank 1 control
 *        task and closing the valves used by this task. 
 * @param None. 
 * @retval None. 
 */
void deinit_t1_level_ctrl_task(void) {
    // Delete semaphores
    vSemaphoreDelete(delete_t1_ctrl_sem);
    vSemaphoreDelete(fill_t1_sem);
    vSemaphoreDelete(stop_fill_t1_sem);
    vSemaphoreDelete(drain_t1_sem);
    vSemaphoreDelete(stop_drain_t1_sem);

    // Close valves because tank 1 control task is being deinitialised/
    // deleted. 
    handle_t1_ctrl_pins(false, false, true);
}

/**
 * @brief Tank 1 control task. This task handles water level control for tank
 *        1 when level control is enabled. 
 * @param param Value passed upon task creation. 
 * @retval None. 
 */
void t1_level_ctrl_task(void *param) {
    // Initialise valve controlling pins and semaphores used by this task
    t1_valve_pins_init();
    init_t1_semaphores();

    // Local variables denoting tank fill and drain state
    bool filling = false, draining = false;

    while (1) {
        if (filling) {
            // If tank is filling and semaphore is given which notifies tank 
            // no longer needs to fill, close the fill valve. 
            if (stop_fill_t1_sem != NULL) {
                if (xSemaphoreTake(stop_fill_t1_sem, 20) == pdTRUE) {
                    filling = false;
                    handle_t1_ctrl_pins(filling, draining, false);
                }
            }
        } else {
            // If tank isn't filling and semaphore is given which notifies tank
            // needs to be filled, open the fill valve. 
            if (fill_t1_sem != NULL) {
                if (xSemaphoreTake(fill_t1_sem, 20) == pdTRUE) {
                    filling = true;
                    handle_t1_ctrl_pins(filling, draining, false);
                }
            }
        }

        if (draining) {
            // If tank is draining and semaphore is given which notifies tank
            // no longer needs drain, close the drain valve. 
            if (stop_drain_t1_sem != NULL) {
                if (xSemaphoreTake(stop_drain_t1_sem, 20) == pdTRUE) {
                    draining = false;
                    handle_t1_ctrl_pins(filling, draining, false);
                }
            }
        } else {
            if (drain_t1_sem != NULL) {
            // If tank isn't draining and semaphore is given which notifies 
            // tank needs to be drained, open the drain valve. 
                if (xSemaphoreTake(drain_t1_sem, 20) == pdTRUE) {
                    draining = true;
                    handle_t1_ctrl_pins(filling, draining, false);
                }
            }
        }

        // If semaphore is given notifying that task must be deleted (occurs
        // when control functionality is disabled), delete this task. 
        if (delete_t1_ctrl_sem != NULL) {
            if (xSemaphoreTake(delete_t1_ctrl_sem, 20) == pdTRUE) {
                // Delete semaphores and close valves. 
                deinit_t1_level_ctrl_task();
                vTaskDelete(NULL);
            }            
        }

        vTaskDelay(20);
    }
}

/**
 * @brief Tank 2 control task semaphore initialiser function. This function
 *        handles initialisation of semaphores used by the tank 2 level 
 *        control task. 
 * @param None. 
 * @retval None. 
 */
void init_t2_semaphores(void) {
    delete_t2_ctrl_sem = xSemaphoreCreateBinary();
    fill_t2_sem = xSemaphoreCreateBinary();
    stop_fill_t2_sem = xSemaphoreCreateBinary();
    drain_t2_sem = xSemaphoreCreateBinary();
    stop_drain_t2_sem = xSemaphoreCreateBinary();
}

/**
 * @brief Tank 2 control pin handler. This function handles setting logic 
 *        levels of valve control pins based on whether the tank requires
 *        filling, draining, or neither. 
 * @param filling Boolean value denoting whether or not the tank needs to fill. 
 * @param draining Boolean value denoting whether or not the tank needs to drain. 
 * @param deinit Boolean value denoting whether or not deinitialisation of the 
 *        tank 2 level control task is occurring. 
 * @retval None.
 */
void handle_t2_ctrl_pins(bool filling, bool draining, bool deinit) {

    /* If deinitialisation isn't occurring, set GPIO pins which control the
    valves to the appropriate states as requested. 
    
    Note that Boolean value corresponds to valve state (i.e., 'true' will 
    open a valve, 'false' will close a valve). */
    if (!deinit) {
        gpio_put(GPIO16, filling);
        gpio_put(GPIO17, draining);
    } else {
        // If deinitialisation is occurring, close both of the tank 2 level 
        // control valves. 
        gpio_put(GPIO16, false);
        gpio_put(GPIO17, false);
    }
}

/**
 * @brief Tank 2 control task deinitialise helper function. This function
 *        handles deinitialisation of semaphores used by the tank 2 control
 *        task and closing the valves used by this task. 
 * @param None. 
 * @retval None. 
 */
void deinit_t2_level_ctrl_task(void) {
    // Delete semaphores
    vSemaphoreDelete(delete_t2_ctrl_sem);
    vSemaphoreDelete(fill_t2_sem);
    vSemaphoreDelete(stop_fill_t2_sem);
    vSemaphoreDelete(drain_t2_sem);
    vSemaphoreDelete(stop_drain_t2_sem);

    // Close valves because tank 1 control task is being deinitialised/
    // deleted. 
    handle_t2_ctrl_pins(false, false, true);
}

/**
 * @brief Tank 2 control task. This task handles water level control for tank
 *        2 when level control is enabled. 
 * @param param Value passed upon task creation. 
 * @retval None. 
 */
void t2_level_ctrl_task(void *param) {
    
    // Initialise valve controlling pins and semaphores used by this task
    t2_valve_pins_init();
    init_t2_semaphores();

    // Local variables denoting tank fill and drain state
    bool filling = false, draining = false;

    while (1) {
        if (filling) {
            // If tank is filling and semaphore is given which notifies tank 
            // no longer needs to fill, close the fill valve. 
            if (stop_fill_t2_sem != NULL) {
                if (xSemaphoreTake(stop_fill_t2_sem, 20) == pdTRUE) {
                    filling = false;
                    handle_t2_ctrl_pins(filling, draining, false);
                }
            }
        } else {
            // If tank isn't filling and semaphore is given which notifies tank
            // needs to be filled, open the fill valve. 
            if (fill_t2_sem != NULL) {
                if (xSemaphoreTake(fill_t2_sem, 20) == pdTRUE) {
                    filling = true;
                    handle_t2_ctrl_pins(filling, draining, false);
                }
            }
        }

        if (draining) {
            // If tank is draining and semaphore is given which notifies tank
            // no longer needs drain, close the drain valve. 
            if (stop_drain_t2_sem != NULL) {
                if (xSemaphoreTake(stop_drain_t2_sem, 20) == pdTRUE) {
                    draining = false;
                    handle_t2_ctrl_pins(filling, draining, false);
                }
            }
        } else {
            // If tank isn't draining and semaphore is given which notifies 
            // tank needs to be drained, open the drain valve. 
            if (drain_t2_sem != NULL) {
                if (xSemaphoreTake(drain_t2_sem, 20) == pdTRUE) {
                    draining = true;
                    handle_t2_ctrl_pins(filling, draining, false);
                }
            }
        }

        // If semaphore is given notifying that task must be deleted (occurs
        // when control functionality is disabled), delete this task. 
        if (delete_t2_ctrl_sem != NULL) {
            if (xSemaphoreTake(delete_t2_ctrl_sem, 20) == pdTRUE) {
                // Delete semaphores and close valves. 
                deinit_t2_level_ctrl_task();
                vTaskDelete(NULL);
            }            
        }

        vTaskDelay(20);
    }
}

/**
 * @brief Level control enable controlling task. This task handles creation
 *        and deletion of the tank level control tasks depending on if
 *        level control is enabled or disabled (as denoted by the switch
 *        connected to GPIO2). 
 * @param
 * @retval None. 
 */
void level_ctrl_enable_task(void *param) {

    // Initialise level control enable pin and interrupt callback
    level_ctrl_enable_pin_init();

    // Create semaphores used by this task
    ctrl_enable_sem = xSemaphoreCreateBinary();
    ctrl_on_sem_1 = xSemaphoreCreateBinary();
    ctrl_on_sem_2 = xSemaphoreCreateBinary();
    ctrl_off_sem_1 = xSemaphoreCreateBinary();
    ctrl_off_sem_2 = xSemaphoreCreateBinary();

    while (1) {
        if (ctrl_enable_sem != NULL) {
            // The following code will execute when rising or falling edge
            // is detected on GPIO2, after semaphore is given by interrupt
            // callback. 
            if (xSemaphoreTake(ctrl_enable_sem, portMAX_DELAY) == pdTRUE) {
                // If GPIO2 is low after an edge change, control functionality
                // is disabled. 
                if (!gpio_get(GPIO2)) {
                    // Give semaphore to delete tank 1 control task
                    if (delete_t1_ctrl_sem != NULL) {
                        xSemaphoreGive(delete_t1_ctrl_sem);
                    }

                    // Give semaphore to delete tank 1 control task
                    if (delete_t2_ctrl_sem != NULL) {
                        xSemaphoreGive(delete_t2_ctrl_sem);
                    }

                    // Give semaphores to notify level measurement tasks 
                    // that level control is disabled. 
                    xSemaphoreGive(ctrl_off_sem_1);
                    xSemaphoreGive(ctrl_off_sem_2);

                } else {
                    // If GPIO2 isn't low after an edge change, control 
                    // functionality is enabled.

                    // Initialise tank level control tasks
                    t1_level_ctrl_task_init();
                    t2_level_ctrl_task_init();

                    // Give semaphores to notify level measurement tasks
                    // that level control is enabled. 
                    xSemaphoreGive(ctrl_on_sem_1);
                    xSemaphoreGive(ctrl_on_sem_2);
                }
            }
        }

        vTaskDelay(1000);
    }
}

/**
 * @brief Tank 1 level control task creation helper function. This function
 *        creates the tank 1 level control task. 
 * @param None. 
 * @retval pdPASS if the task was successfully created, 
 *         errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY if it wasn't successfully
 *         created. 
 */
BaseType_t t1_level_ctrl_task_init(void) {
    return (xTaskCreate((void *)&t1_level_ctrl_task, 
        (const signed char *)"Tank_1_Level_Control_Task", 256, NULL, 1, NULL));
}

/**
 * @brief Tank 2 level control task creation helper function. This function
 *        creates the tank 2 level control task. 
 * @param None. 
 * @retval pdPASS if the task was successfully created, or
 *         errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY if it wasn't successfully
 *         created. 
 */
BaseType_t t2_level_ctrl_task_init(void) {
    return (xTaskCreate((void *)&t2_level_ctrl_task, 
        (const signed char *)"Tank_2_Level_Control_Task", 256, NULL, 1, NULL));
}

/**
 * @brief Level control enable task creation helper function. This function
 *        creates the level control enable task. 
 * @param None. 
 * @retval None. 
 */
void level_ctrl_enable_task_init(void) {
    xTaskCreate((void *)&level_ctrl_enable_task, 
        (const signed char *)"Level_Control_Enable_Task", 256, NULL, 1, NULL);
}