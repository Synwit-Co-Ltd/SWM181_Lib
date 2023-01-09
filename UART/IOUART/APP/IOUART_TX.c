/****************************************************************************************************************************************** 
* 文件名称: IOUART_TX.c
* 功能说明:	GPIO引脚模拟UART发送功能
* 注意事项: 波特率9600
* 版本日期: 
* 升级记录: 
*******************************************************************************************************************************************/
#include "SWM181.h"

#include "IOUART_TX.h"

#define IOUART_STOP_1		0
#define IOUART_STOP_2		1
#define IOUART_STOP			IOUART_STOP_2

#define IOUART_PARITY_NONE	0
#define IOUART_PARITY_EVEN	1
#define IOUART_PARITY_ODD	2
#define IOUART_PARITY		IOUART_PARITY_EVEN


static char *TxBuff = 0;
static uint32_t ChrCnt = 0;
static volatile uint32_t ChrIdx = 0;
static volatile uint32_t BitIdx = 0;

static volatile uint32_t TxBusy = 0;

/****************************************************************************************************************************************** 
* 函数名称:	IOUART_TX_Init()
* 功能说明: IOUART_TX初始化
* 输    入: 无
* 输    出: 无
* 注意事项: 波特率9600
******************************************************************************************************************************************/
void IOUART_TX_Init(void)
{	
	GPIO_Init(GPIOA, PIN4, 1, 0, 0, 0);			//GPIOA.4初始化为输出
#define UART_TX_LOW()	GPIO_ClrBit(GPIOA, PIN4)
#define UART_TX_HIGH()	GPIO_SetBit(GPIOA, PIN4)
	UART_TX_HIGH();								//UART空闲时输出高电平
	
	TIMR_Init(TIMR2, TIMR_MODE_TIMER, SystemCoreClock/9600, 1);
	
	IRQ_Connect(IRQ0_15_TIMR2, IRQ4_IRQ, 1);	//定时器2中断链接到IRQ4中断线，高优先级
}

/****************************************************************************************************************************************** 
* 函数名称:	IOUART_TX_SendChars()
* 功能说明: IOUART发送指定个数字符
* 输    入: char buff[]		要发送的字符
*			uint32_t size	要发送的字符的个数
* 输    出: 无
* 注意事项: 只能在 IOUART_TX_IsBusy() == 0 时发送，否则之前发送的内容可能发送不完全
******************************************************************************************************************************************/
void IOUART_TX_SendChars(char buff[], uint32_t size)
{
	TxBuff = buff;
	ChrCnt = size;
	
	ChrIdx = 0;
	BitIdx = 0;
	TxBusy = 1;
	TIMR_Start(TIMR2);
}

/****************************************************************************************************************************************** 
* 函数名称:	IOUART_TX_IsBusy()
* 功能说明: IOUART否正在发送
* 输    入: 无					
* 输    出: uint32_t		1 IOUART正忙于发送    0 IOUART当前空闲，可以发起新的发送
* 注意事项: 无
******************************************************************************************************************************************/
uint32_t IOUART_TX_IsBusy(void)
{
	return TxBusy;
}

/****************************************************************************************************************************************** 
* 函数名称:	IRQ4_Handler()
* 功能说明: TIMR2中断服务函数
* 输    入: 无
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
void IRQ4_Handler(void)
{
	const uint8_t evenOf1[16] = {1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1};	//整数0--15对应是否有偶数个'1'
	
	if(TIMR_INTStat(TIMR2))
	{
		TIMR_INTClr(TIMR2);
		
		if(TxBusy == 1)
		{
			if(BitIdx == 0)
			{
				UART_TX_LOW();				//起始位
			}
			else if(BitIdx < 9)
			{
				if(TxBuff[ChrIdx] & (0x01 << (BitIdx-1)))
					UART_TX_HIGH();
				else
					UART_TX_LOW();
			}
#if(IOUART_PARITY == IOUART_PARITY_NONE)
	#if(IOUART_STOP == IOUART_STOP_1)
			else if(BitIdx == 9)
	#else //IOUART_STOP == IOUART_STOP_2
			else if(BitIdx == 9)
			{
				UART_TX_HIGH();				//停止位
			}
			else if(BitIdx == 10)
	#endif
#else
			else if(BitIdx == 9)
			{
	#if(IOUART_PARITY == IOUART_PARITY_EVEN)
				if(((evenOf1[TxBuff[ChrIdx] & 0xF] == 1) && (evenOf1[(TxBuff[ChrIdx] >> 4) & 0xF] == 1)) || 
				   ((evenOf1[TxBuff[ChrIdx] & 0xF] == 0) && (evenOf1[(TxBuff[ChrIdx] >> 4) & 0xF] == 0)))	// 偶数个1
	#else //IOUART_PARITY == IOUART_PARITY_ODD
				if(((evenOf1[TxBuff[ChrIdx] & 0xF] == 0) && (evenOf1[(TxBuff[ChrIdx] >> 4) & 0xF] == 1)) || 
				   ((evenOf1[TxBuff[ChrIdx] & 0xF] == 1) && (evenOf1[(TxBuff[ChrIdx] >> 4) & 0xF] == 0)))	// 奇数个1
	#endif
					UART_TX_LOW();
				else
					UART_TX_HIGH();
			}
	#if(IOUART_STOP == IOUART_STOP_1)
			else if(BitIdx == 10)
	#else //IOUART_STOP == IOUART_STOP_2
			else if(BitIdx == 10)
			{
				UART_TX_HIGH();				//停止位
			}
			else if(BitIdx == 11)
	#endif
#endif
			{
				UART_TX_HIGH();				//停止位
				
				ChrIdx++;
				if(ChrIdx < ChrCnt)
				{
					BitIdx = 0;
					
					goto T3Svr_End;		//跳过“BitIdx++”，否则发不出起始位
				}
				else
				{
					TIMR_Stop(TIMR2);
					
					TxBusy = 0;
				}
			}
			
			BitIdx++;
		}
		
T3Svr_End:
		;
	}
}
