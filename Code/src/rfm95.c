#include "rfm95.h"

// Register addresses (basic subset)
#define REG_FIFO 0x00
#define REG_OP_MODE 0x01
#define REG_FIFO_ADDR_PTR 0x0D
#define REG_PAYLOAD_LENGTH 0x22
#define REG_FIFO_TX_BASE_ADDR 0x0E
#define REG_DIO_MAPPING1 0x40

#define MODE_LONG_RANGE_MODE 0x80
#define MODE_SLEEP 0x00
#define MODE_STDBY 0x01
#define MODE_TX 0x03

static void rfm95_write(RFM95_Handle_t *rfm, uint8_t addr, uint8_t data)
{
    uint8_t buf[2] = {addr | 0x80, data};
    HAL_GPIO_WritePin(rfm->cs_port, rfm->cs_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(rfm->hspi, buf, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(rfm->cs_port, rfm->cs_pin, GPIO_PIN_SET);
}

static void rfm95_write_buf(RFM95_Handle_t *rfm, uint8_t addr, uint8_t *data, uint8_t len)
{
    addr |= 0x80;
    HAL_GPIO_WritePin(rfm->cs_port, rfm->cs_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(rfm->hspi, &addr, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(rfm->hspi, data, len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(rfm->cs_port, rfm->cs_pin, GPIO_PIN_SET);
}

void RFM95_Init(RFM95_Handle_t *rfm)
{
    // put in sleep mode first
    rfm95_write(rfm, REG_OP_MODE, MODE_SLEEP | MODE_LONG_RANGE_MODE);
    HAL_Delay(10);
    // standby
    rfm95_write(rfm, REG_OP_MODE, MODE_STDBY | MODE_LONG_RANGE_MODE);
    HAL_Delay(10);
    // FIFO base addr
    rfm95_write(rfm, REG_FIFO_TX_BASE_ADDR, 0x00);
    rfm95_write(rfm, REG_FIFO_ADDR_PTR, 0x00);
    // DIO0 => TxDone
    rfm95_write(rfm, REG_DIO_MAPPING1, 0x40);
}

HAL_StatusTypeDef RFM95_Send(RFM95_Handle_t *rfm, uint8_t *data, uint8_t len)
{
    // reset FIFO pointer
    rfm95_write(rfm, REG_FIFO_ADDR_PTR, 0x00);
    // write payload
    rfm95_write_buf(rfm, REG_FIFO, data, len);
    rfm95_write(rfm, REG_PAYLOAD_LENGTH, len);
    // start transmission
    rfm95_write(rfm, REG_OP_MODE, MODE_TX | MODE_LONG_RANGE_MODE);

    // wait TxDone flag (polling)
    uint8_t irq = 0;
    do {
        HAL_Delay(1);
        // read IRQ register here if implemented
        // skipping for minimal example
    } while (irq == 0);

    return HAL_OK;
}
