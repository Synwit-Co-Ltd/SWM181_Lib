#include "SWM181.h"

#define EEPROM_ADDR	  0x1A000

uint32_t WrBuff[64] = {		// 一次最多写入64字（即256字节）
	0x14141414, 0x15151515, 0x16161616, 0x17171717, 0x18181818, 0x19191919, 0x1A1A1A1A, 0x1B1B1B1B, 
	0x1C1C1C1C, 0x1D1D1D1D, 0x1E1E1E1E, 0x1F1F1F1F, 0x20202020, 0x21212121, 0x22222222, 0x23232323, 
	0x24242424, 0x25252525, 0x26262626, 0x27272727, 0x28282828, 0x29292929, 0x2A2A2A2A, 0x2B2B2B2B, 
	0x2C2C2C2C, 0x2D2D2D2D, 0x2E2E2E2E, 0x2F2F2F2F, 0x30303030, 0x31313131, 0x32323232, 0x33333333, 
	0x34343434, 0x35353535, 0x36363636, 0x37373737, 0x38383838, 0x39393939, 0x3A3A3A3A, 0x3B3B3B3B, 
	0x3C3C3C3C, 0x3D3D3D3D, 0x3E3E3E3E, 0x3F3F3F3F, 0x40404040, 0x41414141, 0x42424242, 0x43434343, 
	0x44444444, 0x45454545, 0x46464646, 0x47474747, 0x48484848, 0x49494949, 0x4A4A4A4A, 0x4B4B4B4B, 
	0x4C4C4C4C, 0x4D4D4D4D, 0x4E4E4E4E, 0x4F4F4F4F, 0x50505050, 0x51515151, 0x52525252, 0x53535353
};
uint32_t RdBuff[64] = {0};


void SerialInit(void);

int main(void)
{
	uint32_t i;
		
	SystemInit();
	
	SerialInit();
		
	printf("\r\nAfter Erase: \r\n");
	
	FLASH_Erase(EEPROM_ADDR);
	
	FLASH_Read(EEPROM_ADDR, RdBuff, 64);
	for(i = 0; i < 64; i++) printf("0x%X, ", RdBuff[i]);
	
	printf("\r\nAfter Write: \r\n");
	
	FLASH_Write(EEPROM_ADDR, WrBuff, 64);
	
	FLASH_Read(EEPROM_ADDR, RdBuff, 64);
	for(i = 0; i < 64; i++) printf("0x%X, ", RdBuff[i]);
	
	while(1==1)
	{
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
