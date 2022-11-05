 /** 
 **************************************************************
 * @file meas.c
 * @author HBN - 45300747
 * @date 30062022
 * @brief Water tank level measurement driver file. This file handles
 *        functionality specific to measuring outputs from the pressure
 *        sensors (on ADC) and calculating water tank level from the 
 *        pressure readings based on the developed calibration equation. 
 *        Functionality specific to checking water tank level control
 *        requirements based on current tank filling/draining status and 
 *        water tank level (which only executes of control is enabled), 
 *        is also included in this driver. 
 *************************************************************** 
 */

#include "meas.h"

/**
 * @brief Tank 1 ADC initialiser function. This function handles ADC 
 *        initialisation and initialisation of the ADC pins used to measure
 *        the water pressure at the bottom of water tank 1. 
 * @param None. 
 * @retval None. 
 */
void init_t1_adc_pins(void) {
    adc_init();
    adc_gpio_init(GPIO26);
    adc_gpio_init(GPIO28);
}

/**
 * @brief Tank 2 ADC initialiser function. This function handles ADC 
 *        initialisation and initialisation of the ADC pins used to measure
 *        the water pressure at the bottom of water tank 2. 
 * @param None. 
 * @retval None. 
 */
void init_t2_adc_pins(void) {
    adc_init();
    adc_gpio_init(GPIO27);
    adc_gpio_init(GPIO28);
}

/**
 * @brief Pressure calculation function. This function calculates pressure
 *        based on the given raw ADC readings. 
 * @param pressure_channel_raw raw ADC reading for the pressure channel. 
 * @param offset_channel_raw raw ADC reading for the offset channel (connected 
 *        directly to GND). 
 * @retval Instantaneous measured pressure. 
 */
float calc_pressure(uint16_t pressure_channel_raw, uint16_t offset_channel_raw) {

    // float corrected_pressure_channel = ((float)(pressure_channel_raw 
    //         - offset_channel_raw)) * (VREF / RES_LEVELS);

    // Determine the voltage at the ADC pin
    float corrected_pressure_channel = (float)pressure_channel_raw * (VREF / RES_LEVELS);

    // Determine the voltage at the pressure sensor output (based on the 
    // resistances used in the voltage divider which steps 5V down to 3V). 
    float sensor_voltage = (500.0 * corrected_pressure_channel) / 280.0;

    // Calculate pressure with respect to the voltage at the output of the 
    // sensor. The sensor has a linear output ranging from 0.2V - 4.7V
    // which corresponds to the pressure range of 0Pa - 10kPa. 
    float inst_pressure = (((20000.0 / 9.0) * sensor_voltage) - (4000.0 / 9.0));

    return inst_pressure;
}

/**
 * @brief Control requirements checker function. This function checks
 *        water tank level readings for the given tank and compares
 *        the reading against the necessary fill and drain thresholds
 *        depending on the water tank filling and draining status. When
 *        valve state change for fill and drain valves is required, semaphore
 *        is given to notify the appropriate level control task. 
 * @param filling Pointer to filling status (passed by reference from
 *        measurement controlling task).
 * @param draining Pointer to draining status (passed by reference from
 *        measurement controlling task).
 * @param height Height of water level within tank. 
 * @param tank Tank which control requirements are being checked for. 
 * @retval None.
 */
void check_ctrl_requirements(bool *filling, bool *draining, float height, uint8_t tank) {
    if ((*filling)) {
        // If tank for which control requirements are being checked is tank
        // 1, and the tank is filling
        if (tank == TANK_1) {
            // If the level of water in the tank is equal to or above the
            // to level, give semaphore to signal level control task to 
            // stop filling.
            if (height >= TANK_1_FILL_TO_LEVEL) {
                if (stop_fill_t1_sem != NULL) {
                    xSemaphoreGive(stop_fill_t1_sem);
                }
                // Set filling local variable in controlling task to false. 
                (*filling) = false;
            }
        
        // If tank for which control requirements are being checked is tank
        // 2, and the tank is filling
        } else if (tank == TANK_2) {
            // If the level of water in the tank is equal to or above the
            // to level, give semaphore to signal level control task to 
            // stop filling.
            if (height >= TANK_2_FILL_TO_LEVEL) {
                if (stop_fill_t2_sem != NULL) {
                    xSemaphoreGive(stop_fill_t2_sem);
                }
                // Set filling local variable in controlling task to false. 
                (*filling) = false;
            }
        }
    } else {
        // If tank for which control requirements are being checked is tank
        // 1, and the tank is not filling
        if (tank == TANK_1) {
            // If the level of water in the tank is less than or equal to the
            // minimum fill level, give semaphore to signal level control task
            // to start filling.
            if (height <= TANK_1_MIN_FILL_LEVEL) {
                if (fill_t1_sem != NULL) {
                    xSemaphoreGive(fill_t1_sem);
                }
                // Set filling local variable in controlling task to true. 
                (*filling) = true;
            }

        // If tank for which control requirements are being checked is tank
        // 2, and the tank is not filling
        } else if (tank == TANK_2) {
            // If the level of water in the tank is less than or equal to the
            // minimum fill level, give semaphore to signal level control task
            // to start filling.
            if (height <= TANK_2_MIN_FILL_LEVEL) {
                if (fill_t2_sem != NULL) {
                    xSemaphoreGive(fill_t2_sem);
                }
                // Set filling local variable in controlling task to true. 
                (*filling) = true;
            }
        }
    }

    if ((*draining)) {
        // If tank for which control requirements are being checked is tank
        // 1, and the tank is draining
        if (tank == TANK_1) {
            // If the level of water in the tank is less than or equal to the
            // drain to level, give semaphore to signal level control task to 
            // stop draining.
            if (height <= TANK_1_DRAIN_TO_LEVEL) {
                if (stop_drain_t1_sem != NULL) {
                    xSemaphoreGive(stop_drain_t1_sem);
                }
                // Set draining local variable in controlling task to false. 
                (*draining) = false;
            }

        // If tank for which control requirements are being checked is tank
        // 2, and the tank is draining
        } else if (tank == TANK_2) {
            // If the level of water in the tank is less than or equal to the
            // drain to level, give semaphore to signal level control task to 
            // stop draining.
            if (height <= TANK_2_DRAIN_TO_LEVEL) {
                if (stop_drain_t2_sem != NULL) {
                    xSemaphoreGive(stop_drain_t2_sem);
                }
                // Set draining local variable in controlling task to false. 
                (*draining) = false;
            }
        }
    } else {
        // If tank for which control requirements are being checked is tank
        // 1, and the tank is not draining
        if (tank == TANK_1) {
            // If the level of water in the tank is equal to or above the
            // maximum fill level, give semaphore to signal level control task
            // to start draining.
            if (height >= TANK_1_MAX_FILL_LEVEL) {
                if (drain_t1_sem != NULL) {
                    xSemaphoreGive(drain_t1_sem);
                }
                // Set draining local variable in controlling task to true. 
                (*draining) = true;
            }

        // If tank for which control requirements are being checked is tank
        // 2, and the tank is not draining
        } else if (tank == TANK_2) {
            // If the level of water in the tank is equal to or above the
            // maximum fill level, give semaphore to signal level control task
            // to start draining.
            if (height >= TANK_2_MAX_FILL_LEVEL) {
                if (drain_t2_sem != NULL) {
                    xSemaphoreGive(drain_t2_sem);
                }
                // Set draining local variable in controlling task to true. 
                (*draining) = true;
            }
        }
    }
}

/**
 * @brief Tank 1 water level measurement task. This task handles water level 
 *        measurement for tank 1. 
 * @param param Value passed upon task creation. 
 * @retval None. 
 */
void t1_meas_task(void *param) {
    // Initialise ADC and appropriate pins for tank 1 level measurement
    init_t1_adc_pins();

    // Local filling, draining, and level control state variables
    bool filling = false, draining = false, ctrl_on = false;

    // Timestamp of last height calculation
    TickType_t last_calculation = 0;

    // Averaging window for smoothing pressure measurements, and
    // current index within window (for adding new data). 
    float avg_window[AVG_WINDOW_WIDTH] = {0.0}; 
    uint8_t avg_window_index = 0;
 
    while (1) {
        // Calculate current system runtime (in seconds)
        TickType_t current_runtime = xTaskGetTickCount() / configTICK_RATE_HZ;

        // If the period between samples has exceeded the specified interval,
        // or the tick count has overflowed, take measurement. 
        if (((current_runtime - last_calculation) > T1_SAMPLE_PERIOD) 
                || (current_runtime < last_calculation)) {
            // Measure ADC input at channel connected to pressure sensor
            // being used to monitor water tank 1 level. 
            adc_select_input(CHANNEL_0);
            uint16_t pressure_channel_1_raw = adc_read();

            // Measure ADC input at offset channel (connected to ground, 
            // as suggested by Raspberry Pi Pico datasheet, only really
            // necessary when shunt reference isn't being used). 
            adc_select_input(CHANNEL_2);
            uint16_t offset_channel_raw = adc_read();

            // Calculate instantaneous pressure as per the current ADC readings. 
            float inst_pressure = calc_pressure(pressure_channel_1_raw, offset_channel_raw);

            // Add instantaneous pressure to current index in averaging window,
            // and increment average window index. 
            avg_window[avg_window_index] = inst_pressure;
            avg_window_index++;

            // If average window index exceeds window length, reset index
            if (avg_window_index >= AVG_WINDOW_WIDTH) {
                avg_window_index = 0;
            }

            // Calculate average of samples in averaging window
            float avg_pressure = 0.0;
            for (uint8_t i = 0; i < AVG_WINDOW_WIDTH; i++) {
                avg_pressure += avg_window[i];
            }
            avg_pressure = avg_pressure / AVG_WINDOW_WIDTH_FLOAT;

            // Calculate height using averaging window, using the equation
            // derived via manual calibration. 
            float height = (0.0124 * (avg_pressure - TANK_1_ZERO_PRESSURE_OFFSET)) + 1.656;

            // If height is lower than the minimum usable water height, consider 
            // tank to be empty.
            if (height < TANK_1_USABLE_HEIGHT_OFFSET) {
                height = 0.0;
            }

            // Put height reading packet in queue if requested to by UART
            // controlling task. 
            if (request_tank_1_height_sem != NULL) {
                if (xSemaphoreTake(request_tank_1_height_sem, 10) == pdTRUE) {
                    if (readings_queue_1 != NULL) {
                        struct packet readings_packet = {0};
                        readings_packet.tank = TANK_1;
                        readings_packet.height = height;

                        xQueueSendToFront(readings_queue_1, (void *) &readings_packet, 
                                portMAX_DELAY);
                    }
                }
            }

            // If ctrl_on_sem_1 is taken, control functionality has been
            // enabled, so update local variable. 
            if (ctrl_on_sem_1 != NULL) {
                if (xSemaphoreTake(ctrl_on_sem_1, 10) == pdTRUE) {
                    ctrl_on = true;
                }
            }

            // If ctrl_off_sem_1 is taken, control functionality has been
            // disabled, so update local variable. 
            if (ctrl_off_sem_1 != NULL) {
                if (xSemaphoreTake(ctrl_off_sem_1, 10) == pdTRUE) {
                    ctrl_on = false;
                }
            }

            // Check control requirements if control is on. 
            if (ctrl_on) {
                check_ctrl_requirements(&filling, &draining, height, TANK_1);

            } else {
                // If control is off, neither filling or draining can occur. 
                filling = false;
                draining = false;
            }

            // Update last calculation timestamp
            last_calculation = current_runtime;
        }

        vTaskDelay(100);
    }
}

/**
 * @brief Tank 2 water level measurement task. This task handles water level 
 *        measurement for tank 2. 
 * @param param Value passed upon task creation. 
 * @retval None. 
 */
void t2_meas_task(void *param) {
    // Initialise ADC and appropriate pins for tank 2 level measurement
    init_t2_adc_pins();

    // Local filling, draining, and level control state variables
    bool filling = false, draining = false, ctrl_on = false;

    // Timestamp of last height calculation
    int last_calculation = 0;

    // Averaging window for smoothing pressure measurements, and
    // current index within window (for adding new data). 
    float avg_window[AVG_WINDOW_WIDTH] = {0.0};
    uint8_t avg_window_index = 0;
 
    while (1) {
        // Calculate current system runtime (in seconds)
        int current_runtime = xTaskGetTickCount() / configTICK_RATE_HZ;

        // If the period between samples has exceeded the specified interval,
        // or the tick count has overflowed, take measurement. 
        if (((current_runtime - last_calculation) > T2_SAMPLE_PERIOD) 
                || (current_runtime < last_calculation)) {
            // Measure ADC input at channel connected to pressure sensor
            // being used to monitor water tank 2 level. 
            adc_select_input(CHANNEL_1);
            uint16_t pressure_channel_2_raw = adc_read();

            // Measure ADC input at offset channel (connected to ground, 
            // as suggested by Raspberry Pi Pico datasheet, only really
            // necessary when shunt reference isn't being used). 
            adc_select_input(CHANNEL_2);
            uint16_t offset_channel_raw = adc_read();

            // Calculate instantaneous pressure as per the current ADC readings. 
            float inst_pressure = calc_pressure(pressure_channel_2_raw, offset_channel_raw);

            // Add instantaneous pressure to current index in averaging window,
            // and increment average window index. 
            avg_window[avg_window_index] = inst_pressure;
            avg_window_index++;

            // If average window index exceeds window length, reset index
            if (avg_window_index >= AVG_WINDOW_WIDTH) {
                avg_window_index = 0;
            }

            // Calculate average of samples in averaging window
            float avg_pressure = 0.0;
            for (uint8_t i = 0; i < AVG_WINDOW_WIDTH; i++) {
                avg_pressure += avg_window[i];
            }
            avg_pressure = avg_pressure / AVG_WINDOW_WIDTH_FLOAT;

            // Calculate height using averaging window, using the equation
            // derived via manual calibration. 
            float height = (0.0124 * (avg_pressure - TANK_2_ZERO_PRESSURE_OFFSET)) + 1.656;

            // If height is lower than the minimum usable water height, consider 
            // tank to be empty.
            if (height < TANK_2_USABLE_HEIGHT_OFFSET) {
                height = 0.0;
            }

            // Put height reading packet in queue if requested to by UART
            // controlling task. 
            if (request_tank_2_height_sem != NULL) {
                if (xSemaphoreTake(request_tank_2_height_sem, 10) == pdTRUE) {
                    if (readings_queue_2 != NULL) {
                        struct packet readings_packet = {0};
                        readings_packet.tank = TANK_2;
                        readings_packet.height = height;

                        xQueueSendToFront(readings_queue_2, (void *) &readings_packet, 
                                portMAX_DELAY);
                    }
                }
            }

            // If ctrl_on_sem_2 is taken, control functionality has been
            // enabled, so update local variable. 
            if (ctrl_on_sem_2 != NULL) {
                if (xSemaphoreTake(ctrl_on_sem_2, 10) == pdTRUE) {
                    ctrl_on = true;
                }
            }

            // If ctrl_off_sem_2 is taken, control functionality has been
            // disabled, so update local variable. 
            if (ctrl_off_sem_2 != NULL) {
                if (xSemaphoreTake(ctrl_off_sem_2, 10) == pdTRUE) {
                    ctrl_on = false;
                }
            }

            // Check control requirements if control is on. 
            if (ctrl_on) {
                check_ctrl_requirements(&filling, &draining, height, TANK_2);

            } else {
                // If control is off, neither filling or draining can occur.
                filling = false;
                draining = false;
            }

            // Update last calculation timestamp
            last_calculation = current_runtime;
        }

        vTaskDelay(100);
    }
}

/**
 * @brief Tank 1 level measurement controlling task creation helper function.
 *        This function creates the tank 1 level measurement controlling task. 
 * @param None. 
 * @retval None. 
 */
void t1_meas_task_init(void) {
    xTaskCreate((void *)&t1_meas_task, (const signed char *)"Tank_1_Measurement_Task", 
        256, NULL, 1, NULL);
}

/**
 * @brief Tank 2 level measurement controlling task creation helper function.
 *        This function creates the tank 2 level measurement controlling task. 
 * @param None. 
 * @retval None. 
 */
void t2_meas_task_init(void) {
    xTaskCreate((void *)&t2_meas_task, (const signed char *)"Tank_2_Measurement_Task", 
        256, NULL, 1, NULL);
}