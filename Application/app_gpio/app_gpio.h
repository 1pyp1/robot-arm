#ifndef _APP_GPIO_H_
#define	_APP_GPIO_H_
#include "main.h"

void app_gpio_init(void); /* GPIO初始化 */
void app_led_run(void); /* LED闪烁运行 */
void app_setup_start(void);//应用程序启动
void app_key_run(void);//key按键运行任务
#endif
