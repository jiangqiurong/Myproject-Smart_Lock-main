#include "includes.h"

static uint8_t bcc_check(uint8_t *buf,uint32_t len)
{
	uint8_t s=0;
	uint8_t i=0;
	uint8_t *p = buf;
	
	for(i=0; i<len; i++)
		s = s^p[i];

	return s;
}


static void tim3_init(void)
{
	//ʹ��tim3��Ӳ��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	//����tim3�ķ�Ƶֵ������ֵ
	//tim3Ӳ��ʱ��=84MHz/8400=10000Hz�����ǽ���10000�μ���������1��ʱ��ĵ���

	TIM_TimeBaseStructure.TIM_Period = 10000/100-1; //����ֵ0 -> 99����10����ʱ��ĵ���
	TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;	//Ԥ��Ƶֵ8400
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;	//ʱ�ӷ�Ƶ����ǰ��û�еģ�����Ҫ��������
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	//����tim3���ж�
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
    /* �����ʱ��3��ʱ����±�־λ */
    TIM_ClearITPendingBit(TIM3,TIM_IT_Update);	
	
	/* �رն�ʱ��3 */
    TIM_Cmd(TIM3, DISABLE); 
}


static void fr_printf_recv(uint8_t *buf,uint32_t len)
{
	
#if FR_DEBUG_EN
	
	uint8_t *p=buf;
	uint32_t i;
	
	printf("fr recv buf:");
	
	for(i=0;i<len;i++)
	{
		printf("%02X ",p[i]);
	
	}
	
	printf("\r\n");
	
#else
	
	(void)0;
	
#endif
}

static void fr_reply_info(uint8_t reply)
{
#if FR_DEBUG_EN
	if(reply==0)dgb_printf_safe("[����ʶ��ģ�� REPLY] �ɹ�\r\n");
	if(reply==1)dgb_printf_safe("[����ʶ��ģ�� REPLY] ģ��ܾ�������\r\n");
	if(reply==2)dgb_printf_safe("[����ʶ��ģ�� REPLY] ¼��/ƥ���㷨����ֹ\r\n");
	if(reply==3)dgb_printf_safe("[����ʶ��ģ�� REPLY] ������Ϣ����\r\n");
	if(reply==4)dgb_printf_safe("[����ʶ��ģ�� REPLY] �����ʧ��\r\n");	
	if(reply==5)dgb_printf_safe("[����ʶ��ģ�� REPLY] δ֪����\r\n");	
	if(reply==6)dgb_printf_safe("[����ʶ��ģ�� REPLY] ��Ч�Ĳ���\r\n");
	if(reply==7)dgb_printf_safe("[����ʶ��ģ�� REPLY] �ڴ治��\r\n");
	if(reply==8)dgb_printf_safe("[����ʶ��ģ�� REPLY] û����¼����û�\r\n");
	if(reply==9)dgb_printf_safe("[����ʶ��ģ�� REPLY] ¼�볬������û�����\r\n");
	if(reply==10)dgb_printf_safe("[����ʶ��ģ�� REPLY] ������¼��\r\n");		
	if(reply==12)dgb_printf_safe("[����ʶ��ģ�� REPLY] ������ʧ��\r\n");
	if(reply==13)dgb_printf_safe("[����ʶ��ģ�� REPLY] ¼��������ʱ\r\n");	
	if(reply==14)dgb_printf_safe("[����ʶ��ģ�� REPLY] ����оƬ��Ȩʧ��\r\n");	
	if(reply==19)dgb_printf_safe("[����ʶ��ģ�� REPLY] ���ļ�ʧ��\r\n");	
	if(reply==20)dgb_printf_safe("[����ʶ��ģ�� REPLY] д�ļ�ʧ��\r\n");		
	if(reply==21)dgb_printf_safe("[����ʶ��ģ�� REPLY] ͨ��Э��δ����\r\n");	
	if(reply==23)dgb_printf_safe("[����ʶ��ģ�� REPLY] RGBͼ��û��ready\r\n");
	if(reply==24)dgb_printf_safe("[����ʶ��ģ�� REPLY] JPG��Ƭ������Ƭע�ᣩ\r\n");	
	if(reply==25)dgb_printf_safe("[����ʶ��ģ�� REPLY] JPG��Ƭ��С����Ƭע�ᣩ\r\n");

#else
	
	(void)0;
	
#endif		
}

static void fr_note_info(uint8_t note)
{
#if FR_DEBUG_EN
	if(note==0)dgb_printf_safe("[����ʶ��ģ�� NOTE] ��������\r\n");
	if(note==1)dgb_printf_safe("[����ʶ��ģ�� NOTE] δ��⵽����\r\n");
	if(note==2)dgb_printf_safe("[����ʶ��ģ�� NOTE] ����̫����ͼƬ�ϱ��أ�δ��¼��\r\n");
	if(note==3)dgb_printf_safe("[����ʶ��ģ�� NOTE] ����̫����ͼƬ�±��أ�δ��¼��\r\n");
	if(note==4)dgb_printf_safe("[����ʶ��ģ�� NOTE] ����̫����ͼƬ����أ�δ��¼��\r\n");	
	if(note==5)dgb_printf_safe("[����ʶ��ģ�� NOTE] ����̫����ͼƬ�ұ��أ�δ��¼��\r\n");	
	if(note==6)dgb_printf_safe("[����ʶ��ģ�� NOTE] ��������̫Զ��δ��¼��\r\n");
	if(note==7)dgb_printf_safe("[����ʶ��ģ�� NOTE] ��������̫����δ��¼��\r\n");
	if(note==8)dgb_printf_safe("[����ʶ��ģ�� NOTE] üë�ڵ�\r\n");
	if(note==9)dgb_printf_safe("[����ʶ��ģ�� NOTE] �۾��ڵ�\r\n");
	if(note==10)dgb_printf_safe("[����ʶ��ģ�� NOTE] �����ڵ�\r\n");		
	if(note==11)dgb_printf_safe("[����ʶ��ģ�� NOTE] ¼�������������\r\n");	
	if(note==12)dgb_printf_safe("[����ʶ��ģ�� NOTE] �ڱ���ģʽ��⵽����״̬\r\n");
	if(note==13)dgb_printf_safe("[����ʶ��ģ�� NOTE] ����״̬\r\n");	
	if(note==14)dgb_printf_safe("[����ʶ��ģ�� NOTE] �ڱ���ģʽ������޷��ж�������״̬\r\n");

#else
	(void)0;
#endif	
}

void fr_send(uint8_t *buf,uint32_t len)
{
	uint8_t *p = buf;


	g_usart2_rx_end = 0;
	g_usart2_rx_cnt=0;
	
	while(len--)
	{
		USART_SendData(USART2,*p++);
		
		while(!USART_GetFlagStatus(USART2,USART_FLAG_TXE));
	}
	

}



int32_t fr_reg_user(const char *name,uint8_t is_admin)
{
	uint8_t buf_tx[64]={0};
	uint8_t buf_rx[USART_PACKET_SIZE]={0};		
    uint32_t timeout=10000;	
	
	BaseType_t xReturn = pdFALSE;	
	
	/* ���ݰ�����ͷ */
	buf_tx[0]=0xEF;
	buf_tx[1]=0xAA;
	
	/* ���ݰ����������ע�� */
	buf_tx[2]=0x1D;
	
	/* ���ݰ������ݳ��� */	
	buf_tx[3]=0x00;
	buf_tx[4]=0x23;
	
	/* ���ݰ��������û���ɫ,����Աд1����ͨ�û�д0 */	
	buf_tx[5]=is_admin;

	/* ���ݰ����û��� */
	if(name!=NULL)
		memcpy(&buf_tx[6],name,strlen(name));


	/* ���ݰ�����������Ĭ�ϲ���ʹ�� */
	buf_tx[38]=0x00;	
	
	
	/* ���ݰ�����ʱʱ�� */
	buf_tx[39]=0x0A;	
	
	
	/* ���ݰ���У��λ */
	buf_tx[40]=bcc_check(&buf_tx[2],38);
	
	fr_send(buf_tx,41);
	
	while(1)
	{
	
		xQueueReset(g_queue_frm);
		
		xReturn = xQueueReceive(g_queue_frm,	/* ��Ϣ���еľ�� */
								buf_rx,			/* �õ�����Ϣ���� */
								timeout); 		/* �ȴ�ʱ�� */
		if (xReturn != pdPASS)
		{
			dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
			break;

		}	
		
		fr_printf_recv(buf_rx,USART_PACKET_SIZE);		

		if((buf_rx[0]==0xEF) && (buf_rx[1]==0xAA))
		{
			//��Ϣ����ΪNOTE
			if(buf_rx[2]==1)
			{		
				
				fr_note_info(buf_rx[6]);
				
				continue;
		
			}	

			//��Ϣ����ΪREPLY
			if(buf_rx[2]==0)
			{
				fr_reply_info(buf_rx[6]);			
				if(buf_rx[6])
					return buf_rx[6];
				
				return 0;
				
			}		
		}	
	
	
	}

		
	return -1;	
}

int32_t fr_del_user_all(void)
{
	uint8_t buf_tx[8]={0};
	uint8_t buf_rx[USART_PACKET_SIZE]={0};		
    uint32_t timeout=10000;	
	
	BaseType_t xReturn = pdFALSE;	
	
	buf_tx[0]=0xEF;
	buf_tx[1]=0xAA;
	
	buf_tx[2]=0x21;
	
	buf_tx[3]=0x00;
	buf_tx[4]=0x00;
	
	buf_tx[5]=0x21;	

	fr_send(buf_tx,6);
	
	
	xQueueReset(g_queue_frm);
	
	xReturn = xQueueReceive(g_queue_frm,	/* ��Ϣ���еľ�� */
							buf_rx,			/* �õ�����Ϣ���� */
							timeout); 		/* �ȴ�ʱ�� */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
		return -1;

	}	
	
	fr_printf_recv(buf_rx,USART_PACKET_SIZE);		
		
	if((buf_rx[0]==0xEF) && (buf_rx[1]==0xAA)  && (buf_rx[5]==0x21) )
	{
		
		if(buf_rx[6])
		{
			if(buf_rx[2]==0)
			{
				fr_reply_info(buf_rx[6]);
			}
			
			if(buf_rx[2]==1)
			{
				fr_note_info(buf_rx[6]);
			}			
			
			return buf_rx[6];
		}			
		else
			return 0;
	}
		
	return -1;			
}

int32_t fr_match(uint8_t *buf)
{
	uint8_t buf_tx[8]={0};
	static uint8_t buf_rx[USART_PACKET_SIZE]={0};		
    uint32_t timeout=10000;	
	
	BaseType_t xReturn = pdFALSE;	
		
	
	buf_tx[0]=0xEF;
	buf_tx[1]=0xAA;
	
	buf_tx[2]=0x12;
	
	buf_tx[3]=0x00;
	buf_tx[4]=0x02;
	
	/* �����ɹ����Ƿ����̶�,1:yes  0 no */
	buf_tx[5]=0x01;	
	
	/* ������ʱʱ��(��λ��) */
	buf_tx[6]=0x0A;	

	/* ���ݰ���У��λ */
	buf_tx[7]=bcc_check(&buf_tx[2],5);
	
	fr_send(buf_tx,8);
	
	
	while(1)
	{
		timeout=10000;
		
		xQueueReset(g_queue_frm);
		
		xReturn = xQueueReceive(g_queue_frm,	/* ��Ϣ���еľ�� */
								buf_rx,			/* �õ�����Ϣ���� */
								timeout); 		/* �ȴ�ʱ�� */
		if (xReturn != pdPASS)
		{
			dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
			break;

		}	
		
		fr_printf_recv(buf_rx,USART_PACKET_SIZE);		
			
		if((buf_rx[0]==0xEF) && (buf_rx[1]==0xAA))
		{
			
			//��Ϣ����ΪNOTE
			if(buf_rx[2]==1)
			{		
				
				fr_note_info(buf_rx[6]);
				
				continue;
		
			}	

			//��Ϣ����ΪREPLY
			if(buf_rx[2]==0)
			{
				fr_reply_info(buf_rx[6]);			
				if(buf_rx[6])
					return buf_rx[6];
				
				memcpy(buf,(void *)&buf_rx[7],36);
				return 0;
				
			}			

		}	
	
	
	}

		
	return -1;			
}

int32_t fr_state_get(void)
{
	uint8_t buf_tx[6]={0};
	uint8_t buf_rx[USART_PACKET_SIZE]={0};		
    uint32_t timeout=10000;	
	
	BaseType_t xReturn = pdFALSE;	
		
	buf_tx[0]=0xEF;
	buf_tx[1]=0xAA;
	buf_tx[2]=0x11;
	buf_tx[3]=0x00;
	buf_tx[4]=0x00;
	buf_tx[5]=0x11;	

	fr_send(buf_tx,6);
	
	
	xQueueReset(g_queue_frm);
	
	xReturn = xQueueReceive(g_queue_frm,	/* ��Ϣ���еľ�� */
							buf_rx,			/* �õ�����Ϣ���� */
							timeout); 		/* �ȴ�ʱ�� */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
		return -1;

	}	
	
	fr_printf_recv(buf_rx,USART_PACKET_SIZE);		

	
	return 0;

}


int32_t fr_get_user_total(void)
{
	int32_t user_total=0;
    uint32_t timeout=4000;	
	uint8_t buf_rx[USART_PACKET_SIZE]={0};		
	BaseType_t xReturn = pdFALSE;	
		
	uint8_t buf_tx[6]={0};

		
	buf_tx[0]=0xEF;
	buf_tx[1]=0xAA;
	
	buf_tx[2]=0x24;
	
	buf_tx[3]=0x00;
	buf_tx[4]=0x00;
	
	buf_tx[5]=0x24;	


	fr_send(buf_tx,6);
	

	xQueueReset(g_queue_frm);
	
	xReturn = xQueueReceive(g_queue_frm,	/* ��Ϣ���еľ�� */
							buf_rx,			/* �õ�����Ϣ���� */
							timeout); 		/* �ȴ�ʱ�� */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
		return -1;

	}	
	
	fr_printf_recv(buf_rx,USART_PACKET_SIZE);		
	
	if((buf_rx[0]==0xEF) && (buf_rx[1]==0xAA))
	{
		if((buf_rx[2]==0) && (buf_rx[5]==0x24) && (buf_rx[6]==0))
		{
			user_total = buf_rx[7];
			
			return user_total;
		}	
	}

	return -1;	

}

int32_t fr_reset(void)
{
	uint8_t buf_tx[6]={0};
	uint8_t buf_rx[USART_PACKET_SIZE]={0};		
    uint32_t timeout=10000;	
	
	BaseType_t xReturn = pdFALSE;	
		
	buf_tx[0]=0xEF;
	buf_tx[1]=0xAA;
	buf_tx[2]=0x10;
	buf_tx[3]=0x00;
	buf_tx[4]=0x00;
	buf_tx[5]=0x10;	


	fr_send(buf_tx,6);

	xQueueReset(g_queue_frm);
	
	xReturn = xQueueReceive(g_queue_frm,	/* ��Ϣ���еľ�� */
							buf_rx,			/* �õ�����Ϣ���� */
							timeout); 		/* �ȴ�ʱ�� */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
		return -1;

	}	
	
	fr_printf_recv(buf_rx,USART_PACKET_SIZE);	
	
	if((buf_rx[0]==0xEF) && (buf_rx[1]==0xAA))
	{
		if((buf_rx[2]==0) && (buf_rx[5]==0x10) && (buf_rx[6]==0x00))
		{
			
			return 0;
		}	
	}	
	
	return -1;
}

int32_t fr_power_down(void)
{
	uint8_t buf_tx[6]={0};
	uint8_t buf_rx[USART_PACKET_SIZE]={0};		
    uint32_t timeout=10000;	
	
	BaseType_t xReturn = pdFALSE;	
		
	buf_tx[0]=0xEF;
	buf_tx[1]=0xAA;
	buf_tx[2]=0xED;
	buf_tx[3]=0x00;
	buf_tx[4]=0x00;
	buf_tx[5]=0xED;

	fr_send(buf_tx,6);

	xQueueReset(g_queue_frm);
	
	xReturn = xQueueReceive(g_queue_frm,	/* ��Ϣ���еľ�� */
							buf_rx,			/* �õ�����Ϣ���� */
							timeout); 		/* �ȴ�ʱ�� */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
		return -1;

	}	
	
	fr_printf_recv(buf_rx,USART_PACKET_SIZE);		
	
	if((buf_rx[0]==0xEF) && (buf_rx[1]==0xAA))
	{
		if((buf_rx[2]==0) && (buf_rx[5]==0xED) && (buf_rx[6]==0x00))
		{
			
			return 0;
		}	
	}	
	
	
	return -1;
}

int32_t fr_init(uint32_t baud)
{
	/* ��ʱ�� ��ʼ�� */
	tim3_init();
	
	/* ����2 ��ʼ������ģ���ϵ���������50ms����ܶԸ�ģ�鷢������ */
	usart2_init(baud);
	
	delay_ms(500);
	
	return fr_reset();
}

void TIM3_IRQHandler(void)
{
	
	uint32_t ulReturn;
	
	BaseType_t xHigherPriorityTaskWoken;	
	
	/* �����ٽ�Σ��ٽ�ο���Ƕ�� */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();	
	
	//����־λ
	if(TIM_GetITStatus(TIM3,TIM_IT_Update) == SET)
	{

		if(g_usart2_rx_cnt)
		{
			/* �Ӷ���ͷ�������� */
			xQueueSendToFrontFromISR(g_queue_frm,(void *)&g_usart2_rx_buf,&xHigherPriorityTaskWoken);
			memset((void *)g_usart2_rx_buf,0,sizeof(g_usart2_rx_buf));
			g_usart2_rx_cnt=0;		
		}


		/* �رն�ʱ��3 */
		TIM_Cmd(TIM3, DISABLE);
		
		//��ձ�־λ
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	}
	
	/* ��������Ϣ������������ȼ����ڵ�ǰ���е��������˳��жϺ��������������л���ִ��ǰ��;
	   ����ȴ���һ��ʱ�ӽ��ĲŽ��������л�
	*/
    if( xHigherPriorityTaskWoken )
    {
		
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }		
	
	/* �˳��ٽ�� */
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );		
}


