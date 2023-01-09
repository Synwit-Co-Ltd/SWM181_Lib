#include "SWM181.h"

#include "can.h"
#include "canfestival.h"
#include "TestSlave.h"

#include "CAN_SWM181.h"
#include "timers_SWM181.h"

s_BOARD SlaveBoard = {"0", "1M"};

unsigned char nodeID;	//节点 nodeID

void SerialInit(void);
int can_slave_start(void);


int main(void)
{
	SystemInit();
	
	SerialInit();

    can_slave_init();

    can_slave_start();

    while(1)
    {
        if(g_ucCanRxFlag)
        {
            g_ucCanRxFlag=0;

            can_recv_thread();
        }
    }
}

void InitNodes(CO_Data* d, UNS32 id)
{
	if(strcmp(SlaveBoard.baudrate, "none")) 
	{
		nodeID=0x0d;
        setNodeId(&TestSlave_Data, nodeID);
        setState(&TestSlave_Data, Initialisation);
	}
}


void TestSlave_heartbeatError(CO_Data* d, UNS8 heartbeatID)
{
	printf("TestSlave_heartbeatError %d\n", heartbeatID);
}

void TestSlave_initialisation(CO_Data* d)
{
	printf("TestSlave_initialisation\n");
}

void TestSlave_preOperational(CO_Data* d)
{
	printf("TestSlave_preOperational\n");
}

void TestSlave_operational(CO_Data* d)
{
	printf("TestSlave_operational\n");
}

void TestSlave_stopped(CO_Data* d)
{
	printf("TestSlave_stopped\n");
}

void TestSlave_post_sync(CO_Data* d)
{
    printf("TestSlave_post_sync\n");
}

void TestSlave_post_TPDO(CO_Data* d)
{
	printf("TestSlave_post_TPDO\n");
}

void TestSlave_storeODSubIndex(CO_Data* d, UNS16 wIndex, UNS8 bSubindex)
{
	/*TODO : 
	 * - call getODEntry for index and subindex, 
	 * - save content to file, database, flash, nvram, ...
	 * */
	printf("TestSlave_storeODSubIndex : %4.4x %2.2x\n", wIndex,  bSubindex);
}

void TestSlave_post_emcy(CO_Data* d, UNS8 nodeID, UNS16 errCode, UNS8 errReg)
{
	printf("Slave received EMCY message. Node: %2.2x  ErrorCode: %4.4x  ErrorRegister: %2.2x\n", nodeID, errCode, errReg);
}

int can_slave_start(void)
{
    if(strcmp(SlaveBoard.baudrate, "none"))
    {
        TestSlave_Data.heartbeatError  = TestSlave_heartbeatError;
        TestSlave_Data.initialisation  = TestSlave_initialisation;
        TestSlave_Data.preOperational  = TestSlave_preOperational;
        TestSlave_Data.operational     = TestSlave_operational;
        TestSlave_Data.stopped         = TestSlave_stopped;
        TestSlave_Data.post_sync       = TestSlave_post_sync;
        TestSlave_Data.post_TPDO       = TestSlave_post_TPDO;
		TestSlave_Data.storeODSubIndex = TestSlave_storeODSubIndex;
        TestSlave_Data.post_emcy       = TestSlave_post_emcy;
	}

    StartTimerLoop(&InitNodes);     // Start timer thread

	return 0;
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
