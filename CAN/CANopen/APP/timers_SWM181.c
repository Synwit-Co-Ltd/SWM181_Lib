#include "timer_canfestival.h"

#include "SWM181.h"

#define CPU_HZ  48

static UNS32 last_time_set = TIMEVAL_MAX;	// Store the last timer value to calculate the elapsed time

void setTimer(TIMEVAL value)
{
    TIMR_Stop(TIMR3);
    TIMR_SetPeriod(TIMR3, TIMR_GetCurValue(TIMR3) + value * CPU_HZ);
    TIMR_Start(TIMR3);
}

TIMEVAL getElapsedTime(void)
{
    uint32_t timer = TIMR_GetCurValue(TIMR3);

    if(timer > last_time_set) timer = timer - last_time_set;
    else                      timer = 0xFFFFFF + timer - last_time_set;

    return timer/CPU_HZ;
}

void TIM3_start(void)
{
    TIMR_Init(TIMR3, TIMR_MODE_TIMER, 0xC00000, 1);
	
	IRQ_Connect(IRQ0_15_TIMR3, IRQ7_IRQ, 2);		//定时器3中断链接到IRQ7中断线，低优先级

    TIMR_Start(TIMR3);
}

void IRQ7_Handler(void)
{
    TIMR_INTClr(TIMR3);

    last_time_set = TIMR_GetCurValue(TIMR3);

    TimeDispatch();
}

void StartTimerLoop(TimerCallback_t init_callback)
{
	SetAlarm(NULL, 0, init_callback, 0, 0);

    TIM3_start();
}
