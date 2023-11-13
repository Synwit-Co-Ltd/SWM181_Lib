#include "SWM181.h"
#include "CircleBuffer.h"

/* 说明：只有当接收 FIFO 中有数据，且在指定时间内未接收到新的数据时，才会触发超时中断。
 * 若应用中希望通过数据间时间间隔作为帧间隔依据，即不管对方发送过来多少个数据，
 * 最后都能产生超时中断，可以通过在接收 ISR 中从 RX FIFO 中读取数据时总是少读一个（即让一个数据留在 RX FIFO 中）来实现。
*/

volatile bool msg_rcvd = false;

CircleBuffer_t CirBuf;

void SerialInit(void);

int main(void)
{
	SystemInit();
	
	SerialInit();
   	
	while(1==1)
	{
		if(msg_rcvd)
		{
			msg_rcvd = false;
			
			UART_INTEn(UART0, UART_IT_TX_THR);
		}
	}
}


void IRQ0_Handler(void)
{
	uint32_t chr;
	
	if(UART_INTStat(UART0, UART_IT_RX_THR))
	{
		while((UART0->FIFO & UART_FIFO_RXLVL_Msk) > 1)
		{
			if(UART_ReadByte(UART0, &chr) == 0)
			{
				CirBuf_Write(&CirBuf, (uint8_t *)&chr, 1);
			}
		}
	}
	else if(UART_INTStat(UART0, UART_IT_RX_TOUT))
	{
		while(UART_IsRXFIFOEmpty(UART0) == 0)
		{
			if(UART_ReadByte(UART0, &chr) == 0)
			{
				CirBuf_Write(&CirBuf, (uint8_t *)&chr, 1);
			}
		}
		
		msg_rcvd = true;
	}
	
	if(UART_INTStat(UART0, UART_IT_TX_THR))
	{
		while(!UART_IsTXFIFOFull(UART0))
		{
			if(!CirBuf_Empty(&CirBuf))
			{
				CirBuf_Read(&CirBuf, (uint8_t *)&chr, 1);
				
				UART_WriteByte(UART0, chr);
			}
			else
			{
				UART_INTDis(UART0, UART_IT_TX_THR);
				
				break;
			}
		}
	}
}


void SerialInit(void)
{
	UART_InitStructure UART_initStruct;
	
	PORT_Init(PORTA, PIN0, FUNMUX_UART0_RXD, 1);	//GPIOA.0配置为UART0输入引脚
	PORT_Init(PORTA, PIN1, FUNMUX_UART0_TXD, 0);	//GPIOA.1配置为UART0输出引脚
 	
 	UART_initStruct.Baudrate = 57600;
	UART_initStruct.DataBits = UART_DATA_8BIT;
	UART_initStruct.Parity = UART_PARITY_NONE;
	UART_initStruct.StopBits = UART_STOP_1BIT;
	UART_initStruct.RXThreshold = 3;
	UART_initStruct.RXThresholdIEn = 1;
	UART_initStruct.TXThreshold = 3;
	UART_initStruct.TXThresholdIEn = 0;
	UART_initStruct.TimeoutTime = 10;		//10个字符时间内未接收到新的数据则触发超时中断
	UART_initStruct.TimeoutIEn = 1;
 	UART_Init(UART0, &UART_initStruct);
	
	IRQ_Connect(IRQ0_15_UART0, IRQ0_IRQ, 1);
	
	UART_Open(UART0);
}

/****************************************************************************************************************************************** 
* 函数名称: fputc()
* 功能说明: printf()使用此函数完成实际的串口打印动作
* 输    入: int ch		要打印的字符
*			FILE *f		文件句柄
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
int fputc(int ch, FILE *f)
{
	UART_WriteByte(UART0, ch);
	
	while(UART_IsTXBusy(UART0));
 	
	return ch;
}
