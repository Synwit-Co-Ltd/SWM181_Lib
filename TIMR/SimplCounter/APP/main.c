#include "SWM181.h"


int main(void)
{	
	SystemInit();
	
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);
	
	PORT_Init(PORTA, PIN7, FUNMUX_TIMR2_IN, 1);
	
	TIMR_Init(TIMR2, TIMR_MODE_COUNTER, 3, 1);	//每计3个边沿进入中断
	
	IRQ_Connect(IRQ0_15_TIMR2, IRQ5_IRQ, 2);	//定时器2中断链接到IRQ5中断线，低优先级
	
	TIMR_Start(TIMR2);
	
	while(1==1)
	{
	}
}

void IRQ5_Handler(void)
{
	TIMR_Stop(TIMR2);			//必须用这三条语句清除中断，并且三条语句的顺序不能调换，否则会多次进入中断处理函数
	TIMR_INTClr(TIMR2);
	TIMR_Start(TIMR2);
	
	GPIO_InvBit(GPIOA, PIN5);
}
