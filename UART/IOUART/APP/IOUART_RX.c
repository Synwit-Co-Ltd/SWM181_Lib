/****************************************************************************************************************************************** 
* 文件名称:	IOUART_RX.c
* 功能说明:	GPIO引脚模拟UART接收功能
* 注意事项: 波特率9600
* 版本日期: 
* 升级记录: 
*******************************************************************************************************************************************/ 
#include "SWM181.h"

#include "IOUART_RX.h"


#define IOUART_STOP_1		0
#define IOUART_STOP_2		1
#define IOUART_STOP			IOUART_STOP_2

#define IOUART_PARITY_NONE	0
#define IOUART_PARITY_EVEN	1
#define IOUART_PARITY_ODD	2
#define IOUART_PARITY		IOUART_PARITY_EVEN


static char *RxBuff = 0;
static uint32_t ChrCnt = 0;
static volatile uint32_t ChrIdx = 0;
static volatile uint32_t BitIdx = 0;

static volatile uint32_t RxStat = IOUART_RX_IDLE;

static uint32_t TIM_BAUD_9600 = 0;			//波特率9600对应定时时长
static uint32_t TIM_BAUD_9600_X_1_5 = 0;	//波特率9600对应定时时长的1.5倍
											//在起始位下降沿之后的1.5个字符周期后采样，这样可以采到数据位的中间位置

static volatile uint32_t Timeout_Cnt = 0;	//超时计数器

/****************************************************************************************************************************************** 
* 函数名称:	IOUART_RX_Init()
* 功能说明: IOUART_RX初始化
* 输    入: 无
* 输    出: 无
* 注意事项: 波特率9600
******************************************************************************************************************************************/
void IOUART_RX_Init(void)
{
	GPIO_Init(GPIOA, PIN5, 0, 1, 0, 0);			//GPIOA.5初始化为输入，上拉
#define UART_RX_Value()		GPIO_GetBit(GPIOA, PIN5)
	
	GPIO_Init(GPIOA, PIN6, 1, 0, 0, 0);			//Sampling Point Indicate，调试用
#define UART_RXI_INV()	GPIO_InvBit(GPIOA, PIN6)
	
	TIM_BAUD_9600       = SystemCoreClock/9600;
	TIM_BAUD_9600_X_1_5 = SystemCoreClock/9600*1.5 * 0.85;
	
	TIMR_Init(TIMR3, TIMR_MODE_TIMER, SystemCoreClock/9600, 1);
	
	IRQ_Connect(IRQ0_15_TIMR3, IRQ9_IRQ, 1);		//定时器3中断链接到IRQ9中断线，高优先级
	
	EXTI_Init(GPIOA, PIN5, EXTI_FALL_EDGE);		//下降沿触发中断
	
	IRQ_Connect(IRQ0_15_GPIOA5, IRQ5_IRQ, 1);	//A5引脚外部中断链接到IRQ5中断线，高优先级
	
	EXTI_Open(GPIOA, PIN5);
}

/****************************************************************************************************************************************** 
* 函数名称:	IOUART_RX_RecvChars()
* 功能说明: IOUART接收指定个数字符
* 输    入: char buff[]		接收到的字符存此处
*			uint32_t size	要接收的字符的个数
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
void IOUART_RX_RecvChars(char buff[], uint32_t size)
{
	RxBuff = buff;
	ChrCnt = size;
	
	ChrIdx = 0;
	RxStat = IOUART_RX_IDLE;
	
	EXTI_Clear(GPIOA, PIN5);
	EXTI_Open(GPIOA, PIN5);
}


/****************************************************************************************************************************************** 
* 函数名称:	IOUART_RX_State()
* 功能说明: IOUART接收模块当前状态
* 输    入: 无
* 输    出: uint32_t		IOUART_RX_IDLE、IOUART_RX_BUSY、IOUART_RX_SUCCESS、UOUART_RX_TIMEOUT、IOUART_RX_FRAMERR
* 注意事项: 无
******************************************************************************************************************************************/
uint32_t IOUART_RX_State(void)
{
	return RxStat;
}

/****************************************************************************************************************************************** 
* 函数名称:	IOUART_RX_Count()
* 功能说明: IOUART接收模块接收到的字符数
* 输    入: 无
* 输    出: uint32_t		接收到的字符个数
* 注意事项: 无
******************************************************************************************************************************************/
uint32_t IOUART_RX_Count(void)
{
	return ChrIdx;
}

/****************************************************************************************************************************************** 
* 函数名称:	IOUART_RX_Clear()
* 功能说明: IOUART接收模块状态清除
* 输    入: 无
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
void IOUART_RX_Clear(void)
{
	ChrIdx = 0;
	
	RxStat = IOUART_RX_IDLE;
}

/****************************************************************************************************************************************** 
* 函数名称:	IRQ5_Handler()
* 功能说明: IOUART接收模块引脚下降沿中断服务函数，检测到下降沿后关闭下降沿中断、启动定时器
* 输    入: 无
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
void IRQ5_Handler(void)
{
	EXTI_Clear(GPIOA, PIN5);
	
	if(RxStat == IOUART_RX_IDLE)
	{
		BitIdx = 0;
		RxStat = IOUART_RX_BUSY;
		
		TIMR_Stop(TIMR3);
		TIMR_SetPeriod(TIMR3, TIM_BAUD_9600_X_1_5);
		TIMR_Start(TIMR3);
	}
}

/****************************************************************************************************************************************** 
* 函数名称:	IRQ9_Handler()
* 功能说明: TIMR3中断服务程序，完成IOUART接收模块的位接收
* 输    入: 无
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
void IRQ9_Handler(void)
{
	static char ChrShf;		//接收移位寄存器
	const uint8_t evenOf1[16] = {1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1};	//整数0--15对应是否有偶数个'1'
	
	if(TIMR_INTStat(TIMR3))
	{
		TIMR_INTClr(TIMR3);
		
		if(RxStat == IOUART_RX_BUSY)
		{	
			UART_RXI_INV();
			
			if(BitIdx == 0)
			{
				TIMR_Stop(TIMR3);
				TIMR_SetPeriod(TIMR3, TIM_BAUD_9600);
				TIMR_Start(TIMR3);
			}
			
			if(BitIdx < 8)
			{
				if(UART_RX_Value()) ChrShf |=  (1 << BitIdx);
				else				ChrShf &= ~(1 << BitIdx);
			}
#if(IOUART_PARITY == IOUART_PARITY_NONE)
	#if(IOUART_STOP == IOUART_STOP_1)
			else if(BitIdx == 8)
	#else //IOUART_STOP == IOUART_STOP_2
			else if(BitIdx == 8)
			{
				if(UART_RX_Value() == 0)
				{
					RxStat = IOUART_RX_FRAMERR;
					
					EXTI_Close(GPIOA, PIN5);
				}
			}
			else if(BitIdx == 9)
	#endif
#else
			else if(BitIdx == 8)
			{
	#if(IOUART_PARITY == IOUART_PARITY_EVEN)
				if(((evenOf1[ChrShf & 0xF] == 1) && (evenOf1[(ChrShf >> 4) & 0xF] == 1)) || 
				   ((evenOf1[ChrShf & 0xF] == 0) && (evenOf1[(ChrShf >> 4) & 0xF] == 0)))	// 偶数个1
	#else //IOUART_PARITY == IOUART_PARITY_ODD
				if(((evenOf1[ChrShf & 0xF] == 0) && (evenOf1[(ChrShf >> 4) & 0xF] == 1)) || 
				   ((evenOf1[ChrShf & 0xF] == 1) && (evenOf1[(ChrShf >> 4) & 0xF] == 0)))	// 奇数个1
	#endif
				{
					if(UART_RX_Value() == 1)
					{
						RxStat = IOUART_RX_PARITYERR;
						
						EXTI_Close(GPIOA, PIN5);
					}
				}
				else
				{
					if(UART_RX_Value() == 0)
					{
						RxStat = IOUART_RX_PARITYERR;
						
						EXTI_Close(GPIOA, PIN5);
					}
				}
			}
	#if(IOUART_STOP == IOUART_STOP_1)
			else if(BitIdx == 9)
	#else //IOUART_STOP == IOUART_STOP_2
			else if(BitIdx == 9)
			{
				if(UART_RX_Value() == 0)
				{
					RxStat = IOUART_RX_FRAMERR;
					
					EXTI_Close(GPIOA, PIN5);
				}
			}
			else if(BitIdx == 10)
	#endif
#endif
			{
				RxStat = IOUART_RX_IDLE;
				
				RxBuff[ChrIdx] = ChrShf;
				
				if(UART_RX_Value() == 1)
				{
					ChrIdx++;
					
					if(ChrIdx == ChrCnt)
					{
						RxStat = IOUART_RX_BUFFULL;
						
						EXTI_Close(GPIOA, PIN5);
					}
				}
				else
				{
					RxStat = IOUART_RX_FRAMERR;
					
					EXTI_Close(GPIOA, PIN5);
				}
			}
			
			BitIdx++;
			
			UART_RXI_INV();
			
			Timeout_Cnt = 0;
		}
		else if((RxStat == IOUART_RX_IDLE) && (ChrIdx != 0))
		{
			Timeout_Cnt++;
			if(Timeout_Cnt == 20)	//2个字符时间
			{
				RxStat = IOUART_RX_TIMEOUT;
				
				EXTI_Close(GPIOA, PIN5);
				
				TIMR_Stop(TIMR3);
				
				Timeout_Cnt = 0;
			}
		}
	}
}
