#include "sensor.h"

#define SHT31_ADDR (0x44 << 1)
static const uint8_t CMD_SINGLE[2] = {0x24, 0x00};

static uint8_t crc8(const uint8_t *data, int len) {
    uint8_t crc = 0xFF;
    for (int j = 0; j < len; j++) {
        crc ^= data[j];
        for (int i = 0; i < 8; i++)
            crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
    }
    return crc;
}

static HAL_StatusTypeDef SHT31_Init(void *handle) {
    I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)handle;
    uint8_t cmd[2] = {0x30, 0xA2}; // Soft reset
    return HAL_I2C_Master_Transmit(hi2c, SHT31_ADDR, cmd, 2, 100);
}

static HAL_StatusTypeDef SHT31_Trigger(void *handle) {
    I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)handle;
    return HAL_I2C_Master_Transmit(hi2c, SHT31_ADDR, (uint8_t*)CMD_SINGLE, 2, 100);
}

static SensorData_t SHT31_Read(void *handle) {
    SensorData_t r = {0};
    I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)handle;
    uint8_t buf[6];

    if (HAL_I2C_Master_Receive(hi2c, SHT31_ADDR, buf, 6, 100) != HAL_OK) {
        r.error = 1;
        return r;
    }
    if (crc8(buf, 2) != buf[2] || crc8(buf+3, 2) != buf[5]) {
        r.error = 2;
        return r;
    }

    uint16_t Traw = (buf[0]<<8)|buf[1];
    uint16_t Hraw = (buf[3]<<8)|buf[4];

    r.temperature_c = -45 + 175 * ((float)Traw / 65535.0f);
    r.humidity_rh = 100 * ((float)Hraw / 65535.0f);
    r.error = 0;
    return r;
}

// Export driver
SensorDriver_t SHT31_Driver = {
    .interface = SENSOR_IF_I2C,
    .Init = SHT31_Init,
    .Trigger = SHT31_Trigger,
    .Read = SHT31_Read
};

// Global pointer to active sensor
SensorDriver_t Sensor;
