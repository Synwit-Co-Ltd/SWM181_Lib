#include "SWM181.h"

#include "SEGGER_RTT.h"


void SerialInit(void);

int main(void)
{
 	uint32_t i, tmp;
	char str[5];
	const char tab[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
 	
 	SystemInit();
	
	SerialInit();
	
	SEGGER_RTT_Init();
	
	GPIO_Init(GPIOA, PIN4, 1, 0, 0, 0);			//GPIOA.4配置为输出引脚，推挽输出
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);			//GPIOA.5配置为输出引脚，推挽输出
	
 	while(1==1)
 	{
 		GPIO_InvBit(GPIOA, PIN4);
 		GPIO_InvBit(GPIOA, PIN5);
 		for(i=0; i<5000000; i++) i=i;
		
  		printf("Hi, World!\r\n");
		
		SEGGER_RTT_printf(0, "Hi, World!\r\n");
		
		GPIO_InvBit(GPIOA, PIN4);
 		GPIO_InvBit(GPIOA, PIN5);
		SEGGER_RTT_printf(0, "H");
		
		GPIO_InvBit(GPIOA, PIN4);
 		GPIO_InvBit(GPIOA, PIN5);
		SEGGER_RTT_printf(0, "%d", 3);
		
		GPIO_InvBit(GPIOA, PIN4);
 		GPIO_InvBit(GPIOA, PIN5);
		SEGGER_RTT_printf(0, "%x", 0xA);
		
		GPIO_InvBit(GPIOA, PIN4);
 		GPIO_InvBit(GPIOA, PIN5);
		SEGGER_RTT_WriteString(0, "H");
		
		tmp = 1345;
		str[0] = tab[(tmp>>12)&0xF];
		str[1] = tab[(tmp>> 8)&0xF];
		str[2] = tab[(tmp>> 4)&0xF];
		str[3] = tab[tmp&0xF];
		str[4] = ',';
		GPIO_InvBit(GPIOA, PIN4);
 		GPIO_InvBit(GPIOA, PIN5);
		SEGGER_RTT_Write(0, str, 5);
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
 	while(UART_IsTXFIFOFull(UART0));
	
	UART_WriteByte(UART0, ch);
 	
	return ch;
}
