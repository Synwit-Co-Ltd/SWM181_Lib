#ifndef __SPIUART_H__
#define __SPIUART_H__

#include <string.h>

#define SPIUART_FRAME_ERR_MASK		(1 << 15)
#define SPIUART_PARITY_ERR_MASK		(1 << 14)

extern volatile uint16_t SPIUART_SendComplete;

void SPIUART_Init(uint16_t baudrate);
void SPIUART_Send(uint8_t buff[], uint16_t cnt);
uint16_t SPIUART_Recv(uint16_t buff[], uint16_t min);
void SPIUART_RecvPrepare(void);

/****************************** ÄÚ²¿º¯Êý ***********************************/
void UART2SPI_encode(uint8_t buff[], uint16_t cnt);
void SPI2UART_decode(uint16_t buff[], uint16_t cnt);


#endif //__SPIUART_H__
