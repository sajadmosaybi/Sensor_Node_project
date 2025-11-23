#include "sensor.h"
#include "sensor_sht31.h"
#include "rfm95.h"             // LoRa driver
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart2;

// LoRa handle
RFM95_Handle_t rfm;

// Timing intervals (ms)
#define SENSOR_READ_INTERVAL_MS 30000  // 30 sec
#define LORA_SEND_INTERVAL_MS   60000  // 1 min

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_SPI1_Init();
    MX_USART2_UART_Init();

    // --- Select the sensor driver ---
    Sensor = SHT31_Driver;

    // --- Initialize sensor ---
    if (Sensor.Init(
        (Sensor.interface == SENSOR_IF_I2C) ? (void*)&hi2c1 :
        (Sensor.interface == SENSOR_IF_SPI) ? (void*)&hspi1 :
        (void*)&huart2
    ) != HAL_OK) {
        printf("Sensor init failed!\r\n");
    }

    // --- Initialize LoRa RFM95 ---
    rfm.hspi = &hspi1;
    rfm.cs_port = GPIOA;  // change if needed
    rfm.cs_pin = GPIO_PIN_4;
    RFM95_Init(&rfm);

    // Timing variables
    uint32_t last_read = HAL_GetTick();
    uint32_t last_send = HAL_GetTick();
    SensorData_t current_data = {0};

    while (1) {
        uint32_t now = HAL_GetTick();

        // --- Sensor Reading ---
        if ((now - last_read) >= SENSOR_READ_INTERVAL_MS) {
            Sensor.Trigger(
                (Sensor.interface == SENSOR_IF_I2C) ? (void*)&hi2c1 :
                (Sensor.interface == SENSOR_IF_SPI) ? (void*)&hspi1 :
                (void*)&huart2
            );
            HAL_Delay(15); // SHT31 measurement time
            current_data = Sensor.Read(
                (Sensor.interface == SENSOR_IF_I2C) ? (void*)&hi2c1 :
                (Sensor.interface == SENSOR_IF_SPI) ? (void*)&hspi1 :
                (void*)&huart2
            );
            last_read = now;

            if (current_data.error == 0)
                printf("Sensor Read -> T=%.2f C, RH=%.2f%%\r\n",
                        current_data.temperature_c,
                        current_data.humidity_rh);
            else
                printf("Sensor read error: %d\r\n", current_data.error);
        }

        // --- LoRa Transmission ---
        if ((now - last_send) >= LORA_SEND_INTERVAL_MS) {
            if (current_data.error == 0) {
                char buf[32];
                int len = sprintf(buf, "T=%.2fC,RH=%.2f%%",
                                  current_data.temperature_c,
                                  current_data.humidity_rh);
                RFM95_Send(&rfm, (uint8_t*)buf, len);
                printf("LoRa Sent: %s\r\n", buf);
            }
            last_send = now;
        }

        HAL_Delay(100); // small delay to reduce CPU usage
    }
}
