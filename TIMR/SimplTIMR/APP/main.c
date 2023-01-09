#include "SWM181.h"


int main(void)
{	
	SystemInit();
	
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);			//输出，接LED
	
	TIMR_Init(TIMR0, TIMR_MODE_TIMER, SystemCoreClock/2, 1);	//每0.5秒钟触发一次中断
	
	IRQ_Connect(IRQ0_15_TIMR0, IRQ5_IRQ, 2);		//定时器0中断链接到IRQ5中断线，低优先级
	
	TIMR_Start(TIMR0);
	
	while(1==1)
	{
	}
}

void IRQ5_Handler(void)
{
	TIMR_INTClr(TIMR0);
	
	GPIO_InvBit(GPIOA, PIN5);	//反转LED亮灭状态
}

