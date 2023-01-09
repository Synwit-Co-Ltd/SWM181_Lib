#include "SWM181.h"


int main(void)
{
	PWM_InitStructure  PWM_initStruct;
	
	SystemInit();
	
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
	PWM_initStruct.HEndAIEn = 0;
	PWM_initStruct.NCycleAIEn = 0;
	PWM_initStruct.HEndBIEn = 0;
	PWM_initStruct.NCycleBIEn = 0;
	
	PWM_Init(PWM1, &PWM_initStruct);
	
	PORT_Init(PORTA, PIN4, FUNMUX_PWM1A_OUT, 0);
	PORT_Init(PORTA, PIN5, FUNMUX_PWM1B_OUT, 0);
	
	PWM_Start(PWM1, 1, 1);
	
	/* 刹车功能测试 */
#if 0
	PORT_Init(PORTA, PIN6, FUNMUX_PWM_BREAK, 1);
	
	PWMG->HALT = (1 << PWMG_HALT_EN_Pos)      |
				 (1 << PWMG_HALT_PWM1_Pos)    |
				 (1 << PWMG_HALT_STOPCNT_Pos) |
				 (0 << PWMG_HALT_VALIDI_Pos)  |
				 (0 << PWMG_HALT_VALIDO_Pos);	// PWM_BREAK电平位低时，PWM1输出变成低
#endif

	while(1==1)
	{
	}
}
