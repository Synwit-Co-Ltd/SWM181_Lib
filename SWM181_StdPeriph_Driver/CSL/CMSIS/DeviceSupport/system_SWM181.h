#ifndef __SYSTEM_SWM181_H__
#define __SYSTEM_SWM181_H__

#ifdef __cplusplus
 extern "C" {
#endif

	 
extern uint32_t SystemCoreClock;		// System Clock Frequency (Core Clock)
extern uint32_t CyclesPerUs;			// Cycles per micro second


extern void SystemInit(void);

extern void SystemCoreClockUpdate (void);


extern void switchToRC24MHz(void);
extern void switchToRC6MHz(void);
extern void switchToRC48MHz(void);
extern void switchToRC12MHz(void);
extern void switchToRC32KHz(void);
extern void switchToXTAL(void);


#ifdef __cplusplus
}
#endif

#endif //__SYSTEM_SWM181_H__
