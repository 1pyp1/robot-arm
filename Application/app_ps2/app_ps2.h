#ifndef _APP_PS2_H_
#define _APP_PS2_H_

#include "main.h"

#define CMD_RETURN_SIZE 1024
#define PS2_LED_RED 0x73    // PS2 手柄红灯模式
#define PS2_LED_GRN 0x41    // PS2 手柄绿灯模式
#define PSX_BUTTON_NUM 16   // 手柄按键数目
#define PSX_MAX_LEN 64      // 手柄命令最大字节数

void app_ps2_init(void);            // PS2 设备控制初始化
void app_ps2_run(void);             // 循环执行任务
void parse_psx_buf(unsigned char* buf, unsigned char mode); // 处理手柄按键字符

#endif
