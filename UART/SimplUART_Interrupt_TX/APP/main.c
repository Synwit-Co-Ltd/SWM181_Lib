#include <string.h>
#include "SWM181.h"


void SerialInit(void);
void UART_SendChars(char data[], uint32_t len);

int main(void)
{
	int i;
	char str_Hi[] = "Hello from Synwit UART!\r\n";
	
	SystemInit();
	
	SerialInit();
	
	GPIO_Init(GPIOA, PIN4, 1, 0, 0, 0);
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);
   	
	while(1==1)
	{
		UART_SendChars(str_Hi, strlen(str_Hi));
		
		for(i = 0; i < SystemCoreClock/4; i++) __NOP();
	}
}


char * UART_TXBuffer = 0;
uint32_t UART_TXCount = 0,
         UART_TXIndex = 0;

void UART_SendChars(char data[], uint32_t len)
{
	UART_TXBuffer = data;
	UART_TXCount = len;
	UART_TXIndex = 0;
	
	UART_INTEn(UART0, UART_IT_TX_THR | UART_IT_TX_DONE);
}


void IRQ0_Handler(void)
{
	if(UART_INTStat(UART0, UART_IT_TX_THR))
	{
		while(UART_IsTXFIFOFull(UART0) == 0)
		{
			if(UART_TXIndex < UART_TXCount)
			{
				UART_WriteByte(UART0, UART_TXBuffer[UART_TXIndex++]);
			}
			else
			{
				UART_INTDis(UART0, UART_IT_TX_THR);
				
				break;
			}
		}
		
		GPIO_InvBit(GPIOA, PIN4);
	}
	else if(UART_INTStat(UART0, UART_IT_TX_DONE))
	{
		GPIO_InvBit(GPIOA, PIN5);
		
		UART_INTDis(UART0, UART_IT_TX_DONE);
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
	UART_initStruct.RXThresholdIEn = 0;
	UART_initStruct.TXThreshold = 3;
	UART_initStruct.TXThresholdIEn = 0;
	UART_initStruct.TimeoutTime = 10;
	UART_initStruct.TimeoutIEn = 0;
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
