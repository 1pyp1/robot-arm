#include "./usart/h_usart.h"

u8 uart_receive_buf[UART_BUF_SIZE];
uint16_t uart1_get_ok;
u8 uart1_mode;

/* 初始化串口 1 */
void Usart1_init(u32 baud) {
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 使能端口时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
    //USART_DeInit (USART1);//串口默认初始化

    //引脚初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //串口配置
    USART_InitStructure.USART_BaudRate = baud;//串口波特率 
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//字长为8位数据格式
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式
    USART_Init(USART1, &USART_InitStructure);
    //NVIC中断
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);
    //设置接受和发送中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // 开启串口接受中断
    USART_ITConfig(USART1, USART_IT_TXE, DISABLE); // 禁止串口发送中断
    USART_Cmd(USART1, ENABLE); // 使能串口 1
}

/* 初始化串口 3 */
void Usart3_init(u32 baud) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    USART_ClockInitTypeDef USART_ClockInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 使能端口时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    //引脚初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;// 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;// 浮空输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //初始化 USART 模块的外围时钟,关闭所有同步时钟相关的引脚输出
    USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
    USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
    USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;//设置时钟相位为第二个边沿捕获数据
    USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;//禁止在最后一个数据位输出时钟脉冲
    USART_ClockInit(USART3, &USART_ClockInitStructure);

    //串口配置
    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    //NVIC中断
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //设置接受和发送中断
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); // 开启串口接受中断
    USART_ITConfig(USART3, USART_IT_TXE, DISABLE); // 禁止串口发送中断
    USART_Cmd(USART3, ENABLE);// 使能串口 3
}

/* 发送字节 */
void uart_send_byte(USART_TypeDef* USARTx, u8 data) {
    USART_SendData(USARTx, data);
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);//等待发送数据寄存器为空
}

/* 串口数据发送，USARTx 为串口组 */
void Usart_SendString(USART_TypeDef* USARTx, unsigned char* str) {
    while (*str) {
        uart_send_byte(USARTx, *str++);
    }
}

/* 串口发送数字 */
void Usart_sendInt(USART_TypeDef* USARTx, int num) {
    static u8 str[20];
    sprintf((char*)str, "%d", num);
    Usart_SendString(USARTx, str);
}

/* 格式化打印 */
void UsartPrintf(USART_TypeDef* USARTx, char* fmt, ...) {
    unsigned char UsartPrintfBuf[296];
    va_list ap;											//指针遍历可变参数
    unsigned char* pStr = UsartPrintfBuf;				//定义一个指针 pStr，让它指向缓冲区的第一个字符
    va_start(ap, fmt);									//让指针指向fmt之后的第一个可变参数
    vsnprintf((char*)UsartPrintfBuf, sizeof(UsartPrintfBuf), fmt, ap);//格式化
    va_end(ap);
    while (*pStr != 0) {
        USART_SendData(USARTx, *pStr++);
        while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
    }
}

/* 重定义 fputc 函数, 写这个函数可以使用 printf, 记得开启 Use MicroLIB */
int fputc(int ch, FILE* f) {
    while ((USART1->SR & 0X40) == 0)
        ; // 循环发送,直到发送完毕
    USART1->DR = (u8)ch;
    return ch;
}

/* 串口 1 中断服务程序 */
void USART1_IRQHandler(void) { /* 最后数据发送\r\n 结束 */
    u8 sbuf_bak;
    static u16 buf_index = 0;

    if (USART_GetFlagStatus(USART1, USART_IT_RXNE) == SET) {	//接收到了新数据
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        sbuf_bak = USART_ReceiveData(USART1);
        //UsartPrintf (USART_DEBUG, "uart1_model cmd\r\n");
        //USART_SendData (USART1, sbuf_bak);
        if (uart1_get_ok) {						//上一次的数据接收到未处理完
            return;
        }

        if (sbuf_bak == '<') {						
            uart1_mode = 4;
            buf_index = 0;
        } else if (uart1_mode == 0) {				//正在寻找帧头
            if (sbuf_bak == '$') {
                // 命令模式 $XXX!
                uart1_mode = 1;
            } else if (sbuf_bak == '#') {
                // 单舵机模式#000P1500T1000! 类似这种命令
                uart1_mode = 2;
            } else if (sbuf_bak == '{') {
                // 多舵机模式 {#000P1500T1000!#001P1500T1000!} 多个单舵机命令用大括号括起来
                uart1_mode = 3;
            } else if (sbuf_bak == '<') {
                // 保存动作组模式 <G0000#000P1500T1000!#001P1500T1000!B000!> 用尖括号括起来 带有组序号
                uart1_mode = 4;
            }
            buf_index = 0;
        }

        uart_receive_buf[buf_index++] = sbuf_bak;

        if ((uart1_mode == 4) && (sbuf_bak == '>')) {		//保存动作组指令结束
            uart_receive_buf[buf_index] = '\0';
            uart1_get_ok = 1;
        } else if ((uart1_mode == 1) && (sbuf_bak == '!')) {
            uart_receive_buf[buf_index] = '\0';
            uart1_get_ok = 1;
        } else if ((uart1_mode == 2) && (sbuf_bak == '!')) {
            uart_receive_buf[buf_index] = '\0';
            uart1_get_ok = 1;
        } else if ((uart1_mode == 3) && (sbuf_bak == '}')) {
            uart_receive_buf[buf_index] = '\0';
            uart1_get_ok = 1;
        }

        if (buf_index >= UART_BUF_SIZE) {
            buf_index = 0;
        }
    }
}

/* 串口 3 中断函数 */
void USART3_IRQHandler(void) {
    u8 sbuf_bak;
    static u16 buf_index = 0;

    if (USART_GetFlagStatus(USART3, USART_IT_RXNE) == SET) {
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
        sbuf_bak = USART_ReceiveData(USART3);
        //printf("uart2_model 命令模式\r\n");
        USART_SendData(USART1, sbuf_bak); // 透传或者调试用
        if (uart1_get_ok) {
            return;
        }

        if (sbuf_bak == '<') {
            uart1_mode = 4;
            buf_index = 0;
        } else if (uart1_mode == 0) {
            if (sbuf_bak == '$') {
                // 命令模式 $XXX!
                uart1_mode = 1;
            } else if (sbuf_bak == '#') {
                // 单舵机模式#000P1500T1000! 类似这种命令
                uart1_mode = 2;
            } else if (sbuf_bak == '{') {
                // 多舵机模式 (#000P1500T1000!#001P1500T1000!) 多个单舵机命令用大括号括起来
                uart1_mode = 3;
            } else if (sbuf_bak == '<') {
                // 保存动作组模式 <G0000#000P1500T1000!#001P1500T1000!B000!> 用尖括号括起来 带有组序号
                uart1_mode = 4;
            }
            buf_index = 0;
        }

        uart_receive_buf[buf_index++] = sbuf_bak;

        if ((uart1_mode == 4) && (sbuf_bak == '>')) {
            uart_receive_buf[buf_index] = '\0';
            uart1_get_ok = 1;
        } else if ((uart1_mode == 1) && (sbuf_bak == '!')) {
            uart_receive_buf[buf_index] = '\0';
            uart1_get_ok = 1;
        } else if ((uart1_mode == 2) && (sbuf_bak == '!')) {
            uart_receive_buf[buf_index] = '\0';
            uart1_get_ok = 1;
        } else if ((uart1_mode == 3) && (sbuf_bak == '}')) {
            uart_receive_buf[buf_index] = '\0';
            uart1_get_ok = 1;
        }

        if (buf_index >= UART_BUF_SIZE) {
            buf_index = 0;
        }
    }
}

