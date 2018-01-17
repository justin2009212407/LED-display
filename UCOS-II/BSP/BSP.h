#ifndef _BSP_H
#define _BSP_H
#include <stdint.h>
#include "includes.h"

#define LED1 0
#define LED2 1

#define BUFFER_SIZE		256
struct serial_buffer_s
{
	uint8_t buf[BUFFER_SIZE];
	uint8_t putIndex;
	uint8_t getIndex;
	uint8_t length;
	uint8_t wanted_num;
	OS_EVENT *xSemaphore;
};

void BSP_Init(void);
void USART3_Config(void);

void led_on(uint8_t);
void led_off(uint8_t);
void RS485_send_bytes(uint8_t *buf, uint16_t size);
uint16_t RS485_recieve_bytes(uint8_t *buf, uint16_t size);

#endif
