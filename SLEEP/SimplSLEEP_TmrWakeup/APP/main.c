#include "SWM181.h"


int main(void)
{	
	uint32_t i;
	
	SystemInit();
	
	GPIO_Init(GPIOA, PIN4, 1, 0, 0, 0);				//接 LED，指示程序执行状态
	
	for(i = 0; i < SystemCoreClock; i++);			//延时，防止上电后SWD立即切掉无法下载程序
	
	SYS->TWKTIM = 32768 * 3;						//睡眠 3秒钟唤醒
	SYS->TWKCR |= (1 << SYS_TWKCR_EN_Pos);			//开启定时器唤醒
	
													//特别提醒：执行下面这一句后SWD两个引脚的数字输入就关闭了，，仿真器就连不上了
	PORTA->INEN = 0;								//进入睡眠前须关闭所有引脚的数字输入功能，以降低睡眠功耗
	PORTB->INEN = 0;
	PORTC->INEN = 0;
	PORTD->INEN = 0;
	
	while(1==1)
	{
		GPIO_SetBit(GPIOA, PIN4);					//点亮LED
		for(i = 0; i < SystemCoreClock/10; i++);
		GPIO_ClrBit(GPIOA, PIN4);					//熄灭LED
		
		EnterSleepMode();							//进入睡眠模式
		
		while((SYS->TWKCR & SYS_TWKCR_ST_Msk) == 0);//等待唤醒条件
		SYS->TWKCR |= (1 << SYS_TWKCR_ST_Msk);		//清除唤醒状态
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
