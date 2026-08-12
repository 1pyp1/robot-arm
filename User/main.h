#ifndef __MAIN_H
#define	__MAIN_H
//标准库文件
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
//系统文件
#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#include "./rcc/h_rcc.h"
#include "./timer/h_timer.h"
#include "./delay/h_delay.h"
#include "./led/h_led.h"
#include "./app_gpio/app_gpio.h"
#include "./beep/h_beep.h"
#include "./key/h_key.h"
#include "./usart/h_usart.h"
#include "./app_usart/app_usart.h"
#include "./flash/h_flash.h"
#include "./servo/h_servo.h"
#include "./app_servo/app_servo.h"
#include "./ps2/h_ps2.h"
#include "./app_ps2/app_ps2.h"
void SWJ_gpio_init(void);
#endif
