/****************************************************************************************************************************************** 
* 文件名称:	SPIUART.c
* 功能说明:	SPI模拟UART，半双工，支持奇偶校验
* 技术支持:	
* 注意事项: 180的SPI使用环形移位寄存器实现，若SI第一位为0，则SO发送最后一位时，时钟采样沿之后很快SO电平会变成0
*			解决方法：半双工通信，发送时SI引脚肯定是高，无问题；接收时将SO引脚切成GPIO输出高电平，SO上的低不会输出到引脚上
* 版本日期:
* 升级记录: 2017/11/21 对方连续发送时检测不到非第一字节的起始位下降沿
*					   解决方法：SPI 只接收10位，不再接收和检测第11位（停止位）
*
******************************************************************************************************************************************/
#include "SWM181.h"

#include "SPIUART.h"


#define IOUART_PARITY_EVEN	1
#define IOUART_PARITY_ODD	2
#define IOUART_PARITY		IOUART_PARITY_EVEN


volatile uint16_t SPIUART_SendComplete = 0;


#define SPI_TX_LEN	64
static uint16_t SPI_TX_Buff[SPI_TX_LEN] = {0};
static uint16_t SPI_TX_Count;
static uint16_t SPI_TX_Index;

#define SPI_RX_LEN	64
static uint16_t SPI_RX_Buff[SPI_RX_LEN] = {0};
static uint16_t SPI_RX_Index;


/****************************************************************************************************************************************** 
* 函数名称:	SPIUART_Init()
* 功能说明:	SPI模拟UART初始化
* 输    入: uint16_t baudrate		串口波特率
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
void SPIUART_Init(uint16_t baudrate)
{
	SPI_InitStructure SPI_initStruct;
	PWM_InitStructure PWM_initStruct;
	
	GPIO_Init(GPIOA, PIN7, 1, 0, 0, 0);				//模拟SPI主机的nCS引脚，需要接SPI从机的nCS引脚
	GPIO_SetBit(GPIOA, PIN7);
	
	PORT_Init(PORTA, PIN8,  PORTA_PIN8_SPI0_SSEL,  1);
	
	PORT_Init(PORTA, PIN9,  PORTA_PIN9_SPI0_MISO,  0);
	GPIO_Init(GPIOA, PIN9,  1, 0, 0, 0);			//当引脚切成GPIO时方向为输出、输出值为高电平
	GPIO_SetBit(GPIOA, PIN9);
#define PA9_AsGPIO()  PORTG->PORTA_SEL2 &= ~(0x03 << ((PIN9-8)*2));
#define PA9_AsMISO()  PORTG->PORTA_SEL2 |= PORTA_PIN9_SPI0_MISO << ((PIN9-8)*2);
	
	PORT_Init(PORTA, PIN10, PORTA_PIN10_SPI0_MOSI, 1);
	GPIO_Init(GPIOA, PIN10, 0, 1, 0, 0);			//当引脚切成GPIO时方向为输入、使能上拉，用于中断检测起始位的下降沿
#define PA10_AsGPIO() PORTG->PORTA_SEL2 &= ~(0x03 << ((PIN10-8)*2));
#define PA10_AsMOSI() PORTG->PORTA_SEL2 |= PORTA_PIN10_SPI0_MOSI << ((PIN10-8)*2);
	
	EXTI_Init(GPIOA, PIN10, EXTI_FALL_EDGE);			//下降沿触发中断
	IRQ_Connect(IRQ0_15_GPIOA10, IRQ9_IRQ, 1);			//高优先级
	
	PORT_Init(PORTA, PIN11, PORTA_PIN11_SPI0_SCLK, 1);
	
	
	SPI_initStruct.clkDiv = SPI_CLKDIV_512;
	SPI_initStruct.FrameFormat = SPI_FORMAT_SPI;
	SPI_initStruct.SampleEdge = SPI_SECOND_EDGE;
	SPI_initStruct.IdleLevel = SPI_HIGH_LEVEL;
	SPI_initStruct.WordSize = 11;
	SPI_initStruct.Master = 0;
	SPI_initStruct.RXHFullIEn = 0;
	SPI_initStruct.TXEmptyIEn = 0;
	SPI_Init(SPI0, &SPI_initStruct);
	
	IRQ_Connect(IRQ0_15_SPI0, IRQ8_IRQ, 1);
	
	SPI_Open(SPI0);
	
	
	PWM_initStruct.mode = PWM_MODE_INDEP;			//A路和B路独立输出
	PWM_initStruct.clk_div = PWM_CLKDIV_1;
	PWM_initStruct.cycleA = SystemCoreClock/baudrate;		
	PWM_initStruct.hdutyA = SystemCoreClock/baudrate/2;
	PWM_initStruct.initLevelA = 0;
	PWM_initStruct.HEndAIEn = 0;
	PWM_initStruct.NCycleAIEn = 0;
	PWM_initStruct.HEndBIEn = 0;
	PWM_initStruct.NCycleBIEn = 0;
	
	PORT_Init(PORTA, PIN4, FUNMUX_PWM1A_OUT, 0);	//模拟SPI主机的CLK引脚，需连接SPI从机的CLK引脚
	
	PWM_Init(PWM1, &PWM_initStruct);

	
#define SPI_MASTER_CLK_GEN()	__disable_irq(); SPI_Open(SPI0); GPIO_ClrBit(GPIOA, PIN7); PWM_Start(PWM1, 1, 0); __enable_irq()
#define SPI_MASTER_CLK_STOP()	__disable_irq(); PWM_Stop(PWM1, 1, 1); GPIO_SetBit(GPIOA, PIN7); SPI_Close(SPI0); __enable_irq()
}


static void _SPIUART_Send(uint16_t cnt);
/****************************************************************************************************************************************** 
* 函数名称:	SPIUART_Send()
* 功能说明:	SPI模拟UART数据发送
* 输    入: uint8_t buff[]		要发送的数据，8位
*			uint16_t cnt		要发送数据的个数
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
void SPIUART_Send(uint8_t buff[], uint16_t cnt)
{
	EXTI_Close(GPIOA, PIN10);
	
	PA10_AsMOSI();
	
	SPIUART_SendComplete = 0;
	
	UART2SPI_encode(buff, cnt);
	
	SPI0->CTRL &= ~SPI_CTRL_DSS_Msk;
	SPI0->CTRL |= (10 << SPI_CTRL_DSS_Pos);
	
	PA9_AsMISO();	//发送前切换
	
	_SPIUART_Send(cnt);
}

static void _SPIUART_Send(uint16_t cnt)
{	
	SPI_TX_Count = cnt;
	SPI_TX_Index = 0;
	
	SPI_Write(SPI0, SPI_TX_Buff[SPI_TX_Index++]);
	
	SPI_MASTER_CLK_GEN();
	
	SPI_INTEn(SPI0, SPI_IT_TX_EMPTY);
}

/****************************************************************************************************************************************** 
* 函数名称:	SPIUART_Recv()
* 功能说明:	读取接收到的数据，返回数据个数，数据的bit15是帧错误标志、bit14是校验错误标志
* 输    入: uint16_t buff[]		读取到的数据存入buff
*			uint16_t min		若数据个数少于 min ，则不读取数据，直接返回0
* 输    出: uint16_t 			数据个数
* 注意事项: buff数组的大小必须不小于SPI_RX_LEN
******************************************************************************************************************************************/
uint16_t SPIUART_Recv(uint16_t buff[], uint16_t min)
{
	uint16_t len;
	
	if(SPI_RX_Index < min) return 0;
	
	SPI_INTDis(SPI0, SPI_IT_TX_EMPTY);
	memcpy(buff, SPI_RX_Buff, (len = SPI_RX_Index) * 2);
	SPI_RX_Index = 0;
	SPI_INTEn(SPI0, SPI_IT_TX_EMPTY);
	
	SPI2UART_decode(buff, len);
	
	return len;
}

void SPIUART_RecvPrepare(void)
{
	PA9_AsGPIO();	//接收时SO输出的数据不发送到引脚上
	
	PA10_AsGPIO();
	
	SPI_RX_Index = 0;
	
	SPI0->CTRL &= ~SPI_CTRL_DSS_Msk;
	SPI0->CTRL |= (9 << SPI_CTRL_DSS_Pos);
	
	EXTI_Clear(GPIOA, PIN10);
	EXTI_Open(GPIOA, PIN10);
}

void IRQ8_Handler(void)
{		
	if(SPI_TX_Index == SPI_TX_Count + 1)
	{
		SPI_MASTER_CLK_STOP();
		
		PA9_AsGPIO();	//发送完时SO输出可能是低，切成GPIO输出高
		
		PA10_AsGPIO();	//准备响应下一个EXTI中断
		
		SPIUART_SendComplete = 1;
		
		SPI_INTDis(SPI0, SPI_IT_TX_EMPTY);
	}
	else if(SPI_TX_Index == SPI_TX_Count)
	{
		SPI_Write(SPI0, 0x7FF);
		
		SPI_TX_Index++;
	}
	else
	{
		while(SPI_IsTXFull(SPI0) == 0)
		{
			if(SPI_TX_Index < SPI_TX_Count)
			{
				SPI_Write(SPI0, SPI_TX_Buff[SPI_TX_Index++]);
			}
			else
			{
				break;
			}
		}
	}
	
	while(SPI_IsRXEmpty(SPI0) == 0)
	{
		volatile uint16_t tmp;
		
		if(SPI_RX_Index < SPI_RX_LEN)
			SPI_RX_Buff[SPI_RX_Index++] = SPI0->DATA;
		else
			tmp = SPI0->DATA;
	}
	
	SPI0->IF = SPI_IF_TFE_Msk;	//清除中断标志，必须在填充TX FIFO后清中断标志
}

void IRQ9_Handler(void)
{	
	EXTI_Clear(GPIOA, PIN10);
	
	PA10_AsMOSI();
	
	SPI_TX_Buff[0] = 0x7FF;
	_SPIUART_Send(1);
}


/********************************************************************************************************
                                            内部函数
********************************************************************************************************/

const uint8_t InvCode[16] = {0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 				//UART先发送LSB、SPI先发送MSB，需要反转位序
							 0xE, 0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF};		//整数0--15对应的位序反转数值
const uint8_t EvenOf1[16] = {1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1};	//整数0--15对应是否有偶数个'1' 

void UART2SPI_encode(uint8_t buff[], uint16_t cnt)
{
	uint16_t i, code;
	
	for(i = 0; i < cnt; i++)
	{
		code = (0 << 10) | 
		       (((InvCode[buff[i] & 0xF] << 4) | InvCode[(buff[i] >> 4) & 0xF]) << 2) |
			   ((
	#if IOUART_PARITY == IOUART_PARITY_EVEN
				 (((EvenOf1[buff[i] & 0xF] == 1) && (EvenOf1[(buff[i] >> 4) & 0xF] == 1)) || 
				  ((EvenOf1[buff[i] & 0xF] == 0) && (EvenOf1[(buff[i] >> 4) & 0xF] == 0)))	/* 	偶数个1 */
	#else //IOUART_PARITY == IOUART_PARITY_ODD
				 (((EvenOf1[buff[i] & 0xF] == 0) && (EvenOf1[(buff[i] >> 4) & 0xF] == 1)) || 
				  ((EvenOf1[buff[i] & 0xF] == 1) && (EvenOf1[(buff[i] >> 4) & 0xF] == 0)))	/* 	奇数个1 */
	#endif
				 ? 0 : 1) << 1) |
		       (1 << 0);					//起始位0、数据位、校验位、停止位1
		
		SPI_TX_Buff[i] = code;
	}
}

void SPI2UART_decode(uint16_t buff[], uint16_t cnt)
{
	uint16_t i, code;
		
	for(i = 0; i < cnt; i++)
	{
		code = (InvCode[(buff[i] >> 1) & 0xF] << 4) | InvCode[(buff[i] >> 5) & 0xF];
	
	#if IOUART_PARITY == IOUART_PARITY_EVEN
		if(((EvenOf1[(buff[i] >> 1) & 0xF] == 1) && (EvenOf1[(buff[i] >> 5) & 0xF] == 1)) || 
		   ((EvenOf1[(buff[i] >> 1) & 0xF] == 0) && (EvenOf1[(buff[i] >> 5) & 0xF] == 0)))	/* 	偶数个1 */
	#else //IOUART_PARITY == IOUART_PARITY_ODD
		if(((EvenOf1[(buff[i] >> 1) & 0xF] == 0) && (EvenOf1[(buff[i] >> 5) & 0xF] == 1)) || 
		   ((EvenOf1[(buff[i] >> 1) & 0xF] == 1) && (EvenOf1[(buff[i] >> 5) & 0xF] == 0)))	/* 	奇数个1 */
	#endif
		{
			if(((buff[i] >> 0) & 1) != 0) code |= SPIUART_PARITY_ERR_MASK;	//校验错误
		}
		
//		if(((buff[i] >> 0) & 1) != 1) 	  code |= SPIUART_FRAME_ERR_MASK;	//帧错误，即停止位不为1
		
		buff[i] = code;
	}
}
