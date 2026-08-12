#include "./app_gpio/app_gpio.h"
void app_gpio_init(void) {
    led_init();  //led初始化
	beep_init(); //蜂鸣器初始化
	key_init();  //按键初始化
	LED_OFF();
	BEEP_OFF();
}
void app_led_run(void){
	static u32 time_count =0;
	if(millis()-time_count<1000){
		return ;
	}
	time_count=millis();
	LED_TOGGLE();
}
void app_setup_start(void){	//LED和蜂鸣器闪烁响三次表示系统启动
	u8 i;
	for(i=0;i<3;i++){
		BEEP_ON();
		LED_ON();
		Delay_ms(100);
		BEEP_OFF();
		LED_OFF();
		Delay_ms(100);
	}
}
/* key 按键运行任务 */
void app_key_run(void) {
    uint8_t a, b;
    
    if (key1_pressing != 0) { /* 按键1被按下 */
        if (key1_pressing == 1) {
            spi_flash_on(1); /* 打开spi引脚复用，关闭led引脚输出 */
            a = 0;
            b = 0;
            a = spi_flash_read_char(0);
            a++;
            b++;
            printf("a = %d b = %d\r\n", a, b);
            spi_flash_erase_sector(0);
            spi_flash_write_char(a, 0);
            key1_pressing = 0;
            printf("key1_pressing = 1\r\n");
        } else if (key1_pressing == 2) {
            printf("key1_pressing = 2\r\n");
        }
    }

    if (key2_pressing != 0) { /* 按键2被按下 */
        if (key2_pressing == 1) {
            spi_flash_on(0); /* 关闭spi引脚复用，打开led引脚输出 */
            key2_pressing = 0;
            printf("key2_pressing = 1\r\n");
        } else if (key2_pressing == 2) {
            printf("key2_pressing = 2\r\n");
        }
    }
}
