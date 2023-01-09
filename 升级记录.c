2018/4/13
UART增加了奇偶校验功能、LIN功能、自动波特率校正功能
SPI增加传输完成中断、修复了FIFO满时半满标志不置位问题
TIMR中断修改成每个定时器在中断向量表中占一个位置
PWM中断修改成每个通道在中断向量表中占一个位置

2018/4/19
UART->DATA寄存器添加PAERR奇偶校验错误指示位
CSL\SWM181_StdPeriph_Driver\SWM181_uart.c文件中UART_ReadByte()修改，添加奇偶错误处理

2018/5/9
CSL\SWM181_StdPeriph_Driver\SWM181_dma.c文件中DMA_CH_Config()函数和DMA_CHM_Config()函数中，将：
DMA->IF  = (1 << chn);		//清除中断标志
DMA->IE |= (1 << chn);
if(int_en)	DMA->IM &= ~(1 << chn);
else		DMA->IM |=  (1 << chn);
修改为：
DMA_CH_INTClr(chn);
if(int_en)	DMA_CH_INTEn(chn);
else		DMA_CH_INTDis(chn);

同时DMA_CH_INTEn()和DMA_CH_INTDis()中不仅配置IM寄存，还加上IE寄存器配置

2018/5/30
CSL\SWM181_StdPeriph_Driver\SWM181_gpio.c文件中添加确保引脚“读-改-写”操作原子性的GPIO_Atomic类型函数

2018/6/4
CSL\CMSIS\DeviceSupport\SWM181.h文件中更新I2C模块寄存器定义，并更新I2C初始化函数和例程

2018/6/27
CSL\SWM181_StdPeriph_Driver\SWM181_can.h文件中struct CAN_RXMessage的定义中添加format字段，表明接收到的这个消息是标准帧还是扩展帧
CSL\SWM181_StdPeriph_Driver\SWM181_can.c文件中CAN_Receive()函数中给CAN_RXMessage->format正确赋值
CAN\SimplCAN_RX_Interrupt例程中打印信息时打印出CAN_RXMessage->format字段

2018/7/2
CSL\CMSIS\DeviceSupport\system_SWM181.c文件内SystemCoreClockUpdate()函数中，将：
if(SYS->LRCCR & SYS_CLKSEL_LFCK_Msk)			//LFCK <= XTAL
改正为：
if(SYS->CLKSEL & SYS_CLKSEL_LFCK_Msk)			//LFCK <= XTAL

2018/7/26
CSL\SWM181_StdPeriph_Driver\SWM181_flash.c文件内FLASH_Read()函数根据当前CACHE BYPASS情况自动处理两种情况下不同方法的读实现
CSL\SWM181_StdPeriph_Driver\SWM181_flash.c文件内FLASH_Erase()和FLASH_Write()函数中调用足够次CACHE_Invalid()

2018/8/17
CSL\SWM181_StdPeriph_Driver\SWM181_uart.c文件内UART_SetBaudrate()函数中，将：
UARTx->BAUD |= ((SystemCoreClock/16/baudrate) << UART_BAUD_BAUD_Pos);
改正为：
UARTx->BAUD |= ((SystemCoreClock/16/baudrate - 1) << UART_BAUD_BAUD_Pos);

2018/8/21
CSL\CMSIS\DeviceSupport\SWM181.h文件中struct UART_TypeDef结构体定义，寄存器CTRL2改叫CFG

2018/8/23
CSL\SWM181_StdPeriph_Driver\SWM181_spi.c文件中添加SPI_INTRXHalfFullClr()、SPI_INTRXFullClr()、SPI_INTTXHalfFullClr()、SPI_INTTXEmptyClr()四个中断标志清除函数

2018/8/28
CSL\SWM181_StdPeriph_Driver\SWM181_spi.c文件中添加SPI_INTTXWordCompleteEn()、SPI_INTTXWordCompleteDis()、SPI_INTTXWordCompleteClr()、SPI_INTTXWordCompleteStat()四个字发送完成中断相关函数

2018/8/31
CSL\SWM181_StdPeriph_Driver\SWM181_spi.c文件中SPI_Init()函数内添加接收、发送FIFO清空，然后清除FIFO复位寄存器位的语句

2018/9/14
CSL\SWM181_StdPeriph_Driver\SWM181_wdt.c文件中WDT_Init()修复复位时间是设定时长2倍的问题

2018/9/25
CSL\SWM181_StdPeriph_Driver\SWM181_sleep.c文件中添加EnterSleepMode()、EnterStopMode()两个函数的IAR实现
CSL\SWM181_StdPeriph_Driver\SWM181_flash.c文件中AppToIsp()函数中，将__dsb(0)替换成__DSB()

2018/10/25
CSL\SWM181_StdPeriph_Driver\SWM181_flash.c文件中FLASH_Erase()函数中，将：
for(i = 0; i < 4096; i += 128)
	CACHE_Invalid(addr + i);
修正为：
addr = addr / 4096 * 4096;
for(i = 0; i < 4096; i += 128)
	CACHE_Invalid(addr + i);
比如以FLASH_Erase(0x8100)调用此函数，[0x8000:0x9000]区间被擦除，但之前的代码Invalid的区间却是[0x8100:0x9100]，这样以Cache方式读取[0x8000:0x8100]区间时就会出错，明明已经被擦除，却仍然返回擦除前的数据

2018/10/29
CSL\SWM181_StdPeriph_Driver\SWM181_sdadc.c文件中SDADC_Config_Set()函数中，将：
case SDADC_CFG_C:
	SDADCx->CFGB = (gian << SDADC_CFG_GAIN_Pos)    |
修正为：
case SDADC_CFG_C:
	SDADCx->CFGC = (gian << SDADC_CFG_GAIN_Pos)    |

CSL\SWM181_StdPeriph_Driver\SWM181_can.c文件中CAN_SetFilter32b()和CAN_SetFilter16b()函数中，将：
CANx->FILTER.AMR[0] = initStruct->FilterMask32b & 0xFF;
CANx->FILTER.AMR[1] = (initStruct->FilterMask32b >>  8) & 0xFF;
... ...
修正为：
CANx->FILTER.AMR[3] = initStruct->FilterMask32b & 0xFF;
CANx->FILTER.AMR[2] = (initStruct->FilterMask32b >>  8) & 0xFF;

2019/01/16
CSL\SWM181_StdPeriph_Driver\SWM181_dma.c文件中DMA_CH_Config()函数中，将：
DMA_CH_INTClr(chn);
if(int_en) DMA_CH_INTEn(chn);
else	   DMA_CH_INTDis(chn);
修正为：
DMA->IE = 0x37;			//即使不使能中断，也可以查询状态/标志
DMA_CH_INTClr(chn);
if(int_en) DMA_CH_INTEn(chn);
else	   DMA_CH_INTDis(chn);

DMA_CHM_Config()也做类似修正，DMA_CH_INTEn()和DMA_CH_INTDis()中不再操作IE，只操作IM

2019/03/04
CSL\SWM181_StdPeriph_Driver\SWM181_uart.c 文件中 UART_GetBaudrate() 中将：
return (UARTx->BAUD & UART_BAUD_BAUD_Msk);
修正为：
return SystemCoreClock/16/(((UARTx->BAUD & UART_BAUD_BAUD_Msk) >> UART_BAUD_BAUD_Pos) + 1);

2019/04/18
CSL\CMSIS\DeviceSupport\SWM181.h 文件中，修正CAN->ECC寄存器的位定义

2019/05/22
CSL\CMSIS\DeviceSupport\SWM181.h 文件中，SYS_TypeDef 添加 RSTSR 寄存器定义

2019/07/31
CSL\SWM181_StdPeriph_Driver\SWM181_uart.c 文件中 UART_LINIsDetected() 函数中，将：
return (UARTx->LINCR & UART_LINCR_BRKDETIE_Msk) ? 1 : 0;
修正为：
return (UARTx->LINCR & UART_LINCR_BRKDETIF_Msk) ? 1 : 0;

2019/09/10
CSL\SWM181_StdPeriph_Driver\SWM181_i2c.c 文件中，添加 I2C_INTEn、I2C_INTDis、I2C_INTClr、I2C_INTStat 函数定义
添加 SimplI2C_MstIEn 主机中断使用演示例程

2020/03/03
CSL\SWM181_StdPeriph_Driver\SWM181_i2c.c 文件中，添加 I2C_Start、I2C_Stop、I2C_Write、I2C_Read 函数定义

2020/03/12
CSL\SWM181_StdPeriph_Driver\SWM181_port.h 文件中，将：
#define PORTA_PIN2_SWCLK		2
#define PORTA_PIN3_SWDIO		2
修正为：
#define PORTA_PIN2_SWCLK		3
#define PORTA_PIN3_SWDIO		3

2020/09/22
CSL\SWM181_StdPeriph_Driver\SWM181_can.c 文件中，删除 CAN_INTRXNotEmptyStat()、CAN_INTTXBufEmptyStat()、CAN_INTErrWarningStat()、CAN_INTRXOverflowStat()、
	CAN_INTWakeupStat()、CAN_INTErrPassiveStat()、CAN_INTArbitrLostStat()、CAN_INTBusErrorStat() 这8个函数定义，添加 CAN_INTStat() 函数定义，因为 CAN->IF 寄存器读取清零，不能多次读取

2021/03/02
CSL\CMSIS\DeviceSupport\startup\arm\startup_SWM181.s 文件中，将：
				DCD	 0
				DCD	 0
				DCD	 SRAM_SWITCH
更改为：
				DCD	 0x0B11FFAC
				DCD	 0x6000
				DCD	 SRAM_SWITCH
防止在用户编写UserBoot时，忘记在0x24地址处写入程序大小，导致APP基于RAM执行时UserBoot不拷贝APP到CodeRAM中，IAR和GCC的startup文件也做类似修改

2021/05/17
CSL\CMSIS\DeviceSupport\SWM181.h 文件中，CAN_TypeDef 定义中，将：
		union {
			struct {
				__O  uint32_t INFO;
				
				__O  uint32_t DATA[12];
			} TXFRAME;
			
			struct {
				__I  uint32_t INFO;
				
				__I  uint32_t DATA[12];
			} RXFRAME;
		};
更改为：
		struct {
			__IO uint32_t INFO;
			
			__IO uint32_t DATA[12];
		} FRAME;
简化结构体定义

2021/05/31
CSL\SWM181_StdPeriph_Driver\SWM181_flash.c 文件中，FLASH_Write() 函数中添加如下代码：
	FLASH->CR = (1 << FLASH_CR_FFCLR_Pos);			//Clear FIFO
	FLASH->CR = 0;
执行Flash写入前清空FIFO，防止FIFO中有数据导致写入操作卡死

2022/06/22
CSL\CMSIS\DeviceSupport\system_SWM181.c 文件中，将 switchToRC24MHz()、switchToRC6MHz()、switchToRC48MHz()、switchToRC12MHz()、switchToRC32KHz()、switchToXTAL() 六个函数由内部函数改为外部函数，方便用户使用

2022/10/21
CSL\SWM190_StdPeriph_Driver\SWM190_uart.c 文件中，将：
void UART_INTRXThresholdEn(UART_TypeDef * UARTx);
void UART_INTRXThresholdDis(UART_TypeDef * UARTx);
uint32_t UART_INTRXThresholdStat(UART_TypeDef * UARTx);
void UART_INTTXThresholdEn(UART_TypeDef * UARTx);
void UART_INTTXThresholdDis(UART_TypeDef * UARTx);
uint32_t UART_INTTXThresholdStat(UART_TypeDef * UARTx);
void UART_INTTimeoutEn(UART_TypeDef * UARTx);
void UART_INTTimeoutDis(UART_TypeDef * UARTx);
uint32_t UART_INTTimeoutStat(UART_TypeDef * UARTx);
void UART_INTTXDoneEn(UART_TypeDef * UARTx);
void UART_INTTXDoneDis(UART_TypeDef * UARTx);
uint32_t UART_INTTXDoneStat(UART_TypeDef * UARTx);
简化为：
void UART_INTEn(UART_TypeDef * UARTx, uint32_t it);
void UART_INTDis(UART_TypeDef * UARTx, uint32_t it);
uint32_t UART_INTStat(UART_TypeDef * UARTx, uint32_t it);

CSL\SWM181_StdPeriph_Driver\SWM181_can.c 文件中，将：
void CAN_INTRXNotEmptyEn(CAN_TypeDef * CANx);
void CAN_INTRXNotEmptyDis(CAN_TypeDef * CANx);
void CAN_INTTXBufEmptyEn(CAN_TypeDef * CANx);
void CAN_INTTXBufEmptyDis(CAN_TypeDef * CANx);
void CAN_INTErrWarningEn(CAN_TypeDef * CANx);
void CAN_INTErrWarningDis(CAN_TypeDef * CANx);
void CAN_INTRXOverflowEn(CAN_TypeDef * CANx);
void CAN_INTRXOverflowDis(CAN_TypeDef * CANx);
void CAN_INTRXOverflowClear(CAN_TypeDef * CANx);
void CAN_INTWakeupEn(CAN_TypeDef * CANx);
void CAN_INTWakeupDis(CAN_TypeDef * CANx);
void CAN_INTErrPassiveEn(CAN_TypeDef * CANx);
void CAN_INTErrPassiveDis(CAN_TypeDef * CANx);
void CAN_INTArbitrLostEn(CAN_TypeDef * CANx);
void CAN_INTArbitrLostDis(CAN_TypeDef * CANx);
void CAN_INTBusErrorEn(CAN_TypeDef * CANx);
void CAN_INTBusErrorDis(CAN_TypeDef * CANx);
简化为：
void CAN_INTEn(CAN_TypeDef * CANx, uint32_t it);
void CAN_INTDis(CAN_TypeDef * CANx, uint32_t it);
void CAN_INTClr(CAN_TypeDef * CANx, uint32_t it);

CSL\SWM181_StdPeriph_Driver\SWM181_spi.c 文件中，将：
void SPI_INTRXHalfFullEn(SPI_TypeDef * SPIx);
void SPI_INTRXHalfFullDis(SPI_TypeDef * SPIx);
void SPI_INTRXHalfFullClr(SPI_TypeDef * SPIx);
uint32_t SPI_INTRXHalfFullStat(SPI_TypeDef * SPIx);
void SPI_INTRXFullEn(SPI_TypeDef * SPIx);
void SPI_INTRXFullDis(SPI_TypeDef * SPIx);
void SPI_INTRXFullClr(SPI_TypeDef * SPIx);
uint32_t SPI_INTRXFullStat(SPI_TypeDef * SPIx);
void SPI_INTRXOverflowEn(SPI_TypeDef * SPIx);
void SPI_INTRXOverflowDis(SPI_TypeDef * SPIx);
void SPI_INTRXOverflowClr(SPI_TypeDef * SPIx);
uint32_t SPI_INTRXOverflowStat(SPI_TypeDef * SPIx);
void SPI_INTTXHalfFullEn(SPI_TypeDef * SPIx);
void SPI_INTTXHalfFullDis(SPI_TypeDef * SPIx);
void SPI_INTTXHalfFullClr(SPI_TypeDef * SPIx);
uint32_t SPI_INTTXHalfFullStat(SPI_TypeDef * SPIx);
void SPI_INTTXEmptyEn(SPI_TypeDef * SPIx);
void SPI_INTTXEmptyDis(SPI_TypeDef * SPIx);
void SPI_INTTXEmptyClr(SPI_TypeDef * SPIx);
uint32_t SPI_INTTXEmptyStat(SPI_TypeDef * SPIx);
void SPI_INTTXCompleteEn(SPI_TypeDef * SPIx);
void SPI_INTTXCompleteDis(SPI_TypeDef * SPIx);
void SPI_INTTXCompleteClr(SPI_TypeDef * SPIx);
uint32_t SPI_INTTXCompleteStat(SPI_TypeDef * SPIx);
void SPI_INTTXWordCompleteEn(SPI_TypeDef * SPIx);
void SPI_INTTXWordCompleteDis(SPI_TypeDef * SPIx);
void SPI_INTTXWordCompleteClr(SPI_TypeDef * SPIx);
uint32_t SPI_INTTXWordCompleteStat(SPI_TypeDef * SPIx);
简化为：
void SPI_INTEn(SPI_TypeDef * SPIx, uint32_t it);
void SPI_INTDis(SPI_TypeDef * SPIx, uint32_t it);
void SPI_INTClr(SPI_TypeDef * SPIx, uint32_t it);
uint32_t SPI_INTStat(SPI_TypeDef * SPIx, uint32_t it);
