#ifndef _APP_SERVO_H_
#define _APP_SERVO_H_

#include "main.h"


#define DJ_NUM 8		// 舵机数量
#define W25Q64_INFO_ADDR_SAVE_STR (((8 << 10) - 4) << 10)// W25Q64 信息存储地址计算 
#define FLAG_VERIFY 0x25// 校验标志
#define ACTION_SIZE 256// 一个动作的存储大小
#define PRE_CMD_SIZE 128// 预命令大小
#define CMD_RETURN_SIZE 1024// 保存命令大小
extern u8 cmd_return[CMD_RETURN_SIZE];// 定义命令数组大小

// 存储命令结构体
typedef struct {
    u32 version;                  // 版本号
    u32 dj_record_num;            // 舵机记录编号
    u8 pre_cmd[PRE_CMD_SIZE + 1]; // 预存命令数组
    int dj_bias_pwm[DJ_NUM + 1];  // 舵机偏差数组
} eeprom_info_t;

extern eeprom_info_t eeprom_info;

void parse_action(u8* uart_receive_buf);//执行舵机命令
void save_action(u8* str);//存储舵机命令
void parse_cmd(u8* cmd);//执行命令模式
void loop_action(void);//动作组批量执行
void rewrite_eeprom(void);//写入 W25Q64 存储位置
void replace_char(u8* str, u8 ch1, u8 ch2);//字符替代
void int_exchange(int* int1, int* int2);// int 变量交换
void print_group(int start, int end);//打印动作组
uint16_t str_contain_str(unsigned char* str, unsigned char* str2);//判断子串是否存在
void do_group_once(int group_num);//执行动作组 1 次
int getMaxTime(u8* str);//获取最大时间
void soft_reset(void);//单片机软件复位

#endif
