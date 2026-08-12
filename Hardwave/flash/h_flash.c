#include "./flash/h_flash.h"

SPI_InitTypeDef SPI_InitStructure;
GPIO_InitTypeDef GPIO_InitStructure;

/* LED 引脚和 SPI 引脚重合，切换 SPI 和 LED 引脚状态 */
void spi_flash_on(u8 x) { //x:0 关闭 spi 引脚复用 x:1 打开
    if (x) {
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_SetBits(GPIOB, GPIO_Pin_13);
        SPI_Cmd(SPI2, ENABLE); //使能 SPI 外设
        spi_write_read(0xff); //启动传输
    } else {
        SPI_Cmd(SPI2, DISABLE); //失能 SPI 外设
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
}

/* 初始化 SPI FLASH 的 IO 口 */
void spi_flash_init(void) {
    //开启时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    //引脚端口初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; // SPI CS
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_12);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //上拉输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(GPIOB, GPIO_Pin_13 | GPIO_Pin_15); //默认电平

    //SPI 初始化
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //设置 SPI 单向或者双向的数据模式:SPI 设置为双线双向全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master; //设置 SPI 工作模式:设置为主 SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; //设置 SPI 的数据大小:SPI 发送接收 8 位帧结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High; //选择了串行时钟的稳态:时钟悬空高
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge; //数据捕获于第二个时钟沿
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; // NSS 信号由硬件（NSS 管脚）还是软件（使用 SSI 位）管理:内部 NSS 信号有 SSI 位控制
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256; //定义波特率预分频的值:波特率预分频值为 256
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //指定数据传输从 MSB 位还是 LSB 位开始:数据传输从 MSB 位开始
    SPI_InitStructure.SPI_CRCPolynomial = 7; // CRC 值计算的多项式
    SPI_Init(SPI2, &SPI_InitStructure); //根据 SPI_InitStruct 中指定的参数初始化外设 SPIx 寄存器

    SPI_Cmd(SPI2, ENABLE); //使能 SPI 外设
    spi_write_read(0xff); //启动传输
    spi_set_speed(SPI_BaudRatePrescaler_2); //设置高速
}

/* SPI 速度设置函数，SPI_BaudRatePrescaler_2 2 分频 */
void spi_set_speed(uint16_t SpeedSet) {
    SPI_InitStructure.SPI_BaudRatePrescaler = SpeedSet;
    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_Cmd(SPI1, ENABLE);
}

/* SPI 读写数据，TxData 发送的数据，SPDAT 返回字节数据 */
u8 spi_write_read(u8 TxData) {
    u8 retry = 0;				//超时计数器
    //检查指定的 SPI 标志位设置与否:发送缓存空标志位
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) {//表示发送缓冲区非空
        retry++;
        if (retry > 200) {
            return 0;
        }
    }
    SPI_I2S_SendData(SPI2, TxData); //通过外设 SPIx 发送一个数据
    retry = 0;
    //检查指定的 SPI 标志位设置与否:接受缓存非空标志位
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET) {//等待SPI2接收缓冲区收到数据
        retry++;
        if (retry > 200) {
            return 0;
        }
    }
    return SPI_I2S_ReceiveData(SPI2); //返回通过 SPIx 最近接收的数据
}

/* 读取芯片 ID，返回值为 OXEF16，表示芯片型号为 W25Q64。
PS: 0XEF13 为 W25Q80；OXEF14 为 W25Q16；OXEF15 为 W25Q32；*/
//协议规定：命令 + 24 位地址 (3 字节) + 读出 2 字节 ID
u16 spi_flash_read_id(void) {
    u16 Temp = 0;
    SPI_FLASH_CS(0); //设置片选引脚选择
    spi_write_read(W25X_ManufactDeviceID); //发送读取 ID 命令
    spi_write_read(0x00);
    spi_write_read(0x00);
    spi_write_read(0x00);
    Temp |= spi_write_read(0xFF) << 8;
    Temp |= spi_write_read(0xFF);
    SPI_FLASH_CS(1); //取消片选引脚选择
    return Temp;
}

/* 读 SPI_FLASH 的状态寄存器，默认:0x00
返回值：BIT 7 6 5 4 3 2 1 0
SPR RV TB BP2 BP1 BP0 WEL BUSY
SPR:默认 0，状态寄存器保护位，配合 WP 使用
TB, BP2, BP1, BP0:FLASH 区域写保护设置
WEL:写使能锁定
BUSY:忙标记位(1，忙;0，空闲) */
u8 spi_flash_read_SR(void) {
    u8 byte = 0;
    SPI_FLASH_CS(0); //使能器件
    spi_write_read(W25X_ReadStatusReg); //发送读取状态寄存器命令
    byte = spi_write_read(0Xff); //读取一个字节
    SPI_FLASH_CS(1); //取消片选
    return byte;
}

/* 写 SPI_FLASH 的状态寄存器，参数 SPR RV TB BP2 BP1 BP0 WEL BUSY
只有 SPR, TB, BP2, BP1, BP0(bit 7, 5, 4, 3, 2)可以写 */
void spi_flash_write_SR(u8 byte) {
    SPI_FLASH_CS(0); //使能器件
    spi_write_read(W25X_WriteStatusReg); //发送写取状态寄存器命令
    spi_write_read(byte); //写入一个字节
    SPI_FLASH_CS(1); //取消片选
}

/* SPI_FLASH 写使能，WEL 置位 */
void spi_flash_write_enable(void) {
    SPI_FLASH_CS(0); //使能器件
    spi_write_read(W25X_WriteEnable); //发送写使能
    SPI_FLASH_CS(1); //取消片选
}

/* SPI_FLASH 写禁止，将 WEL 清零 */
void spi_flash_write_disable(void) {
    SPI_FLASH_CS(0); //使能器件
    spi_write_read(W25X_WriteDisable); //发送写禁止指令
    SPI_FLASH_CS(1); //取消片选
}

/* SPI 读取 1 个字节的数据，参数 ReadAddr 为开始读取的地址(24bit) */
char spi_flash_read_char(u32 readAddr) {
    static char tmp;
    SPI_FLASH_CS(0); //使能器件
    spi_write_read(W25X_ReadData); //发送读取命令
    spi_write_read((u8)((readAddr) >> 16)); //发送 24bit 地址
    spi_write_read((u8)((readAddr) >> 8));
    spi_write_read((u8)(readAddr & 0XFF));
    tmp = spi_write_read(0XFF); //读数
    SPI_FLASH_CS(1);
    return tmp;
}

/* 在指定地址开始读取指定长度的数据
参数 pBuffer 数据存储区，ReadAddr 开始读取的地址(24bit)，NumByteToRead 要读取的字节数(最大 65535) */
void spi_flash_read(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead) {
    u16 i;
    SPI_FLASH_CS(0);
    spi_write_read(W25X_ReadData); //使能器件
    spi_write_read((u8)((ReadAddr) >> 16)); //发送读取命令
    spi_write_read((u8)((ReadAddr) >> 8)); //发送 24bit 地址
    spi_write_read((u8)(ReadAddr & 0XFF));
    for (i = 0; i < NumByteToRead; i++) {
        pBuffer[i] = spi_write_read(0XFF); //循环读数
    }
    SPI_FLASH_CS(1);
}

/* SPI 写入 1 个字节的数据，参数 tmp 数据，WriteAddr 开始写入的地址(24bit) */
void spi_flash_write_char(char tmp, u32 WriteAddr) {
    spi_flash_write_enable();
    SPI_FLASH_CS(0); //使能器件
    spi_write_read(W25X_PageProgram); //发送写页命令
    spi_write_read((u8)((WriteAddr) >> 16)); //发送 24bit 地址
    spi_write_read((u8)((WriteAddr) >> 8));
    spi_write_read((u8)WriteAddr);
    spi_write_read(tmp); //写数
    SPI_FLASH_CS(1); //取消片选
    spi_flash_wait_busy(); //等待写入结束
    // UsartPrintf(USART_DEBUG, "Byte has been written.\r\n");
}

/* 在指定地址开始写入指定长度的数据，实现跨扇区、不定长数据的智能写入
   函数参数: pBuffer-数据存储区, WriteAddr-开始写入的地址 (24bit), NumByteToWrite-要写入的字节数 (最大 65535) */
u8 W25Q_BUF[4096];
void SpiFlashWriteS(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite) {
    u32 secpos;
    u16 secoff;
    u16 secremain;
    u16 i;
    secpos = WriteAddr / 4096; // 扇区地址 0~511 for w25x16
    secoff = WriteAddr % 4096; // 在扇区...内的偏移
    secremain = 4096 - secoff; // 扇区剩余空间大小
    if (NumByteToWrite <= secremain) {
        secremain = NumByteToWrite; // 不大于 4096 个字节
    }
    while (1) {
        spi_flash_read(W25Q_BUF, secpos * 4096, 4096); // 读出整个扇区的内容
        for (i = 0; i < secremain; i++) { // 校验数据
            if (W25Q_BUF[secoff + i] != 0XFF) {
                break; // 需要擦除
            }
        }
        if (i < secremain) { // 需要擦除
            spi_flash_erase_sector(secpos); // 擦除这个扇区
            for (i = 0; i < secremain; i++) { // 复制
                W25Q_BUF[i + secoff] = pBuffer[i];
            }
            spi_flash_write_no_check(W25Q_BUF, secpos * 4096, 4096); // 写入整个扇区
        } else {
            spi_flash_write_no_check(pBuffer, WriteAddr, secremain); // 写已经擦除了的，直接写入扇区剩余区间
        }
        if (NumByteToWrite == secremain) {
            break; // 写入结束了
        } else { // 写入未结束
            secpos++; // 扇区地址增 1
            secoff = 0; // 偏移位置为 0
            pBuffer += secremain; // 指针偏移
            WriteAddr += secremain; // 写地址偏移
            NumByteToWrite -= secremain; // 字节数递减
            if (NumByteToWrite > 4096) {
                secremain = 4096; // 下一个扇区还是写不完
            } else {
                secremain = NumByteToWrite; //下一个扇区可以写完了
            }
        }
    }
}
/* 
 * 在指定地址开始写入指定长度的数据 
 * 函数参数: pBuffer-数据存储区, WriteAddr-开始写入的地址(24bit), NumByteToWrite-要写入的字节数(最大65535) 
 */
void spi_flash_write(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite) {
    u8 SpiFlash_BUF[4096];
    u32 sector = 0;
    u16 sectorremain;
    u16 i;

    sectorremain = 4096 - WriteAddr % 4096; // 单扇区剩余的字节数

    if (NumByteToWrite <= sectorremain) {
        sectorremain = NumByteToWrite;
    }

    while (1) {
        sector = WriteAddr >> 12; // 扇区地址
        spi_flash_read(SpiFlash_BUF, sector * 4096, 4096); // 读取整个扇区到缓冲区

        for (i = 0; i < sectorremain; i++) { // 校验数据
            if (SpiFlash_BUF[WriteAddr % 4096 + i] != 0XFF) { // 需要擦除
                break;
            }
        }

        if (i < sectorremain) { // 需要擦除
            spi_flash_erase_sector(sector); // 擦除这个扇区
            for (i = 0; i < sectorremain; i++) { // 复制
                SpiFlash_BUF[WriteAddr % 4096 + i] = pBuffer[i];
            }
            spi_flash_write_sector(SpiFlash_BUF, sector * 4096, 4096); // 写入整个扇区
        } else {
            spi_flash_write_sector(pBuffer, WriteAddr, sectorremain);
        }

        if (NumByteToWrite == sectorremain) {
            break; // 写入结束了
        } else {
            pBuffer += sectorremain;
            WriteAddr += sectorremain;
            NumByteToWrite -= sectorremain; // 减去已经写入了的字节数
            if (NumByteToWrite > 4096) {
                sectorremain = 4096; // 一次可以写入4096个字节
            } else {
                sectorremain = NumByteToWrite; // 不够4096个字节了
            }
        }
    }
}

/* 
 * SPI在一页内写入少于256个字节的数据 
 * 函数参数: pBuffer-数据存储区, WriteAddr-开始写入的地址(24bit), NumByteToWrite-要写入的字节数(最大256) 
 */
void spi_flash_write_page(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite) {
    u16 i, pageremain;
    
    pageremain = 256 - WriteAddr % 256; // 单页剩余的字节数
    if (NumByteToWrite > pageremain) {
        NumByteToWrite = pageremain;
        // UsartPrintf(USART_DEBUG, "There are not enough writable bytes left in this page!\r\n");
    }

    spi_flash_write_enable();             // 写使能
    SPI_FLASH_CS(0);                      // 使能器件
    spi_write_read(W25X_PageProgram);     // 发送写页命令
    spi_write_read((u8)((WriteAddr) >> 16)); // 发送24bit地址
    spi_write_read((u8)((WriteAddr) >> 8));
    spi_write_read((u8)WriteAddr);

    for (i = 0; i < NumByteToWrite; i++) {
        spi_write_read(pBuffer[i]);       // 循环写数
    }

    SPI_FLASH_CS(1);                      // 取消片选
    spi_flash_wait_busy();                // 等待写入结束
    // UsartPrintf(USART_DEBUG, "%s", NumByteToWrite);
    // UsartPrintf(USART_DEBUG, " bytes have been written. \r\n");
}

/* 
 * SPI在一扇区内写入少于4096个字节的数据 
 * 函数参数: pBuffer-数据存储区, WriteAddr-开始写入的地址(24bit), NumByteToWrite-要写入的字节数(最大4096) 
 */
void spi_flash_write_sector(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite) {
    u16 sectorremain;
    
    sectorremain = 4096 - WriteAddr % 4096; // 单扇区剩余的字节数
    if (NumByteToWrite > sectorremain) {
        NumByteToWrite = sectorremain;
        // UsartPrintf(USART_DEBUG, "There are not enough writable bytes left in this sector!\r\n");
    }
    
    spi_flash_write_no_check(pBuffer, WriteAddr, NumByteToWrite);
}

/* 
 * 无检验写SPI_FLASH 
 * 函数参数: pBuffer-数据存储区, WriteAddr-开始写入的地址(24bit), NumByteToWrite-要写入的字节数(最大65535 1块 16扇区 256页) 
 */
void spi_flash_write_no_check(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite) {
    u16 pageremain;
    
    pageremain = 256 - WriteAddr % 256; // 单页剩余的字节数
    if (NumByteToWrite <= pageremain) {
        pageremain = NumByteToWrite;
    }

    while (1) {
        spi_flash_write_page(pBuffer, WriteAddr, pageremain);
        if (NumByteToWrite == pageremain) {//如果剩余要写的长度等于本次写入的长度
            break; // 写入结束了
        } else {
			//更新指针和计数器，准备写入下一页
            pBuffer += pageremain;
            WriteAddr += pageremain;
            NumByteToWrite -= pageremain; // 减去已经写入了的字节数
            if (NumByteToWrite > 256) {
                pageremain = 256; // 一次可以写入256个字节
            } else {
                pageremain = NumByteToWrite; //如果剩余数据不足256字节，下一次就写入剩余的全部数据。
            }
        }
    }
}

/* 擦除一个扇区 最少150毫秒, 参数Dst_Addr -扇区地址 (最大2047) */
void spi_flash_erase_sector(u32 Dst_Addr) {
    Dst_Addr <<= 12;
    spi_flash_write_enable(); // SET WEL
    spi_flash_wait_busy();
    SPI_FLASH_CS(0);          // 使能器件
    spi_write_read(W25X_SectorErase); // 发送扇区擦除指令
    spi_write_read((u8)((Dst_Addr) >> 16)); // 发送24bit地址
    spi_write_read((u8)((Dst_Addr) >> 8));
    spi_write_read((u8)Dst_Addr);
    SPI_FLASH_CS(1);          // 取消片选
    spi_flash_wait_busy();    // 等待擦除完成
    // UsartPrintf(USART_DEBUG, "Sector has been erased. \r\n");
}

/* 擦除整个芯片 */
void spi_flash_erase_chip(void) {
    spi_flash_write_enable(); // SET WEL
    spi_flash_wait_busy();
    SPI_FLASH_CS(0);          // 使能器件
    spi_write_read(W25X_ChipErase); // 发送芯片擦除命令
    SPI_FLASH_CS(1);          // 取消片选
    // 等待芯片擦除结束
    spi_flash_wait_busy();
    // UsartPrintf(USART_DEBUG, "Chip has been erased. \r\n");
}

/* 等待空闲 */
void spi_flash_wait_busy(void) {
    /* 等待BUSY位清空 */
    while ((spi_flash_read_SR() & 0x01) == 0x01);
}

/* 进入掉电模式 */
void spi_flash_power_down(void) {
    SPI_FLASH_CS(0);              // 使能器件
    spi_write_read(W25X_PowerDown); // 发送掉电命令
    SPI_FLASH_CS(1);              // 取消片选
    Delay_ms(1);                  // us 1 等待TPD
}

/* 唤醒 */
void spi_flash_wake_up(void) {
    SPI_FLASH_CS(0);
    spi_write_read(W25X_ReleasePowerDown); // 发送唤醒指令
    SPI_FLASH_CS(1);                       // 取消片选
	Delay_ms(1); //us 3
}

