#include "SWM181.h"


void SerialInit(void);

int main(void)
{		
	uint32_t addr, temp;
	
	CACHE->CR = (1 << CACHE_CR_RST_Pos) | (CACHE_ALG_LRU << CACHE_CR_ALG_Pos);
	for(addr = 0; addr < 0x800; addr += 128)	// 将程序前2K锁存在Cache中，在分散加载文件中将向量表和中断服务程序定位到程序的前2K
	{
		temp = CACHE_Prefetch(addr);
		temp = temp;
	}
	
	SystemInit();
	
	SerialInit();
   	
	SysTick_Config(SystemCoreClock / 10);
	
	while(1==1)
	{
	}
}


__attribute__((section("..LockInCache")))
void SysTick_Handler(void)
{
	
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
