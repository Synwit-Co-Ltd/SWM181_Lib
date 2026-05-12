/****************************************************************************************************************************************** 
* 文件名称:	SWM181_sleep.c
* 功能说明:	SWM181单片机的进入休眠模式功能驱动库
* 技术支持:	http://www.synwit.com.cn/e/tool/gbook/?bid=1
* 注意事项:
* 版本日期: V1.0.0		2016年1月30日
* 升级记录: 
*******************************************************************************************************************************************
* @attention
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS WITH CODING INFORMATION 
* REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME. AS A RESULT, SYNWIT SHALL NOT BE HELD LIABLE 
* FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT 
* OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN IN CONN-
* -ECTION WITH THEIR PRODUCTS.
*
* COPYRIGHT 2012 Synwit Technology  
*******************************************************************************************************************************************/
#include "SWM181.h"
#include "SWM181_sleep.h"


/* 注意：EnterSleepMode() 和 EnterStopMode() 必须在RAM中执行，Keil下实现方法有：
   方法一、Scatter file
   方法二、SWM181_sleep.c 上右键 =》Options for File "SWM181_sleep.c" =》Properties =》Memory Assignment =》Code/Conts 选择 IRAM1
*/


#if defined ( __ICCARM__ )
__ramfunc
#endif
void EnterSleepMode(void)
{
	while((0x1<<16)&FLASH->STAT);
	SYS->SLEEP |= (1 << SYS_SLEEP_SLEEP_Pos);
}


#if defined ( __ICCARM__ )
__ramfunc
#endif
void EnterStopMode(void)
{
	while((0x1<<16)&FLASH->STAT);
	FLASH->CR = 0x1<<12;
	FLASH->START = 0x1;
	while(0x1&FLASH->START);
	SYS->SLEEP |= (1 << SYS_SLEEP_STOP_Pos);
}
