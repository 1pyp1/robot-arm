#include "./app_usart/app_usart.h"
//static void parse_action(void);
/* usart 串口相关设备控制初始化 */
void app_usart_init(void) {
    Usart1_init(115200); /* usart1 串口初始化 */
    Usart3_init(115200); /* 连接总线设备串口 */
    UsartPrintf(USART_DEBUG, "uartl3_init succeed\r\n");
}
///* 循环检测串口接收到的指令 */
//void app_usart_run(void) {
//    if (uart1_get_ok) {
//        UsartPrintf(USART_DEBUG, "%s\r\n", uart_receive_buf);//回传数据
//        if (uart1_mode == 1) {
//            // 命令模式
//            UsartPrintf(USART_DEBUG, "uartl_model 命令模式\r\n");
//        } else if (uart1_mode == 2) {
//            // 单个舵机调试
//            UsartPrintf(USART_DEBUG, "uartl_mode2 单个舵机调试\r\n");
//        } else if (uart1_mode == 3) {
//            // 多路舵机调试
//            UsartPrintf(USART_DEBUG, "uartl_mode3 多路舵机调试\r\n");
//        } else if (uart1_mode == 4) {
//            // 存储模式
//            UsartPrintf(USART_DEBUG, "uartl_mode3 存储模式\r\n");
//        }
//        uart1_mode = 0;
//        uart1_get_ok = 0;
//    }
//}
/* 循环检测串口接收到的指令 */
//void app_usart_run(void) {
//    if (uart1_get_ok) { // 接收到上位机指令
//        uart1_get_ok = 0;
//        uart1_mode = 0;
//        parse_action(); // 解析指令
//    }
//}
/* 处理 #000P1500T1000! 类似的字符串 */
//static void parse_action(void) {
//    uint16_t time, i = 0;
//    u16 index;
//    float pwm;
//    printf("parse_action = %s\r\n", uart_receive_buf);
//	while(uart_receive_buf[i]){
//		if (uart_receive_buf[i] != '#') { // 指令的开始，先找出下标
//			index=0;
//			i++;
//		while (uart_receive_buf[i] && uart_receive_buf[i] != 'P') { // 检测到P，开始计算舵机执行目标
//			index = index * 10 + uart_receive_buf[i] - '0';
//			i++;
//		}
//	}
//    else if (uart_receive_buf[i] == 'P') {
//        pwm = 0;
//        i++;
//        while (uart_receive_buf[i] && uart_receive_buf[i] != 'T') {
//            pwm = pwm * 10 + uart_receive_buf[i] - '0';
//            i++;
//        }
//    }

//    else if (uart_receive_buf[i] == 'T') { // 检测到T，舵机执行时间
//        time = 0;
//        i++;
//        while (uart_receive_buf[i] && uart_receive_buf[i] != '!') {
//            time = time * 10 + uart_receive_buf[i] - '0';
//            i++;
//        }
//		duoji_doing_set(index, pwm, time); // 设置舵机控制参数函数
//		if (index == 255) {
//			for (index = 0; index < DJ_NUM; index++) {
//				duoji_doing_set(index, pwm, time); // 设置舵机控制参数函数
//			}
//		}
//    }
//    else {
//		i++;
//	}              
//  }        
//}
void app_usart_run(void) {
    if (uart1_get_ok) {
        if (uart1_mode == 1) {
            // 命令模式
            parse_cmd(uart_receive_buf);
        } else if (uart1_mode == 2) {
            // 单个舵机调试
            parse_action(uart_receive_buf);
        } else if (uart1_mode == 3) {
            // 多路舵机调试
            parse_action(uart_receive_buf);
        } else if (uart1_mode == 4) {
            // 存储模式
            save_action(uart_receive_buf);
        }
        uart1_mode = 0;
        uart1_get_ok = 0;
    }
}

