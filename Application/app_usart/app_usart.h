#ifndef __APP_USART_H__
#define __APP_USART_H__

#include "main.h"

void app_usart_init(void); //uart 串口相关设备控制初始化
void app_usart_run(void); //循环检测串口接收到的指令
//void app_uart_init(void);
//void app_uart_run(void); 
#endif
