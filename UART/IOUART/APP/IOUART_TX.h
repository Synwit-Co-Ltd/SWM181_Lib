#ifndef __IOUART_TX_H__
#define __IOUART_TX_H__

void IOUART_TX_Init(void);

void IOUART_TX_SendChars(char buff[], uint32_t size);
uint32_t IOUART_TX_IsBusy(void);

#endif //__IOUART_TX_H__
