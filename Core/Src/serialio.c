#include "stm32h7xx.h"

extern UART_HandleTypeDef huart1;

int _write(int fd, char *ptr, int len)
{
    (void)fd;
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, (uint16_t)len, 1000);
    return len;
}