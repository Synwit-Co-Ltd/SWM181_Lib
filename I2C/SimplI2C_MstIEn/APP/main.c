#include "SWM181.h"

#include <string.h>

#define SLV_ADDR  0x6C

char mst_txbuff[4] = {0x37, 0x55, 0xAA, 0x78};
char mst_rxbuff[4] = {0};
char slv_txbuff[4] = {0};
char slv_rxbuff[4] = {0};

uint32_t mst_txlen, mst_rxlen;

uint32_t mst_rxoper;	// 1 接收操作   0 发送操作

volatile uint32_t mst_rxnack;	// 1 接收到NAK

volatile uint32_t mst_txindx = 0;
volatile uint32_t mst_rxindx = 0;
volatile uint32_t slv_rxindx = 0;
volatile uint32_t slv_txindx = 0;

void SerialInit(void);
void I2C_Mst_Init(void);
void I2C_Slv_Init(void);
void I2C_Mst_Send(uint8_t addr, uint8_t *data, uint32_t len);
void I2C_Mst_Recv(uint8_t addr, uint32_t len);

int main(void)
{
	uint32_t i;
	
 	SystemInit();
 	
	SerialInit();
	
	I2C_Mst_Init();
	I2C_Slv_Init();
	
	GPIO_Init(GPIOA, PIN8, 1, 0, 0, 0);	// 调试辅助用
	
	while(1==1)
	{
		/*************************** Master Write ************************************/
		slv_rxindx = 0;
		
		I2C_Mst_Send(SLV_ADDR, (uint8_t *)mst_txbuff, 4);
		
		while(mst_txindx != mst_txlen + 2)	// 等待发送完成
		{
			if(mst_rxnack)
			{
				printf("Get NACK\r\n");
				goto nextloop;
			}
		}
		
		printf("Master Send %X %X %X %X Success\r\n", mst_txbuff[0], mst_txbuff[1], mst_txbuff[2], mst_txbuff[3]);
		
		
		/********************************** Master Read *******************************/
		slv_txindx = 0;
		memcpy(slv_txbuff, slv_rxbuff, 4);
		
		I2C_Mst_Recv(SLV_ADDR, 4);
		
		while(mst_rxindx != mst_rxlen + 1)	// 等待接收完成
		{
			if(mst_rxnack)
			{
				printf("Get NACK\r\n");
				goto nextloop;
			}
		}
		
		printf("Master Read %X %X %X %X Success\r\n", mst_rxbuff[0], mst_rxbuff[1], mst_rxbuff[2], mst_rxbuff[3]);
		
		if((mst_txbuff[0] == mst_rxbuff[0]) && (mst_txbuff[1] == mst_rxbuff[1]) && (mst_txbuff[2] == mst_rxbuff[2]) && (mst_txbuff[3] == mst_rxbuff[3]))
			printf("Success\r\n");
		else
			printf("Fail\r\n");

nextloop:
		I2C0->MSTCMD = (1 << I2C_MSTCMD_STO_Pos);
		while(I2C0->MSTCMD & I2C_MSTCMD_TIP_Msk) ;	//等待发送完成
		for(i = 0; i < 10000000; i++) ;
	}
}

void I2C_Mst_Init(void)
{
	I2C_InitStructure I2C_initStruct;
	
	PORT_Init(PORTA, PIN4, FUNMUX_I2C0_SCL, 1);		//GPIOA.4配置为I2C0 SCL引脚
	PORTA->OPEND |= (1 << PIN4);					//开漏
	PORTA->PULLU |= (1 << PIN4);					//上拉
	PORT_Init(PORTA, PIN5, FUNMUX_I2C0_SDA, 1);		//GPIOA.5配置为I2C0 SDA引脚
	PORTA->OPEND |= (1 << PIN5);					//开漏
	PORTA->PULLU |= (1 << PIN5);					//上拉
	
	I2C_initStruct.Master = 1;
	I2C_initStruct.Addr7b = 1;
	I2C_initStruct.MstClk = 100000;
	I2C_initStruct.MstIEn = 1;
	I2C_Init(I2C0, &I2C_initStruct);
	
	IRQ_Connect(IRQ0_15_I2C0, IRQ3_IRQ, 3);
	
	I2C_Open(I2C0);
}

void I2C_Mst_Send(uint8_t addr, uint8_t *data, uint32_t len)
{
	uint32_t i;
	
	if(len > 4) len = 4;
	
	for(i = 0; i < len; i++)
		mst_txbuff[i] = data[i];
	
	mst_txlen = len;
	
	mst_rxoper = 0;
	mst_txindx = 0;
	mst_rxnack = 0;
	
	I2C0->MSTDAT = (addr << 1) | 0;
	I2C0->MSTCMD = (1 << I2C_MSTCMD_STA_Pos) |		
				   (1 << I2C_MSTCMD_WR_Pos);	//发送起始位和从机地址
}

void I2C_Mst_Recv(uint8_t addr, uint32_t len)
{
	if(len > 4) len = 4;
	
	mst_rxlen = len;
	
	mst_rxoper = 1;
	mst_rxindx = 0;
	mst_rxnack = 0;
	
	I2C0->MSTDAT = (addr << 1) | 1;
	I2C0->MSTCMD = (1 << I2C_MSTCMD_STA_Pos) |		
				   (1 << I2C_MSTCMD_WR_Pos);	//发送起始位和从机地址
}

void IRQ3_Handler(void)
{	
	I2C0->MSTCMD = (1 << I2C_MSTCMD_IF_Pos);	// 注意：不能用“|=”，因为busy位会导致发送STOP
	GPIO_InvBit(GPIOA, PIN8);
	if(mst_rxoper)	// 接收操作
	{
		if(mst_rxindx == 0)
		{
			if(I2C0->MSTCMD & I2C_MSTCMD_RXACK_Msk)
			{
				mst_rxnack = 1;
				
				return;
			}
			
			I2C0->MSTCMD = (1 << I2C_MSTCMD_RD_Pos) |
						   (0 << I2C_MSTCMD_ACK_Pos);
		}
		else if(mst_rxindx < mst_rxlen-1)
		{
			mst_rxbuff[mst_rxindx-1] = I2C0->MSTDAT;
			
			I2C0->MSTCMD = (1 << I2C_MSTCMD_RD_Pos) |
					       (0 << I2C_MSTCMD_ACK_Pos);
		}
		else if(mst_rxindx == mst_rxlen-1)
		{
			mst_rxbuff[mst_rxindx-1] = I2C0->MSTDAT;
			
			I2C0->MSTCMD = (1 << I2C_MSTCMD_RD_Pos)  |
						   (1 << I2C_MSTCMD_ACK_Pos) |
					       (1 << I2C_MSTCMD_STO_Pos);		//读取数据、发送NACK响应，完成后发送STOP
		}
		else if(mst_rxindx == mst_rxlen)
		{
			mst_rxbuff[mst_rxindx-1] = I2C0->MSTDAT;
		}
		
		mst_rxindx++;
	}
	else			// 发送操作
	{
		if(I2C0->MSTCMD & I2C_MSTCMD_RXACK_Msk)
		{
			mst_rxnack = 1;
			
			return;
		}
		
		if(mst_txindx < mst_txlen)
		{
			I2C0->MSTDAT = mst_txbuff[mst_txindx];
			I2C0->MSTCMD = (1 << I2C_MSTCMD_WR_Pos);
		}
		else if(mst_txindx == mst_txlen)
		{
			I2C0->MSTCMD = (1 << I2C_MSTCMD_STO_Pos);
		}
		
		mst_txindx++;
	}
}

void I2C_Slv_Init(void)
{
	I2C_InitStructure I2C_initStruct;
	
	PORT_Init(PORTA, PIN6, FUNMUX_I2C1_SCL, 1);		//GPIOA.6配置为I2C1 SCL引脚
	PORTA->OPEND |= (1 << PIN6);					//开漏
	PORTA->PULLU |= (1 << PIN6);					//上拉
	PORT_Init(PORTA, PIN7, FUNMUX_I2C1_SDA, 1);		//GPIOA.7配置为I2C1 SDA引脚
	PORTA->OPEND |= (1 << PIN7);					//开漏
	PORTA->PULLU |= (1 << PIN7);					//上拉
	
	I2C_initStruct.Master = 0;
	I2C_initStruct.Addr7b = 1;
	I2C_initStruct.SlvAddr = SLV_ADDR;
	I2C_initStruct.SlvAddrMask = 0xFF;
	I2C_initStruct.SlvSTADetIEn = 1;
	I2C_initStruct.SlvRdReqIEn = 0;
	I2C_initStruct.SlvWrReqIEn = 1;
	I2C_initStruct.SlvTxEndIEn = 1;
	I2C_initStruct.SlvRxEndIEn = 1;
	I2C_initStruct.SlvSTODetIEn = 1;
	I2C_Init(I2C1, &I2C_initStruct);
	
	IRQ_Connect(IRQ0_15_I2C1, IRQ4_IRQ, 2);
	
	I2C_Open(I2C1);
}


void IRQ4_Handler(void)
{
	if(I2C1->SLVIF & I2C_SLVIF_STADET_Msk)				//收到起始位
	{
		I2C1->SLVIF = (1 << I2C_SLVIF_STADET_Pos);
		
		I2C1->SLVTX = slv_txbuff[0];
		slv_txindx = 1;
	}
	else if(I2C1->SLVIF & I2C_SLVIF_STODET_Msk)			//收到停止位
	{
		I2C1->SLVIF = (1 << I2C_SLVIF_STODET_Pos);
	}
	else if(I2C1->SLVIF & I2C_SLVIF_WRREQ_Msk)			//收到写请求
	{
		I2C1->SLVIF = (1 << I2C_SLVIF_WRREQ_Pos);
		
		slv_rxindx = 0;
		I2C1->SLVCR |= (1 << I2C_SLVCR_ACK_Pos);
	}
	else if(I2C1->SLVIF & I2C_SLVIF_RXEND_Msk)			//接收完成
	{
		I2C1->SLVIF = (1 << I2C_SLVIF_RXEND_Pos);
		
		slv_rxbuff[slv_rxindx] = I2C1->SLVRX;
		if(slv_rxindx < 3) slv_rxindx++;
	}
	else if(I2C1->SLVIF & I2C_SLVIF_RDREQ_Msk)			//收到读请求
	{		
		I2C1->SLVIF = (1 << I2C_SLVIF_RDREQ_Pos);
		
		//收到读请求后会立即把SLVTX中的值发送出去，而不是等待软件写入后再发送；所以将第一个数据写入SLVTX只能提前到“收到起始位”
	}
	else if(I2C1->SLVIF & I2C_SLVIF_TXEND_Msk)			//发送完成
	{
		I2C1->SLVIF = (1 << I2C_SLVIF_TXEND_Pos);
		
		I2C1->SLVTX = slv_txbuff[slv_txindx];
		if(slv_txindx < 3) slv_txindx++;
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
