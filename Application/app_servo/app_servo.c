#include "./app_servo/app_servo.h"

u8 cmd_return[CMD_RETURN_SIZE];
eeprom_info_t eeprom_info;

u8 group_do_ok = 1;
u8 AI_mode = 255;

int do_start_index;   // 动作组执行 起始序号
int do_time;          // 动作组执行 执行次数
int group_num_start;  // 动作组执行 起始序号
int group_num_end;    // 动作组执行 终止序号
int group_num_times;  // 动作组执行 起始变量
u32 action_time = 0;

// 发送串口指令
void zx_uart_send_str(u8* str) {
    uart1_get_ok = 1;
    Usart_SendString(USART1, str);
    Usart_SendString(USART3, str);
    uart1_get_ok = 0;
}
// 处理 #000P1500T1000! 类似的字符串
void parse_action(u8* uart_receive_buf) {
    u16 index, time, i = 0;			//舵机ID
    int bias, len;					//偏差值
    float pwm;

    zx_uart_send_str(uart_receive_buf); // 将指令发给总线舵机控制总线舵机
	//通过硬编码的字符位置，检查指令是否为偏差校准指令。（例如 #001PSCK+100!）
    if (uart_receive_buf[0] == '#' && uart_receive_buf[4] == 'P' && uart_receive_buf[5] == 'S' && uart_receive_buf[6] == 'C' && uart_receive_buf[7] == 'K' && uart_receive_buf[12] == '!') {
        // 调整偏差值
        index = (uart_receive_buf[1] - '0') * 100 + (uart_receive_buf[2] - '0') * 10 + (uart_receive_buf[3] - '0');
        bias = (uart_receive_buf[9] - '0') * 100 + (uart_receive_buf[10] - '0') * 10 + (uart_receive_buf[11] - '0');

        if ((bias >= -500) && (bias <= 500) && (index < DJ_NUM)) {
            if (uart_receive_buf[8] == '+') {
                duoji_doing[index].cur = duoji_doing[index].cur - eeprom_info.dj_bias_pwm[index] + bias;//先减掉旧偏差
                eeprom_info.dj_bias_pwm[index] = bias;
            } else if (uart_receive_buf[8] == '-') {
                duoji_doing[index].cur = duoji_doing[index].cur - eeprom_info.dj_bias_pwm[index] - bias;
                eeprom_info.dj_bias_pwm[index] = -bias;
            }
            duoji_doing[index].aim = duoji_doing[index].cur;
            duoji_doing[index].inc = 0.001;
            rewrite_eeprom(); // 把 eeprom_info 写入到 W25Q64_INFO_ADDR_SAVE_STR 位置，久保存到 SPI Flash 中，防止断电丢失
        }
    } else if (uart_receive_buf[0] == '#' && uart_receive_buf[4] == 'P' && uart_receive_buf[5] == 'D' && uart_receive_buf[6] == 'S' && uart_receive_buf[7] == 'T' && uart_receive_buf[8] == '!') {
        // 原地停止执行如 	#001PDST!
        index = (uart_receive_buf[1] - '0') * 100 + (uart_receive_buf[2] - '0') * 10 + (uart_receive_buf[3] - '0');
        if (index < DJ_NUM) {
            duoji_doing[index].inc = 0;
            duoji_doing[index].aim = duoji_doing[index].cur;
        } else if (index == 255) {//如果是 255（广播地址）
            for (index = 0; index < DJ_NUM; index++) {
                duoji_doing[index].inc = 0;
                duoji_doing[index].aim = duoji_doing[index].cur;
            }
        }
        return;
    }

    // 舵机执行
    len = strlen((char*)uart_receive_buf); // 获取串口接收数据的长度
    while (uart_receive_buf[i] && (len >= 1)) {
        if (uart_receive_buf[i] == '#') {
            index = 0;
            i++;
            while (uart_receive_buf[i] && uart_receive_buf[i] != 'P') {
                index = index * 10 + uart_receive_buf[i] - '0';
                i++;
            }	//解析舵机 ID
        } else if (uart_receive_buf[i] == 'P') {
            pwm = 0;
            i++;
            while (uart_receive_buf[i] && uart_receive_buf[i] != 'T') {
                pwm = pwm * 10 + uart_receive_buf[i] - '0';
                i++;
            }	//解析目标脉宽
        } else if (uart_receive_buf[i] == 'T') {
            time = 0;
            i++;
            while (uart_receive_buf[i] && uart_receive_buf[i] != '!') {
                time = time * 10 + uart_receive_buf[i] - '0';
                i++;
            }	//解析目标时间
            pwm += eeprom_info.dj_bias_pwm[index]; // 偏差值
            duoji_doing_set(index, pwm, time);
        } else {
            i++;
        }
    }
}
// 动作组保存函数，只有用<>包含的字符串才能在此函数中进行解析
// <G0000#000P1500T1000!#001P1500T1000!B000!> 用尖括号括起来 带有组序号
void save_action(u8* str) {
    s32 action_index = 0;
    group_do_ok = 1; /* 停止动作组 */

    // 预存命令处理
    spiFlashOn(1); // 切换 SPI 和 LED 引脚状态
    Delay_ms(10);

    // 取消预存储命令 <$!>
    if (str[1] == '$' && str[2] == '!') {
        eeprom_info.pre_cmd[PRE_CMD_SIZE] = 0;
        rewrite_eeprom();
        zx_uart_send_str((u8*)"@CLEAR PRE_CMD OK!");
        return;
    } else if (str[1] == '$') {
        // 设置开机动作组成功！ @SET PRE_CMD OK!
        //<G0000#000P1500T1500!#001P1500T1500!#002P1500T1500!#003P1500T1500!#004P1500T1500!#005P1500T1500!>
        memset(eeprom_info.pre_cmd, 0, sizeof(eeprom_info.pre_cmd));
        strcpy((char*)eeprom_info.pre_cmd, (char*)str + 1); // 对字符串进行复制
        eeprom_info.pre_cmd[strlen((char*)str) - 2] = '\0'; // 赋值字符 0
        eeprom_info.pre_cmd[PRE_CMD_SIZE] = FLAG_VERIFY;		//写入校验标志，预存命令是有效的
        rewrite_eeprom();
        zx_uart_send_str((u8*)"@SET PRE_CMD OK!");
        zx_uart_send_str((u8*)eeprom_info.pre_cmd); // 打印存储进去的指令
        return;
    }

   
    action_index = (str[2] - '0') * 1000 + (str[3] - '0') * 100 + (str[4] - '0') * 10 + (str[5] - '0');

    // <G0000#000P1500T1000!> // 获取动作的组号如果不正确，或是第 6 个字符不是#则认为字符串错误
    if ((action_index == -1) || str[6] != '#') {
        UsartPrintf(USART_DEBUG, "E");
        return;
    }
	//计算当前动作组ID对应的物理地址。如果该地址刚好是一个新扇区的起始地址
	//就先擦除这个扇区，为写入新数据腾出干净的空间。
    if ((action_index * ACTION_SIZE % 4096) == 0) {
        w25x_erase_sector(action_index * ACTION_SIZE / 4096); // 擦除一个扇区
    }

    // 把尖括号替换成大括号直接存储到存储芯片里面去，则在执行动作组的时候直接拿出来解析就可以了
    replace_char(str, '<', '{');
    replace_char(str, '>', '}');
    w25x_write(str, action_index * ACTION_SIZE, strlen((char*)str) + 1);
    Usart_SendString(USART1, (u8*)"A");
    Usart_SendString(USART3, (u8*)"A");
    spiFlashOn(0);
    return;
}
/*
命令解析函数
所有舵机停止命令：      $DST!
第 x 个舵机停止命令：   $DST:x!
单片机重启命令：        $RST!
检查动作组 x 到 y 组命令：$CGP:x-y!
执行第 x 个动作：       $DGS:x!
执行第 x 到 y 组动作 z 次： $DGT:x-y,z! $DGT:0-1,1!
获取第 x 到 y 组动作：  $PTG:%x-%y! $PTG:0-1!
所有舵机复位命令：      $DJR!
获取应答信号：          $GETA!
*/
void parse_cmd(u8* cmd) {
    int pos, i, index, int1, int2;		

    if (pos = str_contain_str(cmd, (u8*)"$DRS!"), pos) {
        // 测试命令
        UsartPrintf(USART_DEBUG, "hello world！");
    } else if (pos = str_contain_str(cmd, (u8*)"$DST!"), pos) {
        // 所有舵机停止命令
		group_do_ok = 1;
		for (i = 0; i < DJ_NUM; i++) {
            duoji_doing[i].inc = 0;
            duoji_doing[i].aim = duoji_doing[i].cur;
        }
		zx_uart_send_str((u8*)"255PDST!");//总线停止
	} else if (pos = str_contain_str(cmd, (u8*)"$DST:"), pos) {
		//第x个舵机停止命令
        if(sscanf((char*)cmd, "$DST:%d!", &index)){
			duoji_doing[index].inc = 0;
            duoji_doing[index].aim = duoji_doing[index].cur;
			sprintf((char*)cmd_return,"#%03dPDST!\r\n",(int)index);
			zx_uart_send_str(cmd_return);
			memset(cmd_return,0,sizeof(cmd_return));
		} 
    } else if (pos = str_contain_str(cmd, (u8*)"$RST!"), pos) {
        // 单片机复位
        soft_reset();
    } else if (pos = str_contain_str(cmd, (u8*)"$PTG:"), pos) {
        // 获取动作组
        if(sscanf((char*)cmd, "$PTG:%d-%d!", &int1,&int2)){
			print_group(int1,int2);			//动作组数据读取出来，并通过串口打印到上位机
		}
    } else if (pos = str_contain_str(cmd, (u8*)"$DGS:"), pos) {
        // 执行第x个动作
        if(sscanf((char*)cmd, "$DGS:%d!", &int1)){
			group_do_ok=1;					//置位标志，准备执行
			do_group_once(int1);
		}
    } else if (pos = str_contain_str(cmd, (u8*)"$DGT:"), pos) {
        // 执行第 x 到 y 组动作 z 次
        if (sscanf((char*)cmd, "$DGT:%d-%d,%d!", &group_num_start, &group_num_end, &group_num_times)) {
			group_do_ok = 1;
			if(group_num_start!=group_num_end){
				do_start_index=group_num_start;
				do_time=group_num_times;
				group_do_ok=0;//状态机开始工作
			}else{
				do_group_once(group_num_start);
            }
		}
	}else if(pos = str_contain_str(cmd, (u8*)"$DJR!"), pos){
		//所有舵机复位命令
		zx_uart_send_str((u8*)"#255P1500T2000!\r\n");
		AI_mode=255;
		for(i=0;i<DJ_NUM;i++){
			duoji_doing[i].aim=1500+eeprom_info.dj_bias_pwm[i];
			duoji_doing[i].time=2000;
			duoji_doing[i].inc=(duoji_doing[i].aim-duoji_doing[i].cur)/(duoji_doing[i].time/20.000);
		}
    } else if (pos = str_contain_str(cmd, (u8*)"$GETA!"), pos) {
        // 获取应答信号
        UsartPrintf(USART_DEBUG, "AAA");
    } else if (pos = str_contain_str(cmd, (u8*)"$BEEP!"), pos) {
        // 蜂鸣器响时间
        beep_on_times(1, 100);
    }
}
// 动作组批量执行
void loop_action(void) {
    static long long systick_ms_bak = 0;//时间戳,记录上一次执行动作的时间点
    // 通过判断舵机是否全部执行完毕，并且是执行动作组 group_do_ok 尚未结束的情况下进入处理
    if (group_do_ok == 0) {
        if (millis() - systick_ms_bak > action_time) {//时间差大于当前动作组的执行时间
            systick_ms_bak = millis();
            if (group_num_times != 0 && do_time == 0) {
                group_do_ok = 1;
                UsartPrintf(USART_DEBUG, "@GroupDone!");
                return;
            }
            // 调用 do_start_index 个动作
            do_group_once(do_start_index);		//从Flash中读取动作数据，并设置每个舵机的目标位置和运动时间
            if (group_num_start < group_num_end) {//正序执行
                if (do_start_index == group_num_end) {//最后一个动作
                    do_start_index = group_num_start;
                    if (group_num_times != 0) {
                        do_time--;
                    }
					return;
				}   
                    do_start_index++;              
            } else {//倒序执行分支
                if (do_start_index == group_num_end) {
                    do_start_index = group_num_start;
                    if (group_num_times != 0) {
                        do_time--;
                    }
					return;
                } 
                 do_start_index--;  
            }
        }
    }else {
		action_time=10;		//空闲状态下的默认时间保护
	}
}

void rewrite_eeprom(void) {
    spiFlashOn(1);
    Delay_ms(10);
    // 擦除一个扇区 最少 150 毫秒
    w25x_erase_sector(W25Q64_INFO_ADDR_SAVE_STR / 4096);
    // 写入整个扇区
    w25x_write((u8*)&eeprom_info, W25Q64_INFO_ADDR_SAVE_STR, sizeof(eeprom_info));
    spiFlashOn(0);
}


// 字符串中的字符替代函数 把 str 字符串中所有的 ch1 换成 ch2
void replace_char(u8* str, u8 ch1, u8 ch2) {
    while (*str) {
        if (*str == ch1) {
            *str = ch2;
        }
        str++;
    }
}
// 打印存储在芯片里的动作组，从串口1中发送出来 $CCP:x-y!这个命令调用
void print_group(int start, int end) {
    spiFlashOn(1);					//打开spi引脚
    Delay_ms(10);
    if (start > end) {
        int_exchange(&start, &end);
    }
    for (; start <= end; start++) {
        memset(uart_receive_buf, 0, sizeof(uart_receive_buf));
        w25x_read(uart_receive_buf, start * ACTION_SIZE, ACTION_SIZE);
        Usart_SendString(USART1, uart_receive_buf);
        Usart_SendString(USART1, (u8*)"\r\n");
    }
    spiFlashOn(0);
}
// 两个 int 变量交换
void int_exchange(int* int1, int* int2) {
    int int_temp;
    int_temp = *int1;
    *int1 = *int2;
    *int2 = int_temp;
}

// 判断子串
uint16_t str_contain_str(unsigned char* str, unsigned char* str2) {
    unsigned char* str_temp, * str_temp2;
    str_temp = str;
    str_temp2 = str2;
    while (*str_temp) {
        if (*str_temp == *str_temp2) {			//第一个字符相同
            while (*str_temp2) {
                if (*str_temp++ != *str_temp2++) {
                    str_temp = str_temp - (str_temp2 - str2) + 1;
                    str_temp2 = str2;
                    break;
                }
            }
            if (!*str_temp2) {
                return (str_temp - str);		//子串 str2 在母串str中首次匹配成功的起始下标
            }
        } else {
            str_temp++;							//第一个不相同，主字符串移到后一位
        }
    }
    return 0;
}

// 执行动作组 1 次，参数是动作组序号
void do_group_once(int group_num) {
    spiFlashOn(1);
    Delay_ms(10);
    // 将 uart_receive_buf 清零
    memset(uart_receive_buf, 0, sizeof(uart_receive_buf));
    // 从存储芯片中读取第 group_num 个动作组
    w25x_read(uart_receive_buf, group_num * ACTION_SIZE, ACTION_SIZE);
   
    // 获取最大的组时间
    action_time = getMaxTime(uart_receive_buf);
    // 把读出来的动作组传递给 parse_action 执行
    parse_action(uart_receive_buf);
	spiFlashOn(0);
}

// 获取最大时间
int getMaxTime(u8* str) {
    int i = 0, max_time = 0, tmp_time = 0;
    while (str[i]) {
        if (str[i] == 'T') {
            tmp_time = (str[i + 1] - '0') * 1000 + (str[i + 2] - '0') * 100 + (str[i + 3] - '0') * 10 + (str[i + 4] - '0');
            if (tmp_time > max_time) {
                max_time = tmp_time;
            }
			i=i+4;
            continue;
        }
        i++;
    }
    return max_time;
}
/* 单片机软件复位 */
void soft_reset(void) {
    UsartPrintf(USART1, "stm32 reset\r\n");
    __set_FAULTMASK(1); // 关闭所有中断
    NVIC_SystemReset(); // 复位
}
