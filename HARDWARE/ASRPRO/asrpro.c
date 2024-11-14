#include "includes.h"

void asr_init(uint32_t baud)
{
	usart1_init(baud);	
	
	//�򿪶˿�D��Ӳ��ʱ�ӣ����ǹ���
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_14; 	//6������
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;	//����
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;//���٣��ٶ�Խ�ߣ���ӦԽ�죬���ǹ��Ļ����
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;//��ʹ������������
	GPIO_Init(GPIOD,&GPIO_InitStructure);		
}

void asr_send_str(char * str)
{
	usart_send_str(USART1,str);
}
