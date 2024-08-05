#include "SWM181.h"


/* 错误处理演示步骤说明：
	一、将 CAN 连接收发器但收发器不连接其他 CAN 节点，执行程序，通过串口助手会发现 CAN->TXERR 会从逐渐递增到 128，但此后不再递增，具体原因参考 CAN spec 上如下说明：
	
		3. When a TRANSMITTER sends an ERROR FLAG the TRANSMIT ERROR COUNT is increased by 8.

		Exception 1:
		If the TRANSMITTER is 'error passive' and detects an ACKNOWLEDGMENT ERROR because of not detecting
		a 'dominant' ACK and does not detect a 'dominant' bit while sending its PASSIVE ERROR FLAG.
	
	二、此时 ACK ERROR 不再递增 CAN->TXERR，将 CAN 与收发器断开，触发 BIT ERROR，通过串口助手可发现 CAN->TXERR 继续递增到 256，
		导致 CAN->SR.BUSOFF 和 CAN->CR.RST 置位，然后 CAN->TXERR 值变成 127，此时 CAN 节点处于 Bus Off 状态，CAN 控制器处于复位状态。
	
	三、CAN->CR.RST 被软件清零后，将 CAN 与收发器相连，CAN 节点退出 Bus Off 状态
*/


void SerialInit(void);

uint32_t sameBits(uint32_t std_id[], uint32_t std_n, uint32_t ext_id[], uint32_t ext_n);

int main(void)
{
	uint32_t i;
	CAN_InitStructure CAN_initStruct;
	uint8_t tx_data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
	
	SystemInit();
	
	SerialInit();
	
   	PORT_Init(PORTA, PIN4, FUNMUX_CAN_RX, 1);	//GPIOA.4配置为CAN输入引脚
	PORT_Init(PORTA, PIN5, FUNMUX_CAN_TX, 0);	//GPIOA.5配置为CAN输出引脚
	
	CAN_initStruct.Mode = CAN_MODE_NORMAL;
	CAN_initStruct.CAN_BS1 = CAN_BS1_4tq;
	CAN_initStruct.CAN_BS2 = CAN_BS2_3tq;
	CAN_initStruct.CAN_SJW = CAN_SJW_3tq;
	CAN_initStruct.Baudrate = 100000;
	CAN_initStruct.FilterMode = CAN_FILTER_32b;
	switch(0)
	{
	case 0:		// 接收ID为任意值的帧
		CAN_initStruct.FilterMask32b  = 0xFFFFFFFF;				// 0 must match    1 don't care
		CAN_initStruct.FilterCheck32b = 0xFFFFFFFF;
		break;
	
	case 1:		// 接收ID为0x122的标准帧
		CAN_initStruct.FilterMask32b = ~(0x7FFu << 21);
		CAN_initStruct.FilterCheck32b = (0x122  << 21);			// 标准帧ID在第21位到31位，详见手册
		break;
	
	case 2:		// 接收ID为0x122的扩展帧
		CAN_initStruct.FilterMask32b = ~(0x1FFFFFFFu << 3);
		CAN_initStruct.FilterCheck32b = (0x00000122  << 3);		// 扩展帧ID在第 3位到31位，详见手册
		break;
	
	case 3:		// 接收ID为0x12X的标准帧，X表示ID的低4位是什么值无所谓
		CAN_initStruct.FilterMask32b = ~(0x7F0u << 21);
		CAN_initStruct.FilterCheck32b = (0x122  << 21);
		break;
	
	case 4:		// 接收ID为0x122和0x101的标准帧
		CAN_initStruct.FilterMode = CAN_FILTER_16b;
		CAN_initStruct.FilterMask16b1 = (uint16_t)~(0x7FFu << 5);
		CAN_initStruct.FilterCheck16b1 = (0x122  << 5);
		CAN_initStruct.FilterMask16b2 = (uint16_t)~(0x7FFu << 5);
		CAN_initStruct.FilterCheck16b2 = (0x101  << 5);
		break;
	
	case 5:		// 即接收ID为0x122、0x235、0x450的标准帧，也接收ID为0x101, 0x235, 0x1780的扩展帧
		{		// 并不是只接收ID为这6个值的帧，而是接收尽量最少的帧、且能保证ID为要求值的帧都能接收到
		uint32_t stdID[] = {0x122, 0x235, 0x450};
		uint32_t extID[] = {0x101, 0x235, 0x1780};
		
		CAN_initStruct.FilterMask32b = ~(sameBits(stdID, 3, extID, 3) << 3);
		CAN_initStruct.FilterCheck32b = (extID[0]  << 3);
		// 或者：
// 		CAN_initStruct.FilterCheck32b = (stdID[0]  << 21);
		}
		break;
	}
	CAN_initStruct.RXNotEmptyIEn = 1;
	CAN_initStruct.RXOverflowIEn = 0;
	CAN_initStruct.ArbitrLostIEn = 0;
	CAN_initStruct.ErrPassiveIEn = 1;
	CAN_Init(CAN, &CAN_initStruct);
	
	CAN_INTEn(CAN, CAN_IT_ERR_WARN | CAN_IT_RX_OVERFLOW);
	
	IRQ_Connect(IRQ0_15_CAN, IRQ5_IRQ, 1);
	
	CAN_Open(CAN);
	
	while(1==1)
	{
		while((CAN->SR & CAN_SR_BUSOFF_Msk) == 0)
		{
			CAN_Transmit(CAN, CAN_FRAME_STD, 0x60D, tx_data, 8, 1);
			
			while(CAN_TXComplete(CAN) == 0) __NOP();
			
			printf("\r\nCAN->TXERR: %d\r\n", CAN->TXERR);
			
			for(i = 0; i < SystemCoreClock/16; i++) __NOP();
		}
	}
}

void IRQ5_Handler(void)
{
	uint32_t i;
	uint32_t int_sr = CAN_INTStat(CAN);
	
	if(int_sr & CAN_IF_RXDA_Msk)
	{
		CAN_RXMessage msg;
		
		CAN_Receive(CAN, &msg);
		
		if(msg.size > 0)
		{
			printf("\r\nReceive %s: %08X, ", msg.format == CAN_FRAME_STD ? "STD" : "EXT", msg.id);
			for(i = 0; i < msg.size; i++) printf("%02X, ", msg.data[i]);
		}
		else if(msg.remote == 1)	//远程帧
		{
			printf("\r\nReceive %s Remote Request", msg.format == CAN_FRAME_STD ? "STD" : "EXT");
		}
	}
	
	if(int_sr & CAN_IF_RXOV_Msk)
	{
		printf("\r\nCAN RX Overflow\r\n");
		
		CAN_Close(CAN);
		for(i = 0; i < CyclesPerUs; i++) __NOP();
		CAN_Open(CAN);
	}
	
	if(int_sr & CAN_IF_ERRWARN_Msk)
	{
		if(CAN->SR & CAN_SR_BUSOFF_Msk)
		{
			printf("\r\nCAN Bus Off\r\n");
			printf("\r\nCAN->CR.RST = %d\r\n", CAN->CR & CAN_CR_RST_Msk ? 1 : 0);	// 发生 BusOff 时 CR.RST 自动置位，进入复位模式
			
			CAN_Open(CAN);			// 清除 CR.RST 位，退出复位模式，进入正常工作模式
		}
		else if(CAN->SR & CAN_SR_ERRWARN_Msk)
		{
			printf("\r\nCAN Error Warning\r\n");
		}
	}
	
	if(int_sr & CAN_IF_ERRPASS_Msk)
	{
		printf("\r\nCAN Error Passive\r\n");
	}
}

/****************************************************************************************************************************************** 
* 函数名称: sameBits()
* 功能说明: 找到所有ID中相同位置值相同的位，比如所有ID的第10位都是0（或1）则返回值的第10位为1
* 输    入: 
* 输    出: 
* 注意事项: 无
******************************************************************************************************************************************/
uint32_t sameBits(uint32_t std_id[], uint32_t std_n, uint32_t ext_id[], uint32_t ext_n)
{
	uint32_t i, j;

	uint32_t mask = 0;
	
	for(i = 0; i < std_n; i++)
		std_id[i] = std_id[i] << 18;
	
	for(i = 0; i < std_n-1; i++)
	{
		for(j = i+1; j < std_n; j++)
		{
			mask |= std_id[i] ^ std_id[j];
		}
	}
	
	for(i = 0; i < ext_n-1; i++)
	{
		for(j = i+1; j < ext_n; j++)
		{
			mask |= ext_id[i] ^ ext_id[j];
		}
	}
	
	return ~mask;
}


void SerialInit(void)
{
	UART_InitStructure UART_initStruct;
	
	PORT_Init(PORTA, PIN0, FUNMUX_UART0_RXD, 1);	//GPIOA.0配置为UART0输入引脚
	PORT_Init(PORTA, PIN1, FUNMUX_UART0_TXD, 0);	//GPIOA.1配置为UART0输出引脚
 	
 	UART_initStruct.Baudrate = 57600;
	UART_initStruct.DataBits = UART_DATA_8BIT;
	UART_initStruct.Parity = UART_PARITY_NONE;
	UART_initStruct.StopBits = UART_STOP_1BIT;
	UART_initStruct.RXThresholdIEn = 0;
	UART_initStruct.TXThresholdIEn = 0;
	UART_initStruct.TimeoutIEn = 0;
 	UART_Init(UART0, &UART_initStruct);
	UART_Open(UART0);
}

/****************************************************************************************************************************************** 
* 函数名称: fputc()
* 功能说明: printf()使用此函数完成实际的串口打印动作
* 输    入: int ch		要打印的字符
*			FILE *f		文件句柄
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
int fputc(int ch, FILE *f)
{
	UART_WriteByte(UART0, ch);
	
	while(UART_IsTXBusy(UART0));
 	
	return ch;
}
