#include "SWM181.h"

#include <RTL.h>

/* 注意：
*	为了简单，在RX_Conf_CM.c中配置所有Task都运行于特权等级，否则当Task中
* 代码写某些内核寄存器（如调用NVIC_DisableIRQ())时就会触发HardFault
************************************************************************/

os_mbx_declare(queueADC, 16);

__task void TaskINI(void);
__task void TaskADC(void);
__task void TaskPWM(void);
void SerialInit(void);

int main(void)
{ 	
 	SystemInit();
	
	SerialInit();
	
	os_sys_init(TaskINI);
}

__task void TaskINI(void)
{
	os_tsk_create(TaskADC, 1);
	os_tsk_create(TaskPWM, 2);
	
	os_mbx_init(&queueADC, sizeof(queueADC));
	
	os_tsk_delete(os_tsk_self());
}

/****************************************************************************************************************************************** 
* 函数名称:	TaskADC()
* 功能说明: 启动ADC采集任务
* 输    入: 无
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
__task void TaskADC(void)
{
	ADC_InitStructure ADC_initStruct;
	
	PORT_Init(PORTE, PIN4,  PORTE_PIN4_ADC_CH0, 0);	//PE.4  => ADC.CH0
	PORT_Init(PORTA, PIN15, PORTA_PIN15_ADC_CH1, 0);//PA.15 => ADC.CH1
	PORT_Init(PORTA, PIN14, PORTA_PIN14_ADC_CH2, 0);//PA.14 => ADC.CH2
	PORT_Init(PORTA, PIN13, PORTA_PIN13_ADC_CH3, 0);//PA.13 => ADC.CH3
	PORT_Init(PORTA, PIN12, PORTA_PIN12_ADC_CH4, 0);//PA.12 => ADC.CH4
	PORT_Init(PORTC, PIN7,  PORTC_PIN7_ADC_CH5, 0);	//PC.7  => ADC.CH5
	PORT_Init(PORTC, PIN6,  PORTC_PIN6_ADC_CH6, 0);	//PC.6  => ADC.CH6
													//ADC_CH7 => ADC.CH7
	
	ADC_initStruct.clk_src = ADC_CLKSRC_HRC_DIV4;
	ADC_initStruct.channels = ADC_CH6;
	ADC_initStruct.samplAvg = ADC_AVG_SAMPLE1;
	ADC_initStruct.trig_src = ADC_TRIGSRC_TIMR3;
	ADC_initStruct.Continue = 0;					//非连续模式，即单次模式
	ADC_initStruct.EOC_IEn = ADC_CH6;	
	ADC_initStruct.OVF_IEn = 0;
	ADC_Init(ADC, &ADC_initStruct);					//配置ADC
	
	IRQ_Connect(IRQ0_15_ADC, IRQ5_IRQ, 1);
	
	ADC_Open(ADC);									//使能ADC
	
	TIMR_Init(TIMR3, TIMR_MODE_TIMER, SystemCoreClock/10, 0);
	TIMR_Start(TIMR3);
	
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);				//调试指示信号
	
	while(1)
	{
		// do nothing
		
		os_dly_wait(500);
	}
}

void IRQ5_Handler(void)
{		
	uint32_t val;
	
	ADC_IntEOCClr(ADC, ADC_CH6);	//清除中断标志
	
	val = ADC_Read(ADC, ADC_CH6);
	
	isr_mbx_send(&queueADC, (void *)val);
	
	GPIO_InvBit(GPIOA, PIN5);
}

/****************************************************************************************************************************************** 
* 函数名称:	TaskPWM()
* 功能说明: 等待ADC转换结果，根据ADC转换结果设置PWM占空比
* 输    入: 无
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
__task void TaskPWM(void)
{
	uint16_t duty;
	PWM_InitStructure PWM_initStruct;
	
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
	
	PORT_Init(PORTA, PIN6, FUNMUX_PWM1A_OUT, 0);
	PORT_Init(PORTA, PIN7, FUNMUX_PWM1B_OUT, 0);
	
	PWM_Start(PWM1, 1, 1);
	
	while(1)
	{
		if(os_mbx_wait(queueADC, (void *)&duty, 10) != OS_R_TMO)
		{
			if(duty <  100) duty =  100;
			if(duty > 4000) duty = 4000;
			
			PWM_SetHDuty(PWM1, PWM_CH_A, 10000 * duty / 4095);
			PWM_SetHDuty(PWM1, PWM_CH_B, 10000 - PWM_GetHDuty(PWM1, PWM_CH_A));
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
