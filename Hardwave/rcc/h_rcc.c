#include "./rcc/h_rcc.h"
//将单片机的系统时钟（SYSCLK）从默认的8MHz提升到最高性能的72MHz，并合理分配各个总线的运行速度
void rcc_init(void){
	ErrorStatus HSEStartUpStatus;	//定义一个枚举变量表示HSE振荡器的开启状态，失败0、成功1
	RCC_DeInit();					//RCC复位
	RCC_HSEConfig(RCC_HSE_ON);		//开启外部时钟，并初始化
	HSEStartUpStatus=RCC_WaitForHSEStartUp();//等待外部时钟准备完毕
	while(HSEStartUpStatus==ERROR);	//启动失败等待
	//设置时钟总线
	RCC_HCLKConfig(RCC_SYSCLK_Div1);				//配置 AHB 总线时钟（HCLK）的分频系数
	RCC_PCLK1Config(RCC_HCLK_Div2);					//配置 APB1 总线时钟（PCLK1）的分频系数36MHZ
	RCC_PCLK2Config(RCC_HCLK_Div1);					//配置 APB2 总线时钟（PCLK2）的分频系数72MHZ
	// 选择 HSE(8MHz) 作为 PLL 输入，并进行 9 倍频
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1,RCC_PLLMul_9);//配置 PLL 的时钟源和倍频系数
	RCC_PLLCmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY)==RESET);//获取锁相环用作系统的时钟标志位，PLL 就绪标志位被置位
	FLASH_SetLatency(FLASH_Latency_2);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);		// 将系统时钟源从默认的HSI切换为PLL的输出 (72MHz)
	while(RCC_GetSYSCLKSource()!=0x08);				 //判断锁相环用作系统时钟0x08=PLL
	return;
}
