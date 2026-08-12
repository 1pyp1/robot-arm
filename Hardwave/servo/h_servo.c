#include "./servo/h_servo.h"

servo_t duoji_doing[DJ_NUM]; // 舵机数量

/* 舵机gpio初始化 */
void servo_init(void) {
    u8 i;
    RCC_APB2PeriphClockCmd(SERVO0_GPIO_CLK | SERVO1_GPIO_CLK | SERVO2_GPIO_CLK |
                           SERVO3_GPIO_CLK | SERVO4_GPIO_CLK | SERVO5_GPIO_CLK, ENABLE); // 使能舵机端口时钟

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = SERVO0_PIN;       // 配置引脚
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 翻转50MHZ
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出
    GPIO_Init(SERVO0_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SERVO1_PIN;
    GPIO_Init(SERVO1_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SERVO2_PIN;
    GPIO_Init(SERVO2_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SERVO3_PIN;
    GPIO_Init(SERVO3_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SERVO4_PIN;
    GPIO_Init(SERVO4_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SERVO5_PIN;
    GPIO_Init(SERVO5_GPIO_PORT, &GPIO_InitStructure);

    /* 给每个舵机进行初始化赋值 */
    for (i = 0; i < DJ_NUM; i++) {
        duoji_doing[i].aim = 1500; // 执行目标
        duoji_doing[i].cur = 1500; // 当前值
        duoji_doing[i].inc = 0;    // 增量
        duoji_doing[i].time = 5000; // 执行时间
    }
}

/* 设置舵机引脚电平，参数index 舵机索引，level 舵机引脚电平 */
void servo_pin_set(u8 index, BitAction level) {
    switch (index) {
        case 0: SERVO0_PIN_SET(level); break;
        case 1: SERVO1_PIN_SET(level); break;
        case 2: SERVO2_PIN_SET(level); break;
        case 3: SERVO3_PIN_SET(level); break;
        case 4: SERVO4_PIN_SET(level); break;
        case 5: SERVO5_PIN_SET(level); break;
        default: break;
    }
}

/* 设置舵机控制参数函数，参数index 舵机编号 aim 执行目标 time 执行时间
   (如果aim 执行目标==0，视为舵机停止) */
void duoji_doing_set(u8 index, int aim, int time) {
    /* 限制输入值大小 */
    if (index >= DJ_NUM) {
        return;
    }

    /* 执行目标 */
    if (aim == 0) {//特殊处理，如果目标 aim 为 0，视为“停止/保持”指令
        duoji_doing[index].inc = 0; // 增量
        duoji_doing[index].aim = duoji_doing[index].cur; // 当前值赋值给执行目标
        return;
    } else if (aim > 2490) {
        aim = 2490;
    } else if (aim < 510) {
        aim = 510;
    }

    /* 执行时间 */
    if (time > 10000) {
        time = 10000;
    }

    /* 当前值 */
    if (duoji_doing[index].cur == aim) {
        aim = aim + 0.0077; // 微调，防抖动/防停滞微调
    }

    if (time < 20) { /* 执行时间太短，舵机直接以最快速度运动 */
        duoji_doing[index].aim = aim;
        duoji_doing[index].time = time;
        duoji_doing[index].inc = 0;
    } else {
        duoji_doing[index].aim = aim;
        duoji_doing[index].time = time;
        duoji_doing[index].inc = (duoji_doing[index].aim - duoji_doing[index].cur) /
                                 (duoji_doing[index].time / 20.000);
    }
}

/* 处理绝对值 */
float abs_float(float value) {
    if (value > 0) {
        return value;
    }
    return (-value);
}

/* 设置舵机每次增加的偏移量 */
void servo_inc_offset(u8 index) {
    int aim_temp;
    if (duoji_doing[index].inc != 0) {
        aim_temp = duoji_doing[index].aim;
        if (aim_temp > 2490) {
            aim_temp = 2490;
        } else if (aim_temp < 500) {
            aim_temp = 500;
        }
        /* 如果执行距离过短小于两个步长，直接运行到执行距离，不用增量 */
        if ((aim_temp-duoji_doing[index].cur) <= abs_float(duoji_doing[index].inc + duoji_doing[index].inc)) {
            duoji_doing[index].cur = aim_temp;
            duoji_doing[index].inc = 0;
        } else {
            duoji_doing[index].cur += duoji_doing[index].inc;
        }
    }
}



