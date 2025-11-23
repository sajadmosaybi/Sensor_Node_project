#ifndef SENSOR_H
#define SENSOR_H

#include "stm32l0xx_hal.h"

// Generic sensor data
typedef struct {
    float temperature_c;
    float humidity_rh;
    int error; // 0 = OK, non-zero = error code
} SensorData_t;

// Enum for hardware interface
typedef enum {
    SENSOR_IF_I2C,
    SENSOR_IF_SPI,
    SENSOR_IF_UART
} SensorInterface_t;

// Abstract sensor driver API
typedef struct {
    SensorInterface_t interface; // I2C / SPI / UART

    // Initialization function
    HAL_StatusTypeDef (*Init)(void *handle);

    // Trigger a measurement (if needed)
    HAL_StatusTypeDef (*Trigger)(void *handle);

    // Read sensor data
    SensorData_t (*Read)(void *handle);
} SensorDriver_t;

// Global pointer to active sensor
extern SensorDriver_t Sensor;

#endif
