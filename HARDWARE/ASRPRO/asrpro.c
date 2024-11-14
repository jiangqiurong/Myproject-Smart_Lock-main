#include "includes.h"

void asr_init(uint32_t baud)
{
	usart1_init(baud);	
	
	//打开端口D的硬件时钟，就是供电
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_14; 	//6号引脚
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;	//输入
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;//高速，速度越高，响应越快，但是功耗会更高
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;//不使能上下拉电阻
	GPIO_Init(GPIOD,&GPIO_InitStructure);		
}

void asr_send_str(char * str)
{
	usart_send_str(USART1,str);
}
