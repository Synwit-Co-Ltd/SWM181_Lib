#ifndef __IOSPI_MST_H__
#define __IOSPI_MST_H__

extern volatile uint16_t IOSPI_Done;
extern volatile uint16_t IOSPI_Data;

void IOSPI_MST_Init(uint32_t freq);

void IOSPI_MST_Write(uint8_t addr, uint16_t data);

void IOSPI_MST_Read(uint8_t addr);

#endif //__IOSPI_MST_H__
