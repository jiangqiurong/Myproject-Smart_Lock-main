#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"



void sr04_init(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/* ����PB8����Ϊ���ģʽ */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;					//��6������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;				//�������ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				//����ģʽ��������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;			//����IO���ٶ�Ϊ100MHz��Ƶ��Խ������Խ�ã�Ƶ��Խ�ͣ�����Խ��
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;			//����Ҫ��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);


	/* ����PB9����Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;					//��6������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;				//�������ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;			//����IO���ٶ�Ϊ100MHz��Ƶ��Խ������Խ�ã�Ƶ��Խ�ͣ�����Խ��
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;			//����Ҫ��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	
	PBout(8)=0;
}


int32_t sr04_get_distance(void)
{
	int32_t t=0;
	PBout(8)=1;
	delay_us(20);
	PBout(8)=0;
	
	//�ȴ������źű�Ϊ�ߵ�ƽ
	while(PBin(9)==0)
	{
		t++;
		delay_us(1);
		
		//�����ʱ���ͷ���һ��������
		if(t>=1000000)
			return -1;
	}
	
	t=0;
	//�����ߵ�ƽ������ʱ��
	while(PBin(9))
	{
	
		//��ʱ9us,����3mm�Ĵ������
		delay_us(9);
		
		t++;
		
		//�����ʱ���ͷ���һ��������
		if(t>=100000)
			return -1;		
	
	}
	
	//��ǰ�Ĵ������
	return 3*(t/2);
}

