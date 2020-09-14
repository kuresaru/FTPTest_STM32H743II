#ifndef __IOE_H
#define __IOE_H
#include "stm32h7xx.h"

#define PCF8574_ADDR 	0X40

#define BEEP_IO         0
#define AP_INT_IO       1
#define DCMI_PWDN_IO    2
#define USB_PWR_IO      3
#define EX_IO      		4
#define MPU_INT_IO      5
#define RS485_RE_IO     6
#define ETH_RESET_IO    7

void ioe_init();
uint8_t ioe_read();
void ioe_write(uint8_t data);
uint8_t ioe_readbit(uint8_t bit);
void ioe_writebit(uint8_t bit, uint8_t val);

#endif