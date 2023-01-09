#include "SWM181.h"

#include <string.h>

#include "IOUART_RX.h"
#include "IOUART_TX.h"

char RxBuff[64] = {0};


int main(void)
{
	char buff[65];
	uint32_t stat;
	
 	SystemInit();
		
	IOUART_RX_Init();
	IOUART_TX_Init();
	
	printf("Hi from Synwit!\r\n");
	
 	while(1==1)
 	{
		IOUART_RX_RecvChars(RxBuff, 64);	//通知UART模块要接收64个字符，接收到的字符放在RxBuff中
		while((stat = IOUART_RX_State()) < IOUART_RX_BUFFULL);
		switch(stat)
		{
		case IOUART_RX_BUFFULL:
		case IOUART_RX_TIMEOUT:
			memset(buff, 0, 65);
			memcpy(buff, RxBuff, IOUART_RX_Count());	//将接收到的数据拷贝出来处理
			IOUART_RX_Clear();							//处理完接收数据必须清零状态
			printf("IOUART Received: %s\r\n", buff);			
			break;
		
		case IOUART_RX_FRAMERR:
		case IOUART_RX_PARITYERR:
			IOUART_RX_Clear();							//处理完接收数据必须清零状态
			printf("IOUART Receive Error!\r\n");
			break;
		}
 	}
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
	IOUART_TX_SendChars((char *)&ch, 1);
	
	while(IOUART_TX_IsBusy());
	
	return ch;
}
