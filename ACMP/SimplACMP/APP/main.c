#include "SWM181.h"

void SerialInit(void);

int main(void)
{
	SystemInit();
	
	SerialInit();
   	
	PORT_Init(PORTB, PIN2, PORTB_PIN2_CMP1N, 0);
	PORT_Init(PORTB, PIN3, PORTB_PIN3_CMP1P, 0);
	
	CMP_Init(CMP1, 0, 1, 1);
	CMP_SetVRef(CMP_VREF_1V50);
	
	IRQ_Connect(IRQ0_15_CMP, IRQ7_IRQ, 2);
	
	CMP_Open(CMP1);
	
	while(1==1)
	{
		
	}
}

void IRQ7_Handler(void)
{
	printf("IRQ7 happened!\r\n");
 	
	CMP_INTClr(CMP1);
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
