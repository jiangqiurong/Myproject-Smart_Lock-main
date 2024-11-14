#include "includes.h"

//�����ʼ��
void motor_init(void)
{

	/* ʹ�ܶ�Ӧ��GPIOD GPIOE ʱ��*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_15;		//��1���͵�15������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;				//����Ϊ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				//����ģʽ��������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;			//����IO���ٶ�Ϊ100MHz��Ƶ��Խ������Խ�ã�Ƶ��Խ�ͣ�����Խ��
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;			//����Ҫ��������
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_10;		//��8���͵�10������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;				//����Ϊ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				//����ģʽ��������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;			//����IO���ٶ�Ϊ100MHz��Ƶ��Խ������Խ�ã�Ƶ��Խ�ͣ�����Խ��
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;			//����Ҫ��������
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	MOTOR_IN1=MOTOR_IN2=MOTOR_IN3=MOTOR_IN4=1;
}

//˫����������ʽ��ת��180�� AD-DC-CB-BA ,��һ��ѭ��i<64ת360��i<32ת180
void motor_corotation_double_pos_180(void)
{
	uint32_t i=0,j=0;
	
	for(i=0; i<32;i++)
	{
		for(j=0;j<8;j++)
		{
			MOTOR_IN4=0;
			MOTOR_IN3=1;
			MOTOR_IN2=1;
			MOTOR_IN1=0;
			delay_ms(2);
			
			MOTOR_IN4=0;
			MOTOR_IN3=0;
			MOTOR_IN2=1;
			MOTOR_IN1=1;
			delay_ms(2);
			
			MOTOR_IN4=1;
			MOTOR_IN3=0;
			MOTOR_IN2=0;
			MOTOR_IN1=1;
			delay_ms(2);
			
			MOTOR_IN4=1;
			MOTOR_IN3=1;
			MOTOR_IN2=0;
			MOTOR_IN1=0;
			delay_ms(2);	
			
					
		}
	}
	//�ر����еƹ�
	MOTOR_IN4=0;
	MOTOR_IN3=0;
	MOTOR_IN2=0;
	MOTOR_IN1=0;
	delay_ms(2);
}

//˫����������ʽ��ת��180�� AB-BC-CD-DA,��һ��ѭ��i<64ת360��i<32ת180
void motor_corotation_double_rev_180(void)
{
	uint32_t i=0,j=0;
	
	for(i=0; i<32;i++)
	{
		for(j=0;j<8;j++)
		{
			MOTOR_IN4=1;
			MOTOR_IN3=1;
			MOTOR_IN2=0;
			MOTOR_IN1=0;
			delay_ms(2);
			
			MOTOR_IN4=1;
			MOTOR_IN3=0;
			MOTOR_IN2=0;
			MOTOR_IN1=1;
			delay_ms(2);
			
			MOTOR_IN4=0;
			MOTOR_IN3=0;
			MOTOR_IN2=1;
			MOTOR_IN1=1;
			delay_ms(2);
			
			MOTOR_IN4=0;
			MOTOR_IN3=1;
			MOTOR_IN2=1;
			MOTOR_IN1=0;
			delay_ms(2);	
			
					
		}
	}
	//�ر����еƹ�
	MOTOR_IN4=0;
	MOTOR_IN3=0;
	MOTOR_IN2=0;
	MOTOR_IN1=0;
	delay_ms(2);
}

//������������ʽ��ת�� D-C-B-A 360��
void motor_corotation_single_pos(void)
{
	uint32_t i=0,j=0;
	
	for(i=0; i<64;i++)
	{
		for(j=0;j<8;j++)
		{
			MOTOR_IN4=0;
			MOTOR_IN3=1;
			MOTOR_IN2=1;
			MOTOR_IN1=1;
			delay_ms(2);
			
			MOTOR_IN4=1;
			MOTOR_IN3=0;
			MOTOR_IN2=1;
			MOTOR_IN1=1;
			delay_ms(2);
			
			MOTOR_IN4=1;
			MOTOR_IN3=1;
			MOTOR_IN2=0;
			MOTOR_IN1=1;
			delay_ms(2);
			
			MOTOR_IN4=1;
			MOTOR_IN3=1;
			MOTOR_IN2=1;
			MOTOR_IN1=0;
			delay_ms(2);	
			
					
		}
	}
}

//������������ʽ��ת�� A-B-C-D 360��
void motor_corotation_single_rev(void)
{
	uint32_t i=0,j=0;
	
	for(i=0; i<64;i++)
	{
		for(j=0;j<8;j++)
		{
			MOTOR_IN4=1;
			MOTOR_IN3=1;
			MOTOR_IN2=1;
			MOTOR_IN1=0;
			delay_ms(2);
			
			MOTOR_IN4=1;
			MOTOR_IN3=1;
			MOTOR_IN2=0;
			MOTOR_IN1=1;
			delay_ms(2);
			
			MOTOR_IN4=1;
			MOTOR_IN3=0;
			MOTOR_IN2=1;
			MOTOR_IN1=1;
			delay_ms(2);
			
			MOTOR_IN4=0;
			MOTOR_IN3=1;
			MOTOR_IN2=1;
			MOTOR_IN1=1;
			delay_ms(2);	
			
					
		}
	}
}

//˫����������ʽ��ת��360�� AD-DC-CB-BA 
void motor_corotation_double_pos(void)
{
	uint32_t i=0,j=0;
	
	for(i=0; i<64;i++)
	{
		for(j=0;j<8;j++)
		{
			MOTOR_IN4=0;
			MOTOR_IN3=1;
			MOTOR_IN2=1;
			MOTOR_IN1=0;
			delay_ms(2);
			
			MOTOR_IN4=0;
			MOTOR_IN3=0;
			MOTOR_IN2=1;
			MOTOR_IN1=1;
			delay_ms(2);
			
			MOTOR_IN4=1;
			MOTOR_IN3=0;
			MOTOR_IN2=0;
			MOTOR_IN1=1;
			delay_ms(2);
			
			MOTOR_IN4=1;
			MOTOR_IN3=1;
			MOTOR_IN2=0;
			MOTOR_IN1=0;
			delay_ms(2);	
			
					
		}
	}
}

//˫����������ʽ��ת��360�� AB-BC-CD-DA
void motor_corotation_double_rev(void)
{
	uint32_t i=0,j=0;
	
	for(i=0; i<64;i++)
	{
		for(j=0;j<8;j++)
		{
			MOTOR_IN4=1;
			MOTOR_IN3=1;
			MOTOR_IN2=0;
			MOTOR_IN1=0;
			delay_ms(2);
			
			MOTOR_IN4=1;
			MOTOR_IN3=0;
			MOTOR_IN2=0;
			MOTOR_IN1=1;
			delay_ms(2);
			
			MOTOR_IN4=0;
			MOTOR_IN3=0;
			MOTOR_IN2=1;
			MOTOR_IN1=1;
			delay_ms(2);
			
			MOTOR_IN4=0;
			MOTOR_IN3=1;
			MOTOR_IN2=1;
			MOTOR_IN1=0;
			delay_ms(2);	
			
					
		}
	}
}

//����������ʽ��ת��360�� AD-D-DC-C-CB-B-BA-A
void motor_corotation_eghit_pos(void)
{
	uint32_t i=0,j=0;
	
	for(i=0; i<64;i++)
	{
		for(j=0;j<8;j++)
		{
			MOTOR_IN4=0;
			MOTOR_IN3=1;
			MOTOR_IN2=1;
			MOTOR_IN1=0;
			delay_ms(2);
			
			MOTOR_IN4=0;
			MOTOR_IN3=1;
			MOTOR_IN2=1;
			MOTOR_IN1=1;
			delay_ms(2);
			
			MOTOR_IN4=0;
			MOTOR_IN3=0;
			MOTOR_IN2=1;
			MOTOR_IN1=1;
			delay_ms(2);
			
			MOTOR_IN4=1;
			MOTOR_IN3=0;
			MOTOR_IN2=1;
			MOTOR_IN1=1;
			delay_ms(2);	
			
			MOTOR_IN4=1;
			MOTOR_IN3=0;
			MOTOR_IN2=0;
			MOTOR_IN1=1;
			delay_ms(2);
			
			MOTOR_IN4=1;
			MOTOR_IN3=1;
			MOTOR_IN2=0;
			MOTOR_IN1=1;
			delay_ms(2);	
			
			MOTOR_IN4=1;
			MOTOR_IN3=1;
			MOTOR_IN2=0;
			MOTOR_IN1=0;
			delay_ms(2);

			MOTOR_IN4=1;
			MOTOR_IN3=1;
			MOTOR_IN2=1;
			MOTOR_IN1=0;
			delay_ms(2);		
		}
	}
}

//����������ʽ��ת�� A-AB-B-BC-C-CD-D-DA
void motor_corotation_eghit_rev(void)
{
	uint32_t i=0,j=0;
	
	for(i=0; i<64;i++)
	{
		for(j=0;j<8;j++)
		{
			MOTOR_IN4=1;
			MOTOR_IN3=1;
			MOTOR_IN2=1;
			MOTOR_IN1=0;
			delay_ms(2);
			
			MOTOR_IN4=1;
			MOTOR_IN3=1;
			MOTOR_IN2=0;
			MOTOR_IN1=0;
			delay_ms(2);
			
			MOTOR_IN4=1;
			MOTOR_IN3=1;
			MOTOR_IN2=0;
			MOTOR_IN1=1;
			delay_ms(2);
			
			MOTOR_IN4=1;
			MOTOR_IN3=0;
			MOTOR_IN2=0;
			MOTOR_IN1=1;
			delay_ms(2);	
			
			MOTOR_IN4=1;
			MOTOR_IN3=0;
			MOTOR_IN2=1;
			MOTOR_IN1=1;
			delay_ms(2);
			
			MOTOR_IN4=0;
			MOTOR_IN3=0;
			MOTOR_IN2=1;
			MOTOR_IN1=1;
			delay_ms(2);	
			
			MOTOR_IN4=0;
			MOTOR_IN3=1;
			MOTOR_IN2=1;
			MOTOR_IN1=1;
			delay_ms(2);

			MOTOR_IN4=0;
			MOTOR_IN3=1;
			MOTOR_IN2=1;
			MOTOR_IN1=0;
			delay_ms(2);		
		}
	}
}
