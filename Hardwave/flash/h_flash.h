#ifndef __H_FLASH_H__
#define __H_FLASH_H__

#include "main.h"

/* SPI 片选引脚选择 */
#define SPI_FLASH_CS(x) GPIO_WriteBit(GPIOB, GPIO_Pin_12, (BitAction)x)

/**********W25Q 系列芯片 ID**********/
#define W25Q80 0XEF13
#define W25Q16 0XEF14
#define W25Q32 0XEF15
#define W25Q64 0XEF16

/**********W25Q64 芯片变量宏定义**********/
#define W25Q64_SECTOR_SIZE 4096 // 4K
#define W25Q64_SECTOR_NUM 2048 // 8*1024/4 = 2048

/**********W25Q64 芯片地址存储表**********/
#define FLASH_ASC16_ADDRESS 0
#define FLASH_HZK16_ADDRESS 0x1000
#define FLASH_SYSTEM_CONFIG_ADDRESS 0x43000

#define FLASH_BITMAP1_SIZE_ADDRESS 0x50000
#define FLASH_BITMAP2_SIZE_ADDRESS FLASH_BITMAP1_SIZE_ADDRESS + 0x28000
#define FLASH_BITMAP3_SIZE_ADDRESS FLASH_BITMAP2_SIZE_ADDRESS + 0x28000
#define FLASH_BITMAP4_SIZE_ADDRESS FLASH_BITMAP3_SIZE_ADDRESS + 0x28000
#define FLASH_BITMAP5_SIZE_ADDRESS FLASH_BITMAP4_SIZE_ADDRESS + 0x28000
#define FLASH_BITMAP6_SIZE_ADDRESS FLASH_BITMAP5_SIZE_ADDRESS + 0x28000

#define FLASH_BITMAPMAIN_SIZE_ADDRESS FLASH_BITMAP6_SIZE_ADDRESS + 0x28000
#define FLASH_BITMAPDS1302_SIZE_ADDRESS FLASH_BITMAPMAIN_SIZE_ADDRESS + 0x28000
#define FLASH_BITMAPPDS18B20_SIZE_ADDRESS FLASH_BITMAPDS1302_SIZE_ADDRESS + 0x28000
#define FLASH_BITMAPBLUETOOTH_SIZE_ADDRESS FLASH_BITMAPPDS18B20_SIZE_ADDRESS + 0x28000

/**********FLASH 相关指令表**********/
#define W25X_WriteEnable 0x06 //写使能指令
#define W25X_WriteDisable 0x04 //写失能指令
#define W25X_ReadStatusReg 0x05 //读取寄存器状态指令
#define W25X_WriteStatusReg 0x01 //写入寄存器状态指令
#define W25X_ReadData 0x03 //读取指令，发送完后马上可以传输
#define W25X_FastReadData 0x0B //快速读取指令，发送完后再发一字节 dummy，之后才连续传输
#define W25X_FastReadDual 0x3B //双输出快速读取指令，允许同时从两个不同的内存区域读取
#define W25X_PageProgram 0x02 //写页命令
#define W25X_SectorErase 0x20 //扇区擦除指令
#define W25X_BlockErase 0xD8 //块擦除之灵感
#define W25X_ChipErase 0xC7 //芯片擦除命令
#define W25X_PowerDown 0xB9 //掉电命令
#define W25X_ReleasePowerDown 0xAB //唤醒指令
#define W25X_DeviceID 0xAB //获取设备 ID 指令
#define W25X_ManufactDeviceID 0x90 //读取芯片 ID 指令
#define W25X_JedecDeviceID 0x9F //读取厂商 ID 和设备 ID

void spi_flash_on(u8 x); // LED 引脚和 SPI 引脚重合，切换 SPI 和 LED 引脚状态
void spi_flash_init(void); //初始化 SPI FLASH 的 IO 口
void spi_set_speed(uint16_t SpeedSet); // SPI 速度设置函数
u8 spi_write_read(u8 TxData); // SPI 读写数据

u16 spi_flash_read_id(void); //读取芯片 ID W25X16 的 ID:0XEF14
u8 spi_flash_read_SR(void); //读 SPI_FLASH 的状态寄存器
void spi_flash_write_SR(u8 byte); //写 SPI_FLASH 的状态寄存器
void spi_flash_write_enable(void); // SPI_FLASH 写使能，WEL 置位
void spi_flash_write_disable(void); // SPI_FLASH 写禁止，将 WEL 清零
char spi_flash_read_char(u32 readAddr); // SPI 读取 1 个字节的数据
void spi_flash_write_char(char tmp, u32 WriteAddr); // SPI 写入 1 个字节的数据

void spi_flash_read(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead); //在指定地址开始读取指定长度的数据
void spi_flash_write(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite); //在指定地址开始写入指定长度的数据
void spi_flash_write_page(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite); // SPI 在一页内写入少于 256 个字节的数据
void spi_flash_write_sector(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite); // SPI 在一扇区内写入少于 4096 个字节的数据
void spi_flash_write_no_check(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite); //无检验写 SPI_FLASH
void spi_flash_erase_sector(u32 Dst_Addr); //擦除一个扇区 最少 150 毫秒
void spi_flash_erase_chip(void); //擦除整个芯片
void spi_flash_wait_busy(void); //等待空闲
void spi_flash_power_down(void); //进入掉电模式
void spi_flash_wake_up(void); //唤醒
void SpiFlashWriteS(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);

#define spiFlashOn(x) spi_flash_on(x)
#define w25x_init() spi_flash_init()
#define w25x_readId() spi_flash_read_id()
#define w25x_read(buf, addr, len) spi_flash_read(buf, addr, len)
#define w25x_write(buf, addr, len) spi_flash_write_no_check(buf, addr, len)
#define w25x_writeS(buf, addr, len) SpiFlashWriteS(buf, addr, len)
#define w25x_erase_sector(addr) spi_flash_erase_sector(addr)
#define w25x_wait_busy() spi_flash_wait_busy()
#endif
