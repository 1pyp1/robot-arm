#include "./delay/h_delay.h"
void Delay_ns(u32 xns){
	while(xns--);
	return;
}
void Delay_us(u32 xus){
	u32 i=0;
	while(xus--){
		i=10;
		while(i--);
	}
}
void Delay_ms(u32 xms){
	u32 systick_temp=millis();				//获取当前滴答时钟数值，用作比较值
	//SysTick计数超过设定值结束循环
	while(xms>(millis()-systick_temp)){//时间戳变量溢出错误
		/*方式延时的时间超过溢出值，假设要延时 1000ms，进入函数时 systick_temp = 0xFFFFFE00
		溢出前经过: 0xFFFFFFFF - 0xFFFFFE00 = 0x1FF = 511ms 执行后 xms = 1000 - 511 = 489
		同时将 systick_temp = 0，下一轮循环变成：等待 millis() - 0 >= 489，即等到 millis() 走到489
		*/
		if(systick_temp>millis()){
		xms=xms-(0xffffffff-systick_temp);
		systick_temp=0;
		}
		
	}
}
void Delay_s(u32 xs){
	while(xs--){
		Delay_ms(1000);
	}
}
