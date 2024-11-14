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
【特别说明】
	当断掉电容指纹模块的3V3供电，该触摸感应才能生效，详细官方说明如下：
	为了达到最优的低功耗设计，实际应用时建议将模组的 VCC_3V3 保持断电状态，通过
	判断TOUCH_OUT 管脚信号控制该路电压开启或者关闭。即当TOUCH_OUT 呈现有效电平
	时，使能 VCC_3V3 电源，此时指纹模组进入到工作状态。
【硬件说明】	
   因此需要自行构建的一个三极管的控制电路
*/
void sfm_touch_init(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	EXTI_InitTypeDef	EXTI_InitStructure;	
	NVIC_InitTypeDef	NVIC_InitStructure;	
	
	//使能系统配置硬件时钟，说白了就是对系统配置的硬件供电
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);	
	
	//打开端口E的硬件时钟，就是对端口B供电
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);	

	//打开端口E的硬件时钟，就是对端口E供电
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB,&GPIO_InitStructure);		
	
	//配置GPIOA的第0根引脚
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	
	GPIO_Init(GPIOE,&GPIO_InitStructure);	
	
	
	//将PE6引脚连接到EXTI6
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource6);	
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line6;	//EXTI6
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			//中断触发
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; 	//上升沿沿就立即触发中断请求，通知CPU立即处理 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;					//使能
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//外部中断9-5的中断号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x05;		//响应优先级 0x05
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//NVIC使能
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

	/* 串口2 初始化 */
	usart2_init(baud);

	/* 光圈控制:全亮->全灭，周期2秒 */
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
	
	/* 发送设置光圈控制命令 */
	buf_tx[0]=0xF5;
	buf_tx[1]=0xC3;	
	buf_tx[2]=led_start;	
	buf_tx[3]=led_end;	
	buf_tx[4]=period;		
	buf_tx[5]=0;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	xReturn = xQueueReceive(g_queue_sfm,	/* 消息队列的句柄 */
							buf_rx,			/* 得到的消息内容 */
							1000); 			/* 等待时间 */
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
	
	/* 注册用户命令 */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x01;	
	buf_tx[2]=(uint8_t)(id>>8);	
	buf_tx[3]=(uint8_t)(id&0x00FF);	
	buf_tx[4]=0x01;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	xReturn = xQueueReceive(g_queue_sfm,	/* 消息队列的句柄 */
							buf_rx,			/* 得到的消息内容 */
							1000); 			/* 等待时间 */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] sfm_reg_user error code is %d\r\n", xReturn);
		return -1;

	}	
	
	if(!((buf_rx[0] == 0xF5) && (buf_rx[1] == 0x01) && (buf_rx[4] == SFM_ACK_SUCCESS)))
	{
		return buf_rx[4];
	}
	
	
	/* 发送第2次 */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x02;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	xReturn = xQueueReceive(g_queue_sfm,	/* 消息队列的句柄 */
							buf_rx,			/* 得到的消息内容 */
							1000); 			/* 等待时间 */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] sfm_reg_user error code is %d\r\n", xReturn);
		return -1;

	}	
	
	if(!((buf_rx[0] == 0xF5) && (buf_rx[1] == 0x02) && (buf_rx[4] == SFM_ACK_SUCCESS)))
	{
		return buf_rx[4];
	}	
	
	

	/* 发送第3次 */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x03;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	xReturn = xQueueReceive(g_queue_sfm,	/* 消息队列的句柄 */
							buf_rx,			/* 得到的消息内容 */
							1000); 			/* 等待时间 */
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
	
	
	/* 1:N比对 */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x0C;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);

	
	xReturn = xQueueReceive(g_queue_sfm,	/* 消息队列的句柄 */
							buf_rx,			/* 得到的消息内容 */
							1000); 			/* 等待时间 */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] sfm_compare_users error code is %d\r\n", xReturn);
		return -1;

	}	
	if(!((buf_rx[0] == 0xF5) && (buf_rx[1] == 0x0C)))
	{
		return SFM_ACK_FAIL;
	}	
	
	/* 返回用户id */
	*id = (buf_rx[2]<<8)|buf_rx[3];
	
	/* 若id = 0x0000，表示比对不成功 */
	if(*id == 0x0000)
		return SFM_ACK_NOUSER;
	
	return SFM_ACK_SUCCESS;
}


int32_t sfm_get_unused_id(uint16_t *id)
{
	uint8_t buf_tx[8]={0};
	uint8_t buf_rx[32]={0};	
	BaseType_t xReturn = pdFALSE;
	
	/* 获取未使用的用户命令 */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x0D;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	xReturn = xQueueReceive(g_queue_sfm,	/* 消息队列的句柄 */
							buf_rx,			/* 得到的消息内容 */
							1000); 			/* 等待时间 */
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
	
	/* 删除用户命令 */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x04;	
	buf_tx[2]=(uint8_t)(id>>8);	
	buf_tx[3]=(uint8_t)(id&0x00FF);	
	buf_tx[4]=0x01;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	xReturn = xQueueReceive(g_queue_sfm,	/* 消息队列的句柄 */
							buf_rx,			/* 得到的消息内容 */
							1000); 			/* 等待时间 */
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
	
	/* 删除所有用户命令 */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x05;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	xReturn = xQueueReceive(g_queue_sfm,	/* 消息队列的句柄 */
							buf_rx,			/* 得到的消息内容 */
							1000); 			/* 等待时间 */
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
	
	/* 删除用户命令 */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x09;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	xReturn = xQueueReceive(g_queue_sfm,	/* 消息队列的句柄 */
							buf_rx,			/* 得到的消息内容 */
							1000); 			/* 等待时间 */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] sfm_compare_users error code is %d\r\n", xReturn);
		return -1;

	}	
	
	if(!((buf_rx[0] == 0xF5) && (buf_rx[1] == 0x09)))
	{
		return SFM_ACK_FAIL;
	}	
	
	/* 返回用户总数 */
	*user_total = (buf_rx[2]<<8)|buf_rx[3];
	
	return SFM_ACK_SUCCESS;
}

int32_t sfm_touch_check(void)
{
	uint8_t buf_tx[8]={0};
	uint8_t buf_rx[32]={0};	
	BaseType_t xReturn = pdFALSE;
	
	/* 检测触摸感应命令 */
	buf_tx[0]=0xF5;
	buf_tx[1]=0x30;	
	buf_tx[2]=0x00;	
	buf_tx[3]=0x00;	
	buf_tx[4]=0x00;		
	buf_tx[5]=0x00;
	buf_tx[6]=bcc_check(&buf_tx[1],5);
	buf_tx[7]=0xF5;
	
	usart_send_bytes(USART2,buf_tx,8);
	
	xReturn = xQueueReceive(g_queue_sfm,	/* 消息队列的句柄 */
							buf_rx,			/* 得到的消息内容 */
							1000); 			/* 等待时间 */
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
		case SFM_ACK_SUCCESS:p="执行成功";
		break;
		
		case SFM_ACK_FAIL:p="执行失败";
		break;	

		case SFM_ACK_FULL:p="数据库满";
		break;		

		case SFM_ACK_NOUSER:p="没有这个用户";
		break;		

		case SFM_ACK_USER_EXIST:p="用户已存在";
		break;	
		
		case SFM_ACK_TIMEOUT:p="图像采集超时";
		break;		
	
		case SFM_ACK_HARDWAREERROR:p="硬件错误";
		break;	
		
		case SFM_ACK_IMAGEERROR:p="图像错误";
		break;	

		case SFM_ACK_BREAK:p="终止当前指令";
		break;	

		case SFM_ACK_ALGORITHMFAIL:p="贴膜攻击检测";
		break;	
		
		case SFM_ACK_HOMOLOGYFAIL:p="同源性校验错误";
		break;

		default :
			p="模块返回确认码有误";break;
	}

	return p;
}

#if SFM_TOUCH_ENABLE 	
void EXTI9_5_IRQHandler(void)
{
	uint32_t ulReturn;
	
	BaseType_t  xHigherPriorityTaskWoken = pdFALSE;	
	
	/* 进入临界段，临界段可以嵌套 */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();	
	
	//检测标志位
	if(EXTI_GetITStatus(EXTI_Line6) == SET)
	{
		/* 自行添加用户代码，如设置事件标志组*/
		printf("EXTI9_5_IRQHandler\r\n");
		
		sfm_power_ctrl(1);
		
		xEventGroupSetBitsFromISR(g_event_group,EVENT_GROUP_SFM_USER_COMPARE,NULL);	
		
		EXTI_ClearITPendingBit(EXTI_Line6);
	}
	
	
	/* 退出临界段 */
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );	
}
#endif
