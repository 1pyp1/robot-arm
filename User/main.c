#include "main.h"
int main(void){
	SWJ_gpio_init();//禁用(JTAG-DP+SW-DP)
	rcc_init();										//配置系统时钟，启动外部高速时钟(HSE)
	SysTick_Init();
	app_gpio_init();
	app_setup_start();
	app_usart_init();
	spi_flash_init(); /* 初始化SPI FLASH的IO口 */
    u16 num = spi_flash_read_id();
    UsartPrintf(USART_DEBUG, "%x", num);
	servo_init();		//舵机初始化
	TIM2_init(20000,72-1);//定时器控制pwm初始化
	app_ps2_init();//初始化手柄
	while(1){
		app_led_run();	//led电平翻转
		app_key_run();//循环执行工作按键
		app_usart_run();//串口运行
		loop_action();//动作组执行
		app_ps2_run();		//处理手柄数据
		
	}
}
void SWJ_gpio_init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO,ENABLE);//使能PA、PB端口时钟
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable,ENABLE);//使能禁用JTAG-DP+SW-DP
}
