#include "./beep/h_beep.h"

/* 初始化蜂鸣器 BEEP */
void beep_init(void) {
    RCC_APB2PeriphClockCmd(BEEP_GPIO_CLK, ENABLE);; //使能端口时钟
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = BEEP_PIN; //配置 pin
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO 翻转 50MHz
    GPIO_Init(BEEP_GPIO_PORT, &GPIO_InitStructure);
	BEEP_ON();
}

/* 蜂鸣器鸣叫时间，单位 ms */
void beep_on_times(int times, int delay) {
    int i;
    for (i = 0; i < times; i++) {
        BEEP_ON();
        Delay_ms(delay);
        BEEP_OFF();
        Delay_ms(delay);
    }
}
