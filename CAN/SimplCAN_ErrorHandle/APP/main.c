#include "SWM181.h"


/* 演示步骤说明：
	一、将 CAN 的 TX 与 RX 相连，或者将 CAN 连接收发器但收发器不连接其他 CAN 节点，或者将 CAN 连接收发器且收发器连接其他 CAN 节点但其他 CAN 节点不响应 0x7FFFFFF ID，
	执行程序，通过串口助手会发现 CAN->TXERR 会从逐渐递增到 128，但此后不再递增，具体原因参考 CAN spec 上如下说明：
	
		3. When a TRANSMITTER sends an ERROR FLAG the TRANSMIT ERROR COUNT is increased by 8.

		Exception 1:
		If the TRANSMITTER is 'error passive' and detects an ACKNOWLEDGMENT ERROR because of not detecting
		a 'dominant' ACK and does not detect a 'dominant' bit while sending its PASSIVE ERROR FLAG.
	
	二、此时 ACK ERROR 不再递增 CAN->TXERR，将 CAN 的 TX 与 RX 连接断开（或者将 CAN 与收发器断开），触发 BIT ERROR，通过串口助手可发现 CAN->TXERR 继续递增到 256，
	导致 CAN->SR.BUSOFF 和 CAN->CR.RST 置位，然后 CAN->TXERR 值变成 127，此时 CAN 节点处于 Bus Off 状态，CAN 控制器处于复位状态。
	
	三、CAN->CR.RST 被软件清零后，将 CAN 的 TX 与 RX 相连（或者将 CAN 与收发器相连），CAN 节点退出 Bus Off 状态
*/


void SerialInit(void);

int main(void)
{
	uint32_t i;
	uint8_t tx_data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
	CAN_InitStructure CAN_initStruct;
	
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
   	CAN_initStruct.FilterMask32b = 0x00000000;
	CAN_initStruct.FilterCheck32b = 0xFFFFFFFF;
	CAN_initStruct.RXNotEmptyIEn = 0;
	CAN_initStruct.RXOverflowIEn = 0;
	CAN_initStruct.ArbitrLostIEn = 0;
	CAN_initStruct.ErrPassiveIEn = 0;
	CAN_Init(CAN, &CAN_initStruct);
	
	CAN_Open(CAN);
	
	while(1==1)
	{
		while((CAN->SR & CAN_SR_BUSOFF_Msk) == 0)
		{
			CAN_Transmit(CAN, CAN_FRAME_STD, 0x60D, tx_data, 8, 1);
			
			while(CAN_TXComplete(CAN) == 0) __NOP();
			
			printf("CAN->TXERR: %d\r\n", CAN->TXERR);
			
			for(i = 0; i < SystemCoreClock/32; i++) __NOP();
		}
		
		printf("CAN->CR.RST: %d\r\n", CAN->CR & CAN_CR_RST_Msk);	// 发生 BusOff 时 CR.RST 自动置位，进入复位模式
		
		CAN->CR &= ~CAN_CR_RST_Msk;		// 清除 CR.RST 位，退出复位模式，进入正常工作模式
		
		for(i = 0; i < SystemCoreClock/2; i++) __NOP();
	}
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
