cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
include(FreeRTOS_Kernel_import.cmake)

project(tank_level_monitoring_control_proj)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

add_executable(main
        src/main.c
        ../mylib/meas/meas.c
        ../mylib/uart/uart.c
        ../mylib/led/led.c
        ../mylib/ctrl/ctrl.c
)

target_include_directories(main PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/../mylib/meas
        ${CMAKE_CURRENT_LIST_DIR}/../mylib/led
        ${CMAKE_CURRENT_LIST_DIR}/../mylib/uart
        ${CMAKE_CURRENT_LIST_DIR}/../mylib/ctrl
)

target_link_libraries(main pico_stdlib hardware_gpio hardware_adc FreeRTOS-Kernel FreeRTOS-Kernel-Heap4)

pico_add_extra_outputs(main)