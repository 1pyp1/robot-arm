#ifndef __H_BEEP_H
#define	__H_BEEP_H

#include "main.h"
#define BEEP_PIN GPIO_Pin_5
#define BEEP_GPIO_PORT GPIOB
#define BEEP_GPIO_CLK RCC_APB2Periph_GPIOB //GPIO 端口时钟

/* 控制 BEEP 的宏 */
#define BEEP_GET_LEVEL() GPIO_ReadOutputDataBit(BEEP_GPIO_PORT, BEEP_PIN)//读取 BEEP 状态
#define BEEP_OFF() GPIO_ResetBits(BEEP_GPIO_PORT, BEEP_PIN) // BEEP 蜂鸣器关闭
#define BEEP_ON() GPIO_SetBits(BEEP_GPIO_PORT, BEEP_PIN) // BEEP 蜂鸣器打开
#define BEEP_TOGGLE() GPIO_WriteBit(BEEP_GPIO_PORT, BEEP_PIN, (BitAction)(1 - BEEP_GET_LEVEL())) // 翻转 BEEP 蜂鸣器
#define beep_on() BEEP_ON()
#define beep_off() BEEP_OFF()

/*******BEEP 相关函数声明*******/
void beep_init(void); // 初始化 BEEP 蜂鸣器
void beep_on_times(int times, int delay);
#endif
