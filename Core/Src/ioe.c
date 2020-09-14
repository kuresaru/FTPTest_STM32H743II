#include "ioe.h"
#include "i2c.h"

void ioe_init()
{
    uint8_t data = 0xFF;
    HAL_I2C_Master_Transmit(&hi2c2, PCF8574_ADDR, &data, 1, 1000);
}

uint8_t ioe_read()
{
    uint8_t buf;
    HAL_I2C_Master_Receive(&hi2c2, PCF8574_ADDR, &buf, 1, 1000);
    return buf;
}

void ioe_write(uint8_t data)
{
    HAL_I2C_Master_Transmit(&hi2c2, PCF8574_ADDR, &data, 1, 1000);
}

uint8_t ioe_readbit(uint8_t bit)
{
    return (ioe_read() & (1 << bit)) ? 1 : 0;
}

void ioe_writebit(uint8_t bit, uint8_t val)
{
    uint8_t data = ioe_read();
    data &= ~(1 << bit);
    if (val)
        data |= (1 << bit);
    ioe_write(data);
}