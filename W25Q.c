#include "W25Q.h"
#include <stdint.h>
static void W25QXX_Wait_Busy(void);


// ===============================================SPI通信驱动================================================
/* @brief    SPI发送指定长度的数据
 * @param    buf  —— 发送数据缓冲区首地址
 * @param    size —— 要发送数据的字节数
 * @retval   成功返回HAL_OK
 */
static HAL_StatusTypeDef SPI_Transmit(uint8_t* send_buf, uint16_t size)
{
    return HAL_SPI_Transmit(&hspi1, send_buf, size, 100);
}


/**
 * @brief   SPI接收指定长度的数据
 * @param   buf  —— 接收数据缓冲区首地址
 * @param   size —— 要接收数据的字节数
 * @retval  成功返回HAL_OK
 */
static HAL_StatusTypeDef SPI_Receive(uint8_t* recv_buf, uint16_t size)
{
   return HAL_SPI_Receive(&hspi1, recv_buf, size, 100);
}

/**
 * @brief   SPI在发送数据的同时接收指定长度的数据
 * @param   send_buf  —— 接收数据缓冲区首地址
 * @param   recv_buf  —— 接收数据缓冲区首地址
 * @param   size —— 要发送/接收数据的字节数
 * @retval  成功返回HAL_OK
 */
static HAL_StatusTypeDef SPI_TransmitReceive(uint8_t* send_buf, uint8_t* recv_buf, uint16_t size)
{
   return HAL_SPI_TransmitReceive(&hspi1, send_buf, recv_buf, size, 100);
}

// ===============================================W25Q16驱动================================================

/**
 * @brief   读取Flash内部的ID
 * @param   none
 * @retval  成功返回device_id
 */
uint16_t W25QXX_ReadID(void)
{
    uint8_t recv_buf[2] = {0};    //recv_buf[0]存放Manufacture ID, recv_buf[1]存放Device ID
    uint16_t device_id = 0;
    uint8_t send_data[4] = {ManufactDeviceID_CMD,0x00,0x00,0x00};   //待发送数据，命令+地址
    
    /* 使能片选 */
    W25Q_CS_CLR();
    
    /* 发送并读取数据 */
    if (HAL_OK == SPI_Transmit(send_data, 4)) 
    {
        if (HAL_OK == SPI_Receive(recv_buf, 2)) 
        {
            device_id = (recv_buf[0] << 8) | recv_buf[1];
        }
    }
    
    /* 取消片选 */
    W25Q_CS_SET();

    return device_id;
}


uint8_t W25QXX_Read_SFDP(uint8_t* buffer){
    uint8_t cmd = 0x5A;
    uint8_t start_addr[4] = {0x00,0x00,0x00,0x00};

	W25QXX_Wait_Busy();

    W25Q_CS_CLR();

    SPI_Transmit(&cmd, 1);
    if (HAL_OK == SPI_Transmit(start_addr, 4)) 
    {
        if (HAL_OK == SPI_Receive(buffer, 256)) 
        {
            W25Q_CS_SET();    

            return 0;
        }
    }
    
    W25Q_CS_SET();    


    return -1;

}

/**
 * @brief     读取W25QXX的状态寄存器，W25Q16一共有3个状态寄存器
 * @param     reg  —— 状态寄存器编号(1~3)
 * @retval    状态寄存器的值
 */
uint8_t W25QXX_ReadSR(uint8_t reg)
{
    uint8_t result = 0; 
    uint8_t send_buf[1];
    switch(reg)
    {
        case 1:
            send_buf[0] = READ_STATU_REGISTER_1;
            break;
        case 2:
            send_buf[0] = READ_STATU_REGISTER_2;
            break;
        case 3:
            send_buf[0] = READ_STATU_REGISTER_3;
            break;
        default:
            send_buf[0] = READ_STATU_REGISTER_1;
            break;
    }
    
     /* 使能片选 */
    W25Q_CS_CLR();    

    if (HAL_OK == SPI_Transmit(send_buf, 1)) 
    {
        if (HAL_OK == SPI_Receive(&result, 1)) 
        {
            W25Q_CS_SET();
            
            return result;
        }
    }
    
    /* 取消片选 */
    W25Q_CS_SET();    

    return 0;
}


/**
 * @brief	阻塞等待Flash处于空闲状态
 * @param   none
 * @retval  none
 */
static void W25QXX_Wait_Busy(void)
{
    while((W25QXX_ReadSR(1) & 0x01) == 0x01); // 等待BUSY位清空
}




/**
 * @brief    W25QXX写使能/失能,将S1寄存器的WEL置位
 * @param    none
 * @retval
 */
void W25QXX_Write_Enable(uint8_t enable)
{
    uint8_t cmd = 0;
    switch (enable) {
        case 0:
        cmd = WRITE_DISABLE_CMD;
        break;
        case 1:
        cmd = WRITE_ENABLE_CMD;
        break;
        default:
        cmd = WRITE_DISABLE_CMD;
        break;
    }
    
    W25Q_CS_CLR();
    
    SPI_Transmit(&cmd, 1);
    
    W25Q_CS_SET();
    
    W25QXX_Wait_Busy();

}

/**
 * @brief   读取SPI FLASH数据
 * @param   buffer      —— 数据存储区
 * @param   start_addr  —— 开始读取的地址(最大32bit)
 * @param   nbytes      —— 要读取的字节数(最大65535)
 * @retval  成功返回0，失败返回-1
 */
int W25QXX_Read(uint8_t* buffer, uint32_t start_addr, uint16_t nbytes)
{
    uint8_t cmd = READ_DATA_CMD;
    
    uint8_t addr[3];
    addr[0] = (start_addr >> 16) & 0xFF;
    addr[1] = (start_addr >> 8) & 0xFF;
    addr[2] = start_addr & 0xFF;
    
	W25QXX_Wait_Busy();
    
     /* 使能片选 */
    W25Q_CS_CLR();    
    SPI_Transmit(&cmd, 1);
    
    if (HAL_OK == SPI_Transmit(addr, 3)) 
    {
        if (HAL_OK == SPI_Receive(buffer, nbytes)) 
        {
            W25Q_CS_SET();    

            return 0;
        }
    }
    
    W25Q_CS_SET();    
    return -1;
}


/**
 * @brief    W25QXX擦除一个扇区
 * @param   sector_addr    —— 扇区地址 根据实际容量设置
 * @retval  none
 * @note    阻塞操作
 */
void W25QXX_Erase_Sector(uint32_t sector_addr)
{
    uint8_t cmd = SECTOR_ERASE_CMD;
    uint8_t addr[3];
    sector_addr *= 4096;    //每个块有16个扇区，每个扇区的大小是4KB，需要换算为实际地址
    addr[0] = (sector_addr >> 16) & 0xFF;
    addr[1] = (sector_addr >> 8) & 0xFF;
    addr[2] = sector_addr & 0xFF;
    
    W25QXX_Write_Enable(1);  //擦除操作即写入0xFF，需要开启写使能
    W25QXX_Wait_Busy();        //等待写使能完成
   
     /* 使能片选 */
    W25Q_CS_CLR();    
    SPI_Transmit(&cmd, 1);
    
    SPI_Transmit(addr, 3);
    
    W25Q_CS_SET();    
    W25QXX_Wait_Busy();       //等待扇区擦除完成
}


/**
 * @brief    页写入操作
 * @param    dat —— 要写入的数据缓冲区首地址
 * @param    WriteAddr —— 要写入的地址
 * @param   byte_to_write —— 要写入的字节数（0-256）
 * @retval    none
 */
void W25QXX_Page_Program(uint8_t* dat, uint32_t WriteAddr, uint16_t nbytes)
{
    uint8_t cmd = PAGE_PROGRAM_CMD;
    uint8_t addr[3];
    addr[0] = (WriteAddr >> 16) & 0xFF;
    addr[1] = (WriteAddr >> 8) & 0xFF;
    addr[2] = WriteAddr & 0xFF;
    
    W25QXX_Write_Enable(1);
    
    /* 使能片选 */
    W25Q_CS_CLR();    
    
    SPI_Transmit(&cmd, 1);

    SPI_Transmit(addr, 3);
    
    SPI_Transmit(dat, nbytes);
    
    W25Q_CS_SET();    
    
    W25QXX_Wait_Busy();
}