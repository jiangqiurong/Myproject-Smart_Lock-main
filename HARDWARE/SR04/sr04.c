#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"



void sr04_init(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/* 配置PB8引脚为输出模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;					//第6根引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;				//设置输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				//推挽模式，增加驱动电流
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;			//设置IO的速度为100MHz，频率越高性能越好，频率越低，功耗越低
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;			//不需要上拉电阻
	GPIO_Init(GPIOB, &GPIO_InitStructure);


	/* 配置PB9引脚为输入模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;					//第6根引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;				//设置输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;			//设置IO的速度为100MHz，频率越高性能越好，频率越低，功耗越低
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;			//不需要上拉电阻
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	
	PBout(8)=0;
}


int32_t sr04_get_distance(void)
{
	int32_t t=0;
	PBout(8)=1;
	delay_us(20);
	PBout(8)=0;
	
	//等待回响信号变为高电平
	while(PBin(9)==0)
	{
		t++;
		delay_us(1);
		
		//如果超时，就返回一个错误码
		if(t>=1000000)
			return -1;
	}
	
	t=0;
	//测量高电平持续的时间
	while(PBin(9))
	{
	
		//延时9us,就是3mm的传输距离
		delay_us(9);
		
		t++;
		
		//如果超时，就返回一个错误码
		if(t>=100000)
			return -1;		
	
	}
	
	//当前的传输距离
	return 3*(t/2);
}

