#include "SWM181.h"

#include "SPIUART.h"

#include "SEGGER_RTT.h"


void SerialInit(void);

int main(void)
{	
	uint32_t i, cnt;
	uint16_t buff[64];
	
	SystemInit();
	
	SerialInit();
	
	SPIUART_Init(9600);
	
	while(1==1)
	{
resend:
		for(i = 0; i < SystemCoreClock/1000; i++) __NOP();
		SPIUART_Send((uint8_t *)"Hi from SPIUART!\n", strlen("Hi from SPIUART!\n"));
		while(SPIUART_SendComplete == 0) __NOP();
		SPIUART_SendComplete = 0;
		
		SPIUART_RecvPrepare();
		while(1==1)
		{
			cnt = SPIUART_Recv(buff, 8);
			if(cnt != 0)
			{
				for(i = 0; i < cnt; i++)
				{
					if(buff[i] & SPIUART_FRAME_ERR_MASK)
					{
						SEGGER_RTT_printf(0, "Frame error!\n");
					}
					else if(buff[i] & SPIUART_PARITY_ERR_MASK)
					{
						SEGGER_RTT_printf(0, "Parity error!\n");
					}
					else
					{
						SEGGER_RTT_printf(0, "%02X ", buff[i] & 0xFF);
					}
				}
				SEGGER_RTT_printf(0, "\n");
				
				goto resend;
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
	UART_initStruct.RXThresholdIEn = 0;
	UART_initStruct.TXThresholdIEn = 0;
	UART_initStruct.TimeoutIEn = 0;
 	UART_Init(UART0, &UART_initStruct);
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
