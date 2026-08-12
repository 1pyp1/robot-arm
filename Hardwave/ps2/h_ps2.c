#include "./ps2/h_ps2.h"

/* 数据存储数组 */
u8 psx_buf[9] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* PS2 手柄初始化 */
void ps2_init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能端口时钟
    RCC_APB2PeriphClockCmd(PS2_DAT_GPIO_CLK | PS2_CMD_GPIO_CLK | PS2_CS_GPIO_CLK | PS2_CLK_GPIO_CLK, ENABLE);

    // 端口配置
    GPIO_InitStructure.GPIO_Pin = PS2_DAT_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;       // 下拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   // 50M
    GPIO_Init(PS2_DAT_GPIO_PORT, &GPIO_InitStructure);

    // 端口配置
    GPIO_InitStructure.GPIO_Pin = PS2_CMD_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   // 50M
    GPIO_Init(PS2_CMD_GPIO_PORT, &GPIO_InitStructure);

    // 端口配置
    GPIO_InitStructure.GPIO_Pin = PS2_CS_PIN;
    GPIO_Init(PS2_CS_GPIO_PORT, &GPIO_InitStructure);

    // 端口配置
    GPIO_InitStructure.GPIO_Pin = PS2_CLK_PIN;
    GPIO_Init(PS2_CLK_GPIO_PORT, &GPIO_InitStructure);
    PS2_CS(1);  // 片选拉高（不选中）
    PS2_CLK(1); // 时钟拉高
    PS2_CMD(1); // 命令线拉高
}

/* 读写 1 个字节 */
u8 ps2_transfer(unsigned char dat) {
    unsigned char rd_data, wt_data, i;
    wt_data = dat;
    rd_data = 0;
    for (i = 0; i < 8; i++) {
		// 1. 发送一位数据（从低位 LSB 开始）
        PS2_CMD((wt_data & (0x01 << i)));
		// 2. 产生时钟脉冲,PS2手柄的响应速度有限，如果延时太短手柄来不及准备数据
        PS2_CLK(1);
        Delay_us(6); // 延迟时间固定很重要
        PS2_CLK(0);
        Delay_us(6); // 延迟时间固定很重要
        PS2_CLK(1);
		// 3. 在时钟上升沿后，读取手柄返回的一位数据
        if (PS2_DAT()) {
            rd_data |= 0x01 << i;
        }
    }
    return rd_data;
}

/* 读取手柄数据 */
void ps2_write_read(void) {
    PS2_CS(0); // 选择片选
    psx_buf[0] = ps2_transfer(START_CMD);     // 发送开始命令
    psx_buf[1] = ps2_transfer(ASK_DAT_CMD);   // 发送请求数据命令
    psx_buf[2] = ps2_transfer(psx_buf[0]);	  
    psx_buf[3] = ps2_transfer(psx_buf[0]);    
    psx_buf[4] = ps2_transfer(psx_buf[0]);    
    psx_buf[5] = ps2_transfer(psx_buf[0]);    
    psx_buf[6] = ps2_transfer(psx_buf[0]);    
    psx_buf[7] = ps2_transfer(psx_buf[0]);    
    psx_buf[8] = ps2_transfer(psx_buf[0]);    
    PS2_CS(1); // 取消片选
}






