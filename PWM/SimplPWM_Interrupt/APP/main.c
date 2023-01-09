#include "SWM181.h"


int main(void)
{
	PWM_InitStructure  PWM_initStruct;
	
	SystemInit();
	
	GPIO_Init(GPIOA, PIN6, 1, 0, 0, 0);			//反转PA6，指示PWM新周期开始中断位置
	GPIO_Init(GPIOA, PIN7, 1, 0, 0, 0);			//反转PA7，指示PWM高电平结束中断位置
	
	PWM_initStruct.clk_div = PWM_CLKDIV_4;		//F_PWM = 24M/4 = 6M
	
	PWM_initStruct.mode = PWM_MODE_INDEP;		//A路和B路独立输出					
	PWM_initStruct.cycleA = 10000;				//6M/10000 = 600Hz			
	PWM_initStruct.hdutyA =  2500;				//2500/10000 = 25%
	PWM_initStruct.deadzoneA = 10;
	PWM_initStruct.initLevelA = 1;
	PWM_initStruct.cycleB = 10000;
	PWM_initStruct.hdutyB =  5000;				//5000/10000 = 50%
	PWM_initStruct.deadzoneB = 10;
	PWM_initStruct.initLevelB = 1;
	PWM_initStruct.HEndAIEn = 1;
	PWM_initStruct.NCycleAIEn = 1;
	PWM_initStruct.HEndBIEn = 0;
	PWM_initStruct.NCycleBIEn = 0;
	
	PWM_Init(PWM1, &PWM_initStruct);
	
	PORT_Init(PORTA, PIN4, FUNMUX_PWM1A_OUT, 0);
	PORT_Init(PORTA, PIN5, FUNMUX_PWM1B_OUT, 0);
	
	IRQ_Connect(IRQ0_15_PWM1, IRQ1_IRQ, 1);
	
	PWM_Start(PWM1, 1, 1);
	
	while(1==1)
	{
	
	}
}

void IRQ1_Handler(void)
{
	if(PWM_IntNCycleStat(PWM1, PWM_CH_A))
	{
		PWM_IntNCycleClr(PWM1, PWM_CH_A);
		
		GPIO_InvBit(GPIOA, PIN6);
	}
	else if(PWM_IntHEndStat(PWM1, PWM_CH_A))
	{
		PWM_IntHEndClr(PWM1, PWM_CH_A);
		
		GPIO_InvBit(GPIOA, PIN7);
	}
}
