#include "./timer/h_timer.h"
static u32 systick_ms = 0;  // 记录时间
/* 初始化SysTick */
void SysTick_Init(void) {  // 1毫秒@72MHz
    // 初始化并启动SysTick计数器及其中断
    SysTick_Config(SystemCoreClock / 1000);  // 1ms需要72000次自减
}

/* SysTick中断 */
void SysTick_Handler(void) {
    systick_ms++;
	// 每隔10ms执行按键扫描
    static uint32_t key_scan_times = 0;
    key_scan_times++;
    if (key_scan_times >= 10) {
        key_scan_times = 0;
		key_scan();
    }
}

    
u32 millis(void){
	return systick_ms;
}

void TIM2_init(u16 arr, u16 psc) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 时钟 TIM2 使能
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // 定时器 TIM2 初始化
    TIM_TimeBaseStructure.TIM_Period = arr;                 // 设置自动重装载寄存器周期的值
    TIM_TimeBaseStructure.TIM_Prescaler = psc;               // 设置时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  // 输入捕获分频：不分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM 向上计数
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);         // ②初始化 TIM2

    TIM_ARRPreloadConfig(TIM2, DISABLE);//关闭TIM2定时器ARR自动重装载寄存器的预装载功能，对ARR寄存器的修改立即生效
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);              // ③允许更新中断

    // 中断优先级 NVIC 设置
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;          // TIM2 中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 先占优先级 0 级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;       // 从优先级 2 级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;          // IRQ 通道被使能
    NVIC_Init(&NVIC_InitStructure);                         // ④初始化 NVIC

    TIM_Cmd(TIM2, ENABLE);                                  // ⑤使能 TIM2
}
/* 定时器2中断函数，输出舵机控制波形 */
void TIM2_IRQHandler(void) {
    static u8 flag = 0;								//脉冲的高电平阶段还是低电平阶段
    static u8 duoji_index1 = 0;
    int temp;										//低电平持续时间

    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) { // 检查TIM2更新中断发生与否
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update); // 清除TIM2更新中断标志

        /* 通过改变重装数值和舵机下标索引，每个舵机定时2500(2.5ms)，执行8个舵机后完成一个周期20000(20ms) */
        if (duoji_index1 == DJ_NUM) {
            duoji_index1 = 0;
        }

        if (flag == 0) {		//高电平
            TIM2->ARR = ((unsigned int)(duoji_doing[duoji_index1].cur));
            servo_pin_set(duoji_index1, Bit_SET);
            servo_inc_offset(duoji_index1);					// 计算下一个周期的目标位置
        } else {
            temp = 2500 - (unsigned int)(duoji_doing[duoji_index1].cur);
            TIM2->ARR = temp;
            servo_pin_set(duoji_index1, Bit_RESET);
            duoji_index1++;
        }
        flag = !flag;
    }
}
