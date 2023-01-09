#include "SWM181.h"


int32_t ADC_Result[1000];

void SerialInit(void);

int main(void)
{
	ADC_InitStructure ADC_initStruct;
	
	SystemInit();
	
	SerialInit();
   	
	
	PORT_Init(PORTE, PIN4,  PORTE_PIN4_ADC_CH0, 0);	//PE.4  => ADC.CH0
	PORT_Init(PORTA, PIN15, PORTA_PIN15_ADC_CH1, 0);//PA.15 => ADC.CH1
	PORT_Init(PORTA, PIN14, PORTA_PIN14_ADC_CH2, 0);//PA.14 => ADC.CH2
	PORT_Init(PORTA, PIN13, PORTA_PIN13_ADC_CH3, 0);//PA.13 => ADC.CH3
	PORT_Init(PORTA, PIN12, PORTA_PIN12_ADC_CH4, 0);//PA.12 => ADC.CH4
	PORT_Init(PORTC, PIN7,  PORTC_PIN7_ADC_CH5, 0);	//PC.7  => ADC.CH5
	PORT_Init(PORTC, PIN6,  PORTC_PIN6_ADC_CH6, 0);	//PC.6  => ADC.CH6
													//ADC_CH7 => ADC.CH7
	
	ADC_initStruct.clk_src = ADC_CLKSRC_HRC_DIV4;
	ADC_initStruct.channels = ADC_CH7;				//使能通道7
	ADC_initStruct.samplAvg = ADC_AVG_SAMPLE1;
	ADC_initStruct.trig_src = ADC_TRIGSRC_SW;
	ADC_initStruct.Continue = 1;					//连续模式
	ADC_initStruct.EOC_IEn = 0;						//使用DMA就不需要使用ADC中断了	
	ADC_initStruct.OVF_IEn = 0;
	ADC_Init(ADC, &ADC_initStruct);					//配置ADC
	
	DMA_CH_Config(DMA_CHR_ADC, (uint32_t)ADC_Result, 1000, 1);
	
	IRQ_Connect(IRQ0_15_DMA, IRQ5_IRQ, 1);
	
	DMA_CH_Open(DMA_CHR_ADC);
	
	ADC_Open(ADC);									//使能ADC
	ADC_Start(ADC);									//启动ADC，开始转换
	
	while(1==1)
	{
		
	}
}

void IRQ5_Handler(void)
{
	uint32_t i, res, chn;
	
	if(DMA_CH_INTStat(DMA_CHR_ADC))
	{
		ADC_Stop(ADC);
		
		DMA_CH_INTClr(DMA_CHR_ADC);		//清除中断标志
		
		for(i = 0; i < 1000; i++)
		{
			chn = (ADC_Result[i] >> 12) & 0x07;
			res = (ADC_Result[i] & 0xFFF);
			printf("%d,", res);
		}
		
		DMA_CH_Open(DMA_CHR_ADC);		//在传输完成后通道开启会自动清零
		ADC_Start(ADC);
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
