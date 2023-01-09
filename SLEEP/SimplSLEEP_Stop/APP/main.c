#include "SWM181.h"

void SerialInit(void);

/* 进入最低功耗休眠模式（Stop模式），只能通过A0引脚下降沿唤醒，唤醒后程序从头开始重新执行 */

int main(void)
{	
	uint32_t i, j;
	
	SystemInit();
	
	SerialInit();
// 	printf("Hi from Synwit\r\n");
	
	GPIO_Init(GPIOA, PIN4, 1, 0, 0, 0);			//接 LED，指示程序执行状态
	GPIO_ClrBit(GPIOA, PIN4);
	for(i = 0; i < 4; i++)
	{
		GPIO_InvBit(GPIOA, PIN4);
		for(j = 0; j < 5000000; j++) __NOP();	//不要注释掉，防止上电后立即进入Stop模式没法JLink更新程序
	}
	
	EnterStopMode();
	
	while(1==1)
	{
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
