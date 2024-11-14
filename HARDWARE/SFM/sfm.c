#include "includes.h"


uint8_t bcc_check(uint8_t *buf,uint32_t len)
{
	uint8_t s=0;
	uint8_t i=0;
	uint8_t *p = buf;
	
	for(i=0; i<len; i++)
		s = s^p[i];

	return s;
}


#if SFM_TOUCH_ENABLE 
/*
���ر�˵����
	���ϵ�����ָ��ģ���3V3���磬�ô�����Ӧ������Ч����ϸ�ٷ�˵�����£�
	Ϊ�˴ﵽ���ŵĵ͹�����ƣ�ʵ��Ӧ��ʱ���齫ģ��� VCC_3V3 ���ֶϵ�״̬��ͨ��
	�ж�TOUCH_OUT �ܽ��źſ��Ƹ�·��ѹ�������߹رա�����TOUCH_OUT ������Ч��ƽ
	ʱ��ʹ�� VCC_3V3 ��Դ����ʱָ��ģ����뵽����״̬��
��Ӳ��˵����	
   �����Ҫ���й�����һ�������ܵĿ��Ƶ�·
*/
void sfm_touch_init(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	EXTI_InitTypeDef	EXTI_InitStructure;	
	NVIC_InitTypeDef	NVIC_InitStructure;	
	
	//ʹ��ϵͳ����Ӳ��ʱ�ӣ�˵���˾��Ƕ�ϵͳ���õ�Ӳ������
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);	
	
	//�򿪶˿�E��Ӳ��ʱ�ӣ����ǶԶ˿�B����
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);	

	//�򿪶˿�E��Ӳ��ʱ�ӣ����ǶԶ˿�E����
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB,&GPIO_InitStructure);		
	
	//����GPIOA�ĵ�0������
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	
	GPIO_Init(GPIOE,&GPIO_InitStructure);	
	
	
	//��PE6�������ӵ�EXTI6
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource6);	
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line6;	//EXTI6
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			//�жϴ���
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; 	//�������ؾ����������ж�����֪ͨCPU�������� 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;					//ʹ��
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//�ⲿ�ж�9-5���жϺ�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x05;		//��Ӧ���ȼ� 0x05
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//NVICʹ��
	NVIC_Init(&NVIC_InitStructure);		
	
}
#endif

#if SFM_TOUCH_ENABLE 
uint32_t sfm_touch_sta(void)
{
	return (PEin(6)==0);
}

void sfm_power_ctrl(uint32_t on)
{
	PBout(15)=on;
}
#endif

int32_t sfm_init(uint32_t baud)
{
	int32_t rt=0;

	/* ����2 ��ʼ�� */
	usart2_init(baud);

	/* ��Ȧ����:ȫ��->ȫ������2�� */
	rt= sfm_ctrl_led(0x00,0x07,0xC8);
	if(rt != SFM_ACK_SUCCESS)	
		return rt;

	return SFM_ACK_SUCCESS;
}


int32_t sfm_ctrl_led(uint8_t led_start,uint8_t led_end,uint8_t period)
{
	uint8_t buf_tx[8]={0};
	uint8_t buf_rx[32]={0};	
	BaseType_t xReturn = pdFALSE;
	
	/* �������ù�Ȧ�������� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0xC3;	
	buf_tx[2]=led_start;	
	buf_tx[3]=led_end;	
	buf_tx[4]=period;		
	buf_tx[5]=0;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	xReturn = xQueueReceive(g_queue_sfm,	/* ��Ϣ���еľ�� */
							buf_rx,			/* �õ�����Ϣ���� */
							1000); 			/* �ȴ�ʱ�� */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] sfm_ctrl_led error code is %d\r\n", xReturn);
		return -1;

	}	
	
	if((buf_rx[0] == 0xF5) && (buf_rx[1] == 0xC3)&& (buf_rx[2] == SFM_ACK_SUCCESS))
	{
		return SFM_ACK_SUCCESS;
	}
	
	return SFM_ACK_FAIL;
}

int32_t sfm_reg_user(uint16_t id)
{
	uint8_t buf_tx[8]={0};
	uint8_t buf_rx[32]={0};	
	BaseType_t xReturn = pdFALSE;
	
	/* ע���û����� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x01;	
	buf_tx[2]=(uint8_t)(id>>8);	
	buf_tx[3]=(uint8_t)(id&0x00FF);	
	buf_tx[4]=0x01;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	xReturn = xQueueReceive(g_queue_sfm,	/* ��Ϣ���еľ�� */
							buf_rx,			/* �õ�����Ϣ���� */
							1000); 			/* �ȴ�ʱ�� */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] sfm_reg_user error code is %d\r\n", xReturn);
		return -1;

	}	
	
	if(!((buf_rx[0] == 0xF5) && (buf_rx[1] == 0x01) && (buf_rx[4] == SFM_ACK_SUCCESS)))
	{
		return buf_rx[4];
	}
	
	
	/* ���͵�2�� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x02;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	xReturn = xQueueReceive(g_queue_sfm,	/* ��Ϣ���еľ�� */
							buf_rx,			/* �õ�����Ϣ���� */
							1000); 			/* �ȴ�ʱ�� */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] sfm_reg_user error code is %d\r\n", xReturn);
		return -1;

	}	
	
	if(!((buf_rx[0] == 0xF5) && (buf_rx[1] == 0x02) && (buf_rx[4] == SFM_ACK_SUCCESS)))
	{
		return buf_rx[4];
	}	
	
	

	/* ���͵�3�� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x03;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	xReturn = xQueueReceive(g_queue_sfm,	/* ��Ϣ���еľ�� */
							buf_rx,			/* �õ�����Ϣ���� */
							1000); 			/* �ȴ�ʱ�� */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] sfm_reg_user error code is %d\r\n", xReturn);
		return -1;

	}	
	
	if(!((buf_rx[0] == 0xF5) && (buf_rx[1] == 0x03) && (buf_rx[4] == SFM_ACK_SUCCESS)))
	{
		return buf_rx[4];
	}
	
	if(buf_rx[2] == ((uint8_t)(id>>8)))
		if(buf_rx[3] == ((uint8_t)(id&0x00FF)))
			return SFM_ACK_SUCCESS;
		
	return buf_rx[2];
}

int32_t sfm_compare_users(uint16_t *id)
{
	uint8_t buf_tx[8]={0};
	uint8_t buf_rx[32]={0};	
	BaseType_t xReturn = pdFALSE;
	
	
	/* 1:N�ȶ� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x0C;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);

	
	xReturn = xQueueReceive(g_queue_sfm,	/* ��Ϣ���еľ�� */
							buf_rx,			/* �õ�����Ϣ���� */
							1000); 			/* �ȴ�ʱ�� */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] sfm_compare_users error code is %d\r\n", xReturn);
		return -1;

	}	
	if(!((buf_rx[0] == 0xF5) && (buf_rx[1] == 0x0C)))
	{
		return SFM_ACK_FAIL;
	}	
	
	/* �����û�id */
	*id = (buf_rx[2]<<8)|buf_rx[3];
	
	/* ��id = 0x0000����ʾ�ȶԲ��ɹ� */
	if(*id == 0x0000)
		return SFM_ACK_NOUSER;
	
	return SFM_ACK_SUCCESS;
}


int32_t sfm_get_unused_id(uint16_t *id)
{
	uint8_t buf_tx[8]={0};
	uint8_t buf_rx[32]={0};	
	BaseType_t xReturn = pdFALSE;
	
	/* ��ȡδʹ�õ��û����� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x0D;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	xReturn = xQueueReceive(g_queue_sfm,	/* ��Ϣ���еľ�� */
							buf_rx,			/* �õ�����Ϣ���� */
							1000); 			/* �ȴ�ʱ�� */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] sfm_compare_users error code is %d\r\n", xReturn);
		return -1;

	}	
	
	*id = (buf_rx[2]<<8)|buf_rx[3];
	
	if((buf_rx[0] == 0xF5) && (buf_rx[1] == 0x0D) && (buf_rx[4] == SFM_ACK_SUCCESS))
	{
		return SFM_ACK_SUCCESS;
	}

	return SFM_ACK_FAIL;
}

int32_t sfm_del_user(uint16_t id)
{
	uint8_t buf_tx[8]={0};
	uint8_t buf_rx[32]={0};	
	BaseType_t xReturn = pdFALSE;
	
	/* ɾ���û����� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x04;	
	buf_tx[2]=(uint8_t)(id>>8);	
	buf_tx[3]=(uint8_t)(id&0x00FF);	
	buf_tx[4]=0x01;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	xReturn = xQueueReceive(g_queue_sfm,	/* ��Ϣ���еľ�� */
							buf_rx,			/* �õ�����Ϣ���� */
							1000); 			/* �ȴ�ʱ�� */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] sfm_compare_users error code is %d\r\n", xReturn);
		return -1;

	}	
	
	if((buf_rx[0] == 0xF5) && (buf_rx[1] == 0x04) && (buf_rx[4] == SFM_ACK_SUCCESS))
	{
		return SFM_ACK_SUCCESS;
	}

	return SFM_ACK_FAIL;
}


int32_t sfm_del_user_all(void)
{
	uint8_t buf_tx[8]={0};
	uint8_t buf_rx[32]={0};	
	BaseType_t xReturn = pdFALSE;
	
	/* ɾ�������û����� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x05;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	xReturn = xQueueReceive(g_queue_sfm,	/* ��Ϣ���еľ�� */
							buf_rx,			/* �õ�����Ϣ���� */
							1000); 			/* �ȴ�ʱ�� */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] sfm_compare_users error code is %d\r\n", xReturn);
		return -1;

	}	
	
	if((buf_rx[0] == 0xF5) && (buf_rx[1] == 0x05) && (buf_rx[4] == SFM_ACK_SUCCESS))
	{
		return SFM_ACK_SUCCESS;
	}

	return SFM_ACK_FAIL;
}

int32_t sfm_get_user_total(uint16_t *user_total)
{
	uint8_t buf_tx[8]={0};
	uint8_t buf_rx[32]={0};	
	BaseType_t xReturn = pdFALSE;
	
	/* ɾ���û����� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x09;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	xReturn = xQueueReceive(g_queue_sfm,	/* ��Ϣ���еľ�� */
							buf_rx,			/* �õ�����Ϣ���� */
							1000); 			/* �ȴ�ʱ�� */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] sfm_compare_users error code is %d\r\n", xReturn);
		return -1;

	}	
	
	if(!((buf_rx[0] == 0xF5) && (buf_rx[1] == 0x09)))
	{
		return SFM_ACK_FAIL;
	}	
	
	/* �����û����� */
	*user_total = (buf_rx[2]<<8)|buf_rx[3];
	
	return SFM_ACK_SUCCESS;
}

int32_t sfm_touch_check(void)
{
	uint8_t buf_tx[8]={0};
	uint8_t buf_rx[32]={0};	
	BaseType_t xReturn = pdFALSE;
	
	/* ��ⴥ����Ӧ���� */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x30;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	xReturn = xQueueReceive(g_queue_sfm,	/* ��Ϣ���еľ�� */
							buf_rx,			/* �õ�����Ϣ���� */
							1000); 			/* �ȴ�ʱ�� */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] sfm_compare_users error code is %d\r\n", xReturn);
		return -1;

	}	
	
	if((buf_rx[0] == 0xF5) && (buf_rx[1] == 0x30) && (buf_rx[4] == SFM_ACK_SUCCESS))
	{
		return SFM_ACK_SUCCESS;
	}	
	
	return SFM_ACK_FAIL;
}

const char *sfm_error_code(uint8_t error_code)
{
	const char *p;
	
	switch(error_code)
	{
		case SFM_ACK_SUCCESS:p="ִ�гɹ�";
		break;
		
		case SFM_ACK_FAIL:p="ִ��ʧ��";
		break;	

		case SFM_ACK_FULL:p="���ݿ���";
		break;		

		case SFM_ACK_NOUSER:p="û������û�";
		break;		

		case SFM_ACK_USER_EXIST:p="�û��Ѵ���";
		break;	
		
		case SFM_ACK_TIMEOUT:p="ͼ��ɼ���ʱ";
		break;		
	
		case SFM_ACK_HARDWAREERROR:p="Ӳ������";
		break;	
		
		case SFM_ACK_IMAGEERROR:p="ͼ�����";
		break;	

		case SFM_ACK_BREAK:p="��ֹ��ǰָ��";
		break;	

		case SFM_ACK_ALGORITHMFAIL:p="��Ĥ�������";
		break;	
		
		case SFM_ACK_HOMOLOGYFAIL:p="ͬԴ��У�����";
		break;

		default :
			p="ģ�鷵��ȷ��������";break;
	}

	return p;
}

#if SFM_TOUCH_ENABLE 	
void EXTI9_5_IRQHandler(void)
{
	uint32_t ulReturn;
	
	BaseType_t  xHigherPriorityTaskWoken = pdFALSE;	
	
	/* �����ٽ�Σ��ٽ�ο���Ƕ�� */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();	
	
	//����־λ
	if(EXTI_GetITStatus(EXTI_Line6) == SET)
	{
		/* ��������û����룬�������¼���־��*/
		printf("EXTI9_5_IRQHandler\r\n");
		
		sfm_power_ctrl(1);
		
		xEventGroupSetBitsFromISR(g_event_group,EVENT_GROUP_SFM_USER_COMPARE,NULL);	
		
		EXTI_ClearITPendingBit(EXTI_Line6);
	}
	
	
	/* �˳��ٽ�� */
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );	
}
#endif
