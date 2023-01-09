#include "SWM181.h"


int main(void)
{	
	SystemInit();
	
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);			//输出，接LED
	
	GPIO_Init(GPIOA, PIN4, 0, 1, 0, 0);			//输入，上拉使能，接KEY
	
	EXTI_Init(GPIOA, PIN4, EXTI_FALL_EDGE);		//下降沿触发中断
	
	IRQ_Connect(IRQ0_15_GPIOA4, IRQ4_IRQ, 2);
	
	EXTI_Open(GPIOA, PIN4);
	
	while(1==1)
	{
	}
}

void IRQ4_Handler(void)
{
	EXTI_Clear(GPIOA, PIN4);
	
	GPIO_InvBit(GPIOA, PIN5);
}
