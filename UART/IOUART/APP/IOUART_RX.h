#ifndef __IOUART_RX_H__
#define __IOUART_RX_H__

void IOUART_RX_Init(void);

void IOUART_RX_RecvChars(char buff[], uint32_t size);

uint32_t IOUART_RX_State(void);
uint32_t IOUART_RX_Count(void);
void IOUART_RX_Clear(void);

#define IOUART_RX_IDLE		0	//空闲中，接收到停止位进入
#define IOUART_RX_BUSY		1	//接收中，接收到起始位进入
#define IOUART_RX_BUFFULL	2	//接收满，停止接收
#define IOUART_RX_TIMEOUT	3	//收超时，停止接收
#define IOUART_RX_FRAMERR	4	//帧错误，停止接收
#define IOUART_RX_PARITYERR 5	//奇偶校验错误，停止接收

#endif //__IOUART_RX_H__
