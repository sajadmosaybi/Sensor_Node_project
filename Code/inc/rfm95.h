#ifndef RFM95_H
#define RFM95_H

#include "stm32l0xx_hal.h"

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
} RFM95_Handle_t;

void RFM95_Init(RFM95_Handle_t *rfm);
HAL_StatusTypeDef RFM95_Send(RFM95_Handle_t *rfm, uint8_t *data, uint8_t len);

#endif
