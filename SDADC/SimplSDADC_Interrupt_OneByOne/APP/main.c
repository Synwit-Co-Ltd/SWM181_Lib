#include "SWM181.h"

uint16_t SDADC_Result[6];

void SerialInit(void);

int main(void)
{
	SDADC_InitStructure SDADC_initStruct;
	
	SystemInit();
	
	SerialInit();
	
	
	PORT_Init(PORTC, PIN6, PORTC_PIN6_SDADC_CH0P, 0);	//PC.6 => SDADC.CH0P
	PORT_Init(PORTC, PIN7, PORTC_PIN7_SDADC_CH0N, 0);	//PC.7 => SDADC.CH0N
	PORT_Init(PORTC, PIN4, PORTC_PIN4_SDADC_CH1P, 0);	//PC.4 => SDADC.CH1P
	PORT_Init(PORTC, PIN5, PORTC_PIN5_SDADC_CH1N, 0);	//PC.5 => SDADC.CH1N
	PORT_Init(PORTC, PIN2, PORTC_PIN2_SDADC_CH3P, 0);	//PC.2 => SDADC.CH3P
	PORT_Init(PORTC, PIN3, PORTC_PIN3_SDADC_CH3N, 0);	//PC.3 => SDADC.CH3N
	PORT_Init(PORTD, PIN0, PORTD_PIN0_SDADC_CH4P, 0);	//PD.0 => SDADC.CH4P
	PORT_Init(PORTD, PIN1, PORTD_PIN1_SDADC_CH4N, 0);	//PD.1 => SDADC.CH4N
	PORT_Init(PORTB, PIN14,PORTB_PIN14_SDADC_CH5P,0);	//PB.14=> SDADC.CH5P
	PORT_Init(PORTB, PIN15,PORTB_PIN15_SDADC_CH5N,0);	//PB.15=> SDADC.CH5N
	
	
	SDADC_initStruct.clk_src = SDADC_CLKSRC_HRC_DIV8;
	SDADC_initStruct.channels = SDADC_CH0;
	SDADC_initStruct.out_cali = SDADC_OUT_CALIED;
	SDADC_initStruct.refp_sel = SDADC_REFP_AVDD;
	SDADC_initStruct.trig_src = SDADC_TRIGSRC_SW;
	SDADC_initStruct.Continue = 0;						//������ģʽ��������ģʽ
	SDADC_initStruct.EOC_IEn = 1;	
	SDADC_initStruct.OVF_IEn = 0;
	SDADC_initStruct.HFULL_IEn = 0;
	SDADC_initStruct.FULL_IEn = 0;
	SDADC_Init(SDADC, &SDADC_initStruct);				//����SDADC
	
	SDADC_Config_Set(SDADC, SDADC_CFG_A, SDADC_CFG_GAIN_1, 1, 1);
	SDADC_Config_Cali(SDADC, SDADC_CFG_A, SDADC_CALI_COM_GND, 0);
	SDADC_Config_Sel(SDADC, SDADC_CFG_A, SDADC_CH0 | SDADC_CH1 | SDADC_CH2);
	SDADC_Config_Set(SDADC, SDADC_CFG_B, SDADC_CFG_GAIN_1, 1, 0);
	SDADC_Config_Cali(SDADC, SDADC_CFG_B, SDADC_CALI_COM_GND, 0);
	SDADC_Config_Sel(SDADC, SDADC_CFG_B, SDADC_CH3 | SDADC_CH4 | SDADC_CH5);
	
	IRQ_Connect(IRQ0_15_SDADC, IRQ5_IRQ, 1);
	
	SDADC_Open(SDADC);									//ʹ��SDADC
	SDADC_Start(SDADC);
	
	while(1==1)
	{
		printf("%6d,", SDADC_Result[2]);
	}
}

void IRQ5_Handler(void)
{
	int16_t res;
	uint32_t i, chn;
	
	SDADC_IntEOCClr(SDADC);			//����жϱ�־
	
	res = SDADC_Read(SDADC, &chn);
	SDADC_Result[chn] = res + 32768;
	
	SDADC_Close(SDADC);		//ֻ���ڹر�ʱ�л�ͨ��
	switch(chn)
	{
	case 0:
		SDADC_ChnClose(SDADC, SDADC_CH0);	//�ر�ͨ��0
		SDADC_ChnOpen(SDADC, SDADC_CH1);	//����ͨ��1
		break;
	
	case 1:
		SDADC_ChnClose(SDADC, SDADC_CH1);	//�ر�ͨ��1
		SDADC_ChnOpen(SDADC, SDADC_CH2);	//����ͨ��2
		break;
	
	case 2:
		SDADC_ChnClose(SDADC, SDADC_CH2);	//�ر�ͨ��2
		SDADC_ChnOpen(SDADC, SDADC_CH0);	//����ͨ��0
		break;
	}
	SDADC_Open(SDADC);
	
	for(i=0;i<1000;i++);			//����ʱ�Ǳ���ģ����̺��п��ܵ���SDADC�޷�����
	
	SDADC_Start(SDADC);				//�ٴ����������ϵõ��������ӡ
}

void SerialInit(void)
{
	UART_InitStructure UART_initStruct;
	
	PORT_Init(PORTA, PIN0, FUNMUX_UART0_RXD, 1);		//GPIOA.0����ΪUART0��������
	PORT_Init(PORTA, PIN1, FUNMUX_UART0_TXD, 0);		//GPIOA.1����ΪUART0�������
 	
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
* ��������: fputc()
* ����˵��: printf()ʹ�ô˺������ʵ�ʵĴ��ڴ�ӡ����
* ��    ��: int ch		Ҫ��ӡ���ַ�
*			FILE *f		�ļ����
* ��    ��: ��
* ע������: ��
******************************************************************************************************************************************/
int fputc(int ch, FILE *f)
{
	UART_WriteByte(UART0, ch);
	
	while(UART_IsTXBusy(UART0));
 	
	return ch;
}