#include "SWM181.h"


volatile uint32_t PulseWidth = 0;

void SerialInit(void);
void TestSignal(void);

int main(void)
{	
	uint32_t i;
	
	SystemInit();
	
	SerialInit();
	
	TestSignal();		// 产生测试信号供Pulse功能测量
	
	PORT_Init(PORTA, PIN4, FUNMUX_PULSE_IN, 1);		// PA4 -> Pulse_IN
	
	TIMR_Init(TIMR0, TIMR_MODE_TIMER, 0xFFFFFFFF, 0);
	
	TIMRG->PCTRL = (1 << TIMRG_PCTRL_EN_Pos) |
				   (1 << TIMRG_PCTRL_HIGH_Pos);		// 测量高电平长度
	TIMRG->IE |=   (1 << TIMRG_IE_PULSE_Pos);
	
	IRQ_Connect(IRQ0_15_PULSE, IRQ4_IRQ, 1);
		
	while(1==1)
	{		
		printf("Pulse Width = %d\r\n", PulseWidth);
		
		for(i = 0; i < 5000000; i++);
	}
}

void IRQ4_Handler(void)
{
	if(TIMRG->IF & TIMRG_IF_PULSE_Msk)
	{
		TIMRG->IF = (1 << TIMRG_IF_PULSE_Pos);
		
		PulseWidth = TIMRG->PCVAL;
		
 		TIMRG->PCTRL |= (1 << TIMRG_PCTRL_EN_Pos);		// 再次测量
	}
}

void TestSignal(void)
{
	PWM_InitStructure  PWM_initStruct;
	
	PWM_initStruct.clk_div = PWM_CLKDIV_4;		//F_PWM = 24M/4 = 6M
	
	PWM_initStruct.mode = PWM_MODE_INDEP;		//A路和B路独立输出					
	PWM_initStruct.cycleA = 10000;				//6M/10000 = 600Hz			
	PWM_initStruct.hdutyA =  2500;				//2500/10000 = 25%
	PWM_initStruct.initLevelA = 1;
	PWM_initStruct.cycleB = 10000;
	PWM_initStruct.hdutyB =  5000;				//5000/10000 = 50%
	PWM_initStruct.initLevelB = 1;
	PWM_initStruct.HEndAIEn = 0;
	PWM_initStruct.NCycleAIEn = 0;
	PWM_initStruct.HEndBIEn = 0;
	PWM_initStruct.NCycleBIEn = 0;
	
	PWM_Init(PWM1, &PWM_initStruct);
	
	PORT_Init(PORTA, PIN5, FUNMUX_PWM1B_OUT, 0);
	
	PWM_Start(PWM1, 1, 1);
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
