#include "SWM181.h"


void SerialInit(void);

int main(void)
{
	uint32_t i;
	SLCD_InitStructure SLCD_initStruct;
	
	SystemInit();
	
	SerialInit();
	
	PORT_Init(PORTA, PIN4, PORTA_PIN4_LCD_COM0, 0);
	PORT_Init(PORTA, PIN5, PORTA_PIN5_LCD_COM1, 0);
	PORT_Init(PORTA, PIN6, PORTA_PIN6_LCD_COM2, 0);
	PORT_Init(PORTA, PIN7, PORTA_PIN7_LCD_COM3, 0);
	
	PORT_Init(PORTB, PIN0, PORTB_PIN0_LCD_SEG0, 0);
	PORT_Init(PORTB, PIN1, PORTB_PIN1_LCD_SEG1, 0);
	PORT_Init(PORTB, PIN2, PORTB_PIN2_LCD_SEG2, 0);
	PORT_Init(PORTB, PIN3, PORTB_PIN3_LCD_SEG3, 0);
	
	SLCD_initStruct.Duty = SLCD_DUTY_1DIV4;
	SLCD_initStruct.Bias = SLCD_BIAS_1DIV2;
	SLCD_initStruct.FrameFreq = SLCD_FRAMEFREQ_16Hz;
	SLCD_initStruct.DriveCurr = SLCD_DRIVECURR_25uA;
	SLCD_Init(SLCD, &SLCD_initStruct);
	
	SLCD_Open(SLCD);
	
	while(1==1)
	{
		SLCD_AllOn(SLCD);
		for(i = 0; i < 10000000; i++);
		
		SLCD_Clear(SLCD);
		for(i = 0; i < 10000000; i++);
		
		SLCD_SegWrite(SLCD_COM0, SLCD_SEG0, 1);
		SLCD_SegWrite(SLCD_COM1, SLCD_SEG1, 1);
		SLCD_SegWrite(SLCD_COM2, SLCD_SEG2, 1);
		SLCD_SegWrite(SLCD_COM3, SLCD_SEG3, 1);
		for(i = 0; i < 10000000; i++);
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
