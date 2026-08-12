#ifndef __H_USART_H
#define	__H_USART_H

#include "main.h"
#define USART_DEBUG USART1//调试打印所用串口组 
#define UART_BUF_SIZE 1024 // 定义最大接收字节数 
extern u8 uart_receive_buf[UART_BUF_SIZE]; // 接收缓冲最大字节，末字节为换行符
extern uint16_t uart1_get_ok; // 接收完成标记
extern u8 uart1_mode; // 指令的模式
/*******串口开启/关闭中断*******/
#define interrupt_open() {__enable_irq();} //全局中断使能
#define uart1_open()  {USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);}
#define uart1_close() {USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);}
#define uart3_open()  {USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);}
#define uart3_close() {USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);}
/*******串口相关函数声明*******/
void Usart1_init(u32 baud); //初始化串口1
void Usart3_init(u32 baud); //初始化串口3
void Usart_SendString(USART_TypeDef* USARTx, unsigned char* str); //Usart 发送字符串
void Usart_sendInt(USART_TypeDef* USARTx, int num); //串口发送数字
void UsartPrintf(USART_TypeDef* USARTx, char* fmt, ...); //格式化打印
#endif
