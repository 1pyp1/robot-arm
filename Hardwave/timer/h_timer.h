#ifndef __H_TIMER_H
#define	__H_TIMER_H
//系统文件
#include "main.h"
void SysTick_Init(void);
u32 millis(void);
void TIM2_init(u16 arr,u16 psc);
#endif
