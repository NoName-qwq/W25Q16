#ifndef __W25Q_H
#define __W25Q_H 

#include <stdlib.h>
#include "spi.h"

/*
    SFDP Register
    000000h - 0000FFh

    Security Register
    001000h - 0010FFh
    002000h - 0020FFh
    003000h - 0030FFh

    页编程只能将1改为0，无法将0改为1
    擦除可将0改为1

    擦/写时BUSY=1
    读数据指令在BUSY=1时被忽略
    读状态寄存器指令可以在任何时候使用 以读取BUSY状态

    本库擦/写函数末端含有BUSY等待
    擦/写后可直接读数据


    **本库在STM32F407ZET6 测试通过
    **使用SPI1通信 CS引脚为PB15
    **移植可直接修改.h宏定义和.c的SPI通信驱动
    **使用的W25Q16未引出HOLD和WP引脚，模块布线HOLD连接VCC，WP疑似未作处理

*/

#define W25Q_CS_PORT    GPIOB
#define W25Q_CS_PIN     GPIO_PIN_15

#define W25Q_CS_SET()  HAL_GPIO_WritePin(W25Q_CS_PORT, W25Q_CS_PIN, GPIO_PIN_SET)
#define W25Q_CS_CLR()  HAL_GPIO_WritePin(W25Q_CS_PORT, W25Q_CS_PIN, GPIO_PIN_RESET)        //片选端口   	 


#define ManufactDeviceID_CMD	0x90

#define READ_STATU_REGISTER_1   0x05
#define READ_STATU_REGISTER_2   0x35
#define READ_STATU_REGISTER_3   0x15

#define READ_DATA_CMD	        0x03

#define WRITE_ENABLE_CMD	    0x06
#define WRITE_DISABLE_CMD	    0x04

#define SECTOR_ERASE_CMD	    0x20
#define CHIP_ERASE_CMD	        0xc7

#define PAGE_PROGRAM_CMD        0x02


uint16_t W25QXX_ReadID(void);
uint8_t W25QXX_Read_SFDP(uint8_t* buffer);
uint8_t W25QXX_ReadSR(uint8_t reg);
void W25QXX_Write_Enable(uint8_t enable);
int W25QXX_Read(uint8_t* buffer, uint32_t start_addr, uint16_t nbytes);
void W25QXX_Erase_Sector(uint32_t sector_addr);
void W25QXX_Page_Program(uint8_t* dat, uint32_t WriteAddr, uint16_t nbytes);

#endif