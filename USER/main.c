#include "includes.h"

GPIO_InitTypeDef GPIO_InitStructure;
EXTI_InitTypeDef	EXTI_InitStructure;
USART_InitTypeDef USART_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
RTC_TimeTypeDef RTC_TimeStructure;
RTC_DateTypeDef  RTC_DateStructure;
RTC_InitTypeDef  RTC_InitStructure;
RTC_AlarmTypeDef RTC_AlarmStructure;
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

static TaskHandle_t app_task_init_handle = NULL;
static TaskHandle_t app_task_rtc_handle = NULL;
static TaskHandle_t app_task_key_handle = NULL;
static TaskHandle_t app_task_frm_handle = NULL;
static TaskHandle_t app_task_sr04_handle = NULL;
static TaskHandle_t app_task_motor_handle = NULL;
static TaskHandle_t app_task_usart_handle = NULL;


/* ����:��ʼ�� */ 
void app_task_init(void* pvParameters);  

/* ����:ʵʱʱ����ʾ */ 
void app_task_rtc(void* pvParameters); 

/* ����:���� */  
void app_task_key(void* pvParameters); 

/* ����:����ʶ�� */  
void app_task_frm(void* pvParameters); 

/* ����:��������� */  
void app_task_sr04(void* pvParameters); 

/* ����:��������������� */  
void app_task_motor(void* pvParameters); 

/* ����:app_task_usart */ 
void app_task_usart(void* pvParameters);  

/* �������ź������ */
SemaphoreHandle_t g_mutex_printf;

SemaphoreHandle_t g_mutex_oled;

SemaphoreHandle_t  g_sem_rtc;

/* �¼���־���� */
EventGroupHandle_t g_event_group;	

EventGroupHandle_t g_event_task_sta;

/* ��Ϣ���о�� */
QueueHandle_t g_queue_frm;

QueueHandle_t g_queue_usart;

/* printf�������߶ȷ�װ */
void dgb_printf_safe(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	
	/* ��ȡ�����ź��� */
	xSemaphoreTake(g_mutex_printf,portMAX_DELAY);
	
	vprintf(format, args);
			
	/* �ͷŻ����ź��� */
	xSemaphoreGive(g_mutex_printf);	

	va_end(args);
}

//����һ������״̬�ṹ�����ڱ�ʶ�����״̬
struct task_status g_task_sta;


int main(void)
{
	/* ����ϵͳ�ж����ȼ�����4 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	/* ϵͳ��ʱ���ж�Ƶ��ΪconfigTICK_RATE_HZ */
	SysTick_Config(SystemCoreClock/configTICK_RATE_HZ);										
	
	/* ��ʼ������1 */
	usart1_init(9600);     
	
	/* ��ʼ������3 */
	usart3_init(9600); 

	
	/* ����app_task_init���� */
	xTaskCreate((TaskFunction_t )app_task_init,  		/* ������ں��� */
			  (const char*    )"app_task_init",			/* �������� */
			  (uint16_t       )512,  				/* ����ջ��С */
			  (void*          )NULL,				/* ������ں������� */
			  (UBaseType_t    )5, 					/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_init_handle);	/* ������ƿ�ָ�� */ 
	
	
	/* ����������� */
	vTaskStartScheduler(); 
			  
	while(1);

}

void app_task_init(void* pvParameters)
{
	/* ��ʼ������ģ�� */
	asr_init(9600);
	
	/* ��������ʼ�� */	
	beep_init();
	
	/* ��ʪ�ȳ�ʼ�� */
	dht11_init();
	
	/* rtc��ʼ�� */
	rtc_init();
	
	/* ������ʼ�� */
	key_init();
	
	/* LED��ʼ�� */
	led_init();
	
	/* oled��ʼ�� */
	OLED_Init();
	
	OLED_Clear();
	
	OLED_ShowCHinese(2, 3, 0);  //��
	OLED_ShowCHinese(20, 3, 1); //��
	OLED_ShowCHinese(38, 3, 2); //��
	OLED_ShowCHinese(56, 3, 3); //��
	OLED_ShowCHinese(74, 3, 4); //��
	OLED_ShowCHinese(92, 3, 5); //��
	OLED_ShowCHinese(110, 3, 6);//��
	
	/* SR04��ʼ�� */
	sr04_init();	
	
	/* �����ʼ�� */
	motor_init();
	
	
	/* ���������� */	  
	g_mutex_printf = xSemaphoreCreateMutex();
	
	g_mutex_oled = xSemaphoreCreateMutex();	
	
	//������ֵ�ź���
	g_sem_rtc = xSemaphoreCreateBinary();
	
	/* �����¼���־�� */
	g_event_group = xEventGroupCreate();
	
	g_event_task_sta = xEventGroupCreate();

	/* ������Ϣ���� */
	g_queue_frm = xQueueCreate(16, sizeof(g_usart2_rx_buf));	
	
	//������Ϣ���У����г���Ϊ5��Ԫ�ش�СΪ32�ֽ�
	g_queue_usart = xQueueCreate(5,32);
	
	/* �����ٽ�����ʵ���ϻ�ֹͣ�ں˵��Ȼ��йر��ж� */
	taskENTER_CRITICAL();
	
	//����״̬��ʼ��
	g_task_sta.task_rtc = 1;
	g_task_sta.task_key = 1;
	g_task_sta.task_sr04 = 1;
	g_task_sta.task_frm = 1;
	g_task_sta.task_motor = 1;
	g_task_sta.task_usart = 1;
	
	/* ����app_task_rtc���� */		  
	xTaskCreate((TaskFunction_t )app_task_rtc,  		/* ������ں��� */
			  (const char*    )"app_task_rtc",			/* �������� */
			  (uint16_t       )512,  					/* ����ջ��С */
			  (void*          )NULL,					/* ������ں������� */
			  (UBaseType_t    )5, 						/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_rtc_handle);	/* ������ƿ�ָ�� */	
			  
	/* ����app_task_key���� */		  
	xTaskCreate((TaskFunction_t )app_task_key,  		/* ������ں��� */
			  (const char*    )"app_task_key",			/* �������� */
			  (uint16_t       )512,  					/* ����ջ��С */
			  (void*          )NULL,					/* ������ں������� */
			  (UBaseType_t    )6, 						/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_key_handle);	/* ������ƿ�ָ�� */  
			  
	/* ����app_task_frm���� */		  
	xTaskCreate((TaskFunction_t )app_task_frm,  		/* ������ں��� */
			  (const char*    )"app_task_frm",			/* �������� */
			  (uint16_t       )1024,  					/* ����ջ��С */
			  (void*          )NULL,					/* ������ں������� */
			  (UBaseType_t    )7, 						/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_frm_handle);	/* ������ƿ�ָ�� */				  
	
	/* ����app_task_sr04���� */		  
	xTaskCreate((TaskFunction_t )app_task_sr04,  		/* ������ں��� */
			  (const char*    )"app_task_sr04",			/* �������� */
			  (uint16_t       )512,  					/* ����ջ��С */
			  (void*          )NULL,					/* ������ں������� */
			  (UBaseType_t    )7, 						/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_sr04_handle);	/* ������ƿ�ָ�� */		
			  
	/* ����app_task_motor���� */		  
	xTaskCreate((TaskFunction_t )app_task_motor,  		/* ������ں��� */
			  (const char*    )"app_task_motor",			/* �������� */
			  (uint16_t       )512,  					/* ����ջ��С */
			  (void*          )NULL,					/* ������ں������� */
			  (UBaseType_t    )7, 						/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_motor_handle);	/* ������ƿ�ָ�� */	
			  
	/* ����app_task_usart���� */		  
	xTaskCreate((TaskFunction_t )app_task_usart,  		/* ������ں��� */
			  (const char*    )"app_task_usart",			/* �������� */
			  (uint16_t       )512,  					/* ����ջ��С */
			  (void*          )NULL,					/* ������ں������� */
			  (UBaseType_t    )7, 						/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_usart_handle);	/* ������ƿ�ָ�� */	
	
			  
	/* �˳��ٽ�����ʵ���ϻָ��ں˵��Ȼ��д��ж� */
	taskEXIT_CRITICAL();
			  
	//����ʼ��������ϣ���ǰ����Ҫɾ�����ͷ�ռ�õ��ڴ�ռ�
	vTaskDelete(NULL);		  
}   

void app_task_rtc(void* pvParameters)
{	
	uint8_t buf[16]={0};

	while(1)
	{
		//���g_task_sta.task_rtcΪ0�����������ı���
		if(g_task_sta.task_rtc == 0)
		{
			//�����¼������ߵȴ����¼��ĺ�����RTC����׼������
			xEventGroupSetBits(g_event_task_sta, EVENT_TASK_RTC_SUSPEND);
			
			vTaskSuspend(NULL);
		}
		
		
		//�ȴ���ֵ�ź���������rtc����
		xSemaphoreTake(g_sem_rtc,portMAX_DELAY);
		
		RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
		
		memset(buf, 0, sizeof buf);

				/* ��ʽ���ַ��� */
		sprintf((char *)buf, "%02d:%02d:%02d", RTC_TimeStructure.RTC_Hours,
				RTC_TimeStructure.RTC_Minutes,
				RTC_TimeStructure.RTC_Seconds);
		
		dgb_printf_safe("[app_task_rtc][info]: %s\r\n",buf);

		OLED_SAFE(
		
		OLED_ShowString(0,0,buf,16);			
		
		);
	}	
}

void app_task_key(void* pvParameters)
{
	EventBits_t EventValue=0;
		
	key_init();
	
	for(;;)
	{	
		//�ȴ��¼����е���Ӧ�¼�λ����ͬ��
		EventValue=xEventGroupWaitBits((EventGroupHandle_t	)g_event_group,		
									   (EventBits_t			)0x0F,
									   (BaseType_t			)pdTRUE,				
									   (BaseType_t			)pdFALSE,
									   (TickType_t			)portMAX_DELAY);
		//��ʱ����
		vTaskDelay(50);		
		
		if(EventValue & EVENT_GROUP_KEY1_DOWN)
		{
			//��ֹEXTI0�����ж�
			NVIC_DisableIRQ(EXTI0_IRQn);
			
			//ȷ���ǰ���
			if(PAin(0) == 0)
			{
				dgb_printf_safe("S1 Press\r\n");
				
				//�ȴ������ͷ�
				while(PAin(0)==0)
					vTaskDelay(20);	

				xEventGroupSetBits(g_event_group,EVENT_GROUP_FRM_USER_REG);
			
			}
				
			//����EXTI0�����ж�
			NVIC_EnableIRQ(EXTI0_IRQn);					
		}
		
		if(EventValue & EVENT_GROUP_KEY2_DOWN)
		{
			//��ֹEXTI2�����ж�
			NVIC_DisableIRQ(EXTI2_IRQn);

				
			if(PEin(2) == 0)
			{
				dgb_printf_safe("S2 Press\r\n");
				
				//�ȴ������ͷ�
				while(PEin(2)==0)
					vTaskDelay(20);
			
				xEventGroupSetBits(g_event_group,EVENT_GROUP_FRM_USER_COMPARE);				
			
			}

			//����EXTI2�����ж�
			NVIC_EnableIRQ(EXTI2_IRQn);	
		}	
		
		if(EventValue & EVENT_GROUP_KEY3_DOWN)
		{
			//��ֹEXTI3�����ж�
			NVIC_DisableIRQ(EXTI3_IRQn);
			
				
			if(PEin(3) == 0)	
			{
				dgb_printf_safe("S3 Press\r\n");
				
				//�ȴ������ͷ�
				while(PEin(3)==0)
					vTaskDelay(20);

				xEventGroupSetBits(g_event_group,EVENT_GROUP_FRM_USER_TOTAL);
			}
				
			//����EXTI3�����ж�
			NVIC_EnableIRQ(EXTI3_IRQn);	
		}
		
		if(EventValue & EVENT_GROUP_KEY4_DOWN)
		{
			//��ֹEXTI4�����ж�
			NVIC_DisableIRQ(EXTI4_IRQn);
				
			if(PEin(4) == 0)	
			{
				dgb_printf_safe("S4 Press\r\n");
				
				
				//�ȴ������ͷ�
				while(PEin(4)==0)
					vTaskDelay(20);
			
				xEventGroupSetBits(g_event_group,EVENT_GROUP_FRM_USER_DEL_ALL);				
			}
				
			//����EXTI4�����ж�
			NVIC_EnableIRQ(EXTI4_IRQn);	
		}
	}
} 

void app_task_sr04(void* pvParameters)
{
	int32_t distance=0;	
	
	uint32_t task_suspend = 0;
	
	EventBits_t EventValue=0;	
	
	EventBits_t SuspendValue = 0;//�ж������Ƿ񶼹���ı�־λ
	
	for(;;)
	{
		distance = sr04_get_distance();//�����ʼ��λ�Ǻ���
		
		
		if(distance>=20 && distance<=500)
		{
			if(task_suspend==0)
			{			
				g_task_sta.task_rtc = 0;
				
				/* �߼���ȴ�������𣬱�����ŵ�OLED��������ʾ */
				SuspendValue = xEventGroupWaitBits(g_event_task_sta,
												   EVENT_TASK_RTC_SUSPEND,
												   pdTRUE,
												   pdTRUE,
												   portMAX_DELAY);
				
				if(SuspendValue & EVENT_TASK_RTC_SUSPEND)
				{
					dgb_printf_safe("[������ģ��] ̽�⵽������ǰ��ִ������ƥ�����,�뽫������׼����ͷ�������� 0.2m ~ 1m \r\n");
					
					OLED_SAFE(
					
					OLED_Clear();
					OLED_DrawBMP(48, 1, 80, 5, frm_icon_32x32);
					OLED_ShowCHinese(10, 6, 7);   //��
					OLED_ShowCHinese(28, 6, 8);   //��
					OLED_ShowCHinese(46, 6, 9);   //׼
					OLED_ShowCHinese(64, 6, 10);  //��
					OLED_ShowCHinese(82, 6, 11);  //��
					OLED_ShowCHinese(100, 6, 12); //ͷ		
					
					);
					
					/* ��������ʶ�� */
					xEventGroupSetBits(g_event_group,EVENT_GROUP_FRM_USER_COMPARE);	
					
					delay_ms(500);
					
					/* �ȴ�����ʶ����� */
					//�ȴ��¼����е���Ӧ�¼�λ����ͬ��
					EventValue=xEventGroupWaitBits(	(EventGroupHandle_t	)g_event_group,		
													(EventBits_t		)EVENT_GROUP_MOTOR_USER_END|EVENT_GROUP_FRM_USER_COMPARE_AGAIN,
													(BaseType_t			)pdTRUE,				
													(BaseType_t			)pdFALSE,
													(TickType_t			)portMAX_DELAY);

					if(EventValue & EVENT_GROUP_MOTOR_USER_END)
					{
						while(1)
						{
							distance = sr04_get_distance();	//�����ʼ��λ�Ǻ���

							delay_ms(500);
							
							if(distance>=20 && distance<=500)
							{
								dgb_printf_safe("[������ģ��] ����ʶ��ɹ��ˣ����뿪��ǰ...\r\n");
							}
							
							if(distance>400)
								break;
						}		
						
						delay_ms(1000);	
					}
			
					/* ����ʶ��ʧ�ܣ���Ҫ���´�������ʶ�� */
					if(EventValue & EVENT_GROUP_FRM_USER_COMPARE_AGAIN)
					{
						//����������������֤ʧ�ܣ���������֤
						//asr_send_str("8#");
						
						OLED_SAFE(
						
						OLED_Clear();
						OLED_ShowCHinese(10, 3, 13);   //��
						OLED_ShowCHinese(28, 3, 14);   //��
						OLED_ShowCHinese(46, 3, 15);   //��
						OLED_ShowCHinese(64, 3, 16);   //֤
						OLED_ShowCHinese(82, 3, 19);   //ʧ
						OLED_ShowCHinese(100, 3, 20);  //��		
						
						);
						
						delay_ms(1000);
						
						OLED_SAFE(
					
						OLED_Clear();
						OLED_ShowCHinese(18, 3, 24);   //��
						OLED_ShowCHinese(36, 3, 26);   //��
						OLED_ShowCHinese(54, 3, 27);   //��
						OLED_ShowCHinese(72, 3, 28);   //��
						OLED_ShowCHinese(90, 3, 29);   //֤
								
						);
						
						delay_ms(1000);
						
						dgb_printf_safe("[������ģ��] �ٴδ�������ʶ��...\r\n");
						
						delay_ms(1000);	
						
						/* ��������ʶ�� */
						//xEventGroupSetBits(g_event_group,EVENT_GROUP_FRM_USER_COMPARE);	
					}
					
				}
				
				task_suspend = 1;
				
			}//if(task_suspend==0)��β
			
		}//if(distance>=20 && distance<=400)��β
		
		if(distance>=500 && distance<=4000)
		{
			if(task_suspend)
			{
				/* �ָ�ָ�������� */				
				g_task_sta.task_rtc=1;				
				
				OLED_SAFE(
				
					OLED_Clear();
					OLED_ShowCHinese(2, 3, 0);  //��
					OLED_ShowCHinese(20, 3, 1); //��
					OLED_ShowCHinese(38, 3, 2); //��
					OLED_ShowCHinese(56, 3, 3); //��
					OLED_ShowCHinese(74, 3, 4); //��
					OLED_ShowCHinese(92, 3, 5); //��
					OLED_ShowCHinese(110, 3, 6);//��
				
				);	
					
				vTaskResume(app_task_rtc_handle);//�ָ�ʵʱʱ������
			
				task_suspend = 0;
			}
		}
		delay_ms(1000);
	}//for(;;)��β
	
}

void app_task_frm(void* pvParameters)
{
	uint8_t buf[64]={0};
	int32_t user_total;
	EventBits_t EventValue=0;
	
	dgb_printf_safe("3D����ʶ����������\r\n");
	
	/* ��3D����ʶ��ģ��������� */
	while(fr_init(115200)!=0)
	{
		delay_ms(1000);
	
		dgb_printf_safe("[����ʶ��ģ��] ������ ...\r\n");		
	}
	
	dgb_printf_safe("[����ʶ��ģ��] ��������\r\n");	

	for(;;)
	{	
		//�ȴ��¼����е���Ӧ�¼�λ����ͬ��
		EventValue=xEventGroupWaitBits((EventGroupHandle_t	)g_event_group,		
									   (EventBits_t			)0xF0,
									   (BaseType_t			)pdTRUE,				
									   (BaseType_t			)pdFALSE,
									   (TickType_t			)portMAX_DELAY);

		/* ������� */
		if(EventValue & EVENT_GROUP_FRM_USER_REG)
		{
			dgb_printf_safe("\r\n\r\n=====================================\r\n\r\n");
			dgb_printf_safe("[����ʶ��ģ��] ִ��������Ӳ���,�뽫������׼����ͷ�������� 0.2m ~ 1m \r\n");
			
			if(0 == fr_reg_user(NULL,0))
			{
				dgb_printf_safe("[����ʶ��ģ��] ע���û��ɹ�!\r\n");
				beep_on();
				delay_ms(100);
				beep_off();
			}
			else
			{
				dgb_printf_safe("[����ʶ��ģ��] ע���û�ʧ��!\r\n");
			}
			
			delay_ms(500);	
			
			/* �������ģʽ */
			fr_power_down();
			
			delay_ms(500);							
		}
		
		/* ˢ���� */
		if(EventValue & EVENT_GROUP_FRM_USER_COMPARE)
		{
			dgb_printf_safe("\r\n\r\n=====================================\r\n\r\n");
			dgb_printf_safe("[����ʶ��ģ��] ִ������ƥ�����,�뽫������׼����ͷ�������� 0.2m ~ 1m \r\n");
			
			memset(buf,0,sizeof buf);
			
			if(0 == fr_match(buf))
			{
				dgb_printf_safe("[����ʶ��ģ��] ����ƥ��ɹ�!\r\n");
				
				beep_on();
				delay_ms(100);
				beep_off();
				
				/* ����ʶ��ɹ���֪ͨ�ȴ����¼������� */
				xEventGroupSetBits(g_event_group,EVENT_GROUP_FRM_USER_COMPARE_END);	
			}
			else
			{
				dgb_printf_safe("[����ʶ��ģ��] ����ƥ��ʧ��!\r\n");
				
				/* ����ʶ��ʧ�ܺ�֪ͨ�ȴ����¼������� */
				xEventGroupSetBits(g_event_group,EVENT_GROUP_FRM_USER_COMPARE_AGAIN);					
			}	
			
			delay_ms(500);	
			
			/* �������ģʽ */
			fr_power_down();
			
			delay_ms(500);				
		}		
		
		/* ��ȡ�û����� */
		if(EventValue & EVENT_GROUP_FRM_USER_TOTAL)
		{
			dgb_printf_safe("\r\n\r\n=====================================\r\n\r\n");
	
			user_total = fr_get_user_total();
			
			if(user_total < 0 )
			{
				dgb_printf_safe("[����ʶ��ģ��] ��ȡ��ע����û�����ʧ��!\r\n");
			}
			else
			{
				dgb_printf_safe("[����ʶ��ģ��] ��ȡ��ע����û�����:%d\r\n",user_total);
				beep_on();
				delay_ms(100);
				beep_off();
			}
			
			delay_ms(500);	
			
			/* �������ģʽ */
			fr_power_down();
			
			delay_ms(500);			
		
		}	

		/* ɾ���������� */
		if(EventValue & EVENT_GROUP_FRM_USER_DEL_ALL)
		{
			dgb_printf_safe("\r\n\r\n=====================================\r\n\r\n");

			if(0 == fr_del_user_all())
			{
				dgb_printf_safe("[����ʶ��ģ��] ɾ�������û��ɹ�!\r\n");
				beep_on();
				delay_ms(100);
				beep_off();
			
			}
			else
			{
				dgb_printf_safe("[����ʶ��ģ��] ɾ�������û�ʧ��!\r\n");
			}
			
			delay_ms(500);	
			
			/* �������ģʽ */
			fr_power_down();
			
			delay_ms(500);				
		}			
	}
}

void app_task_motor(void* pvParameters)
{	
	uint32_t task_suspend = 0;
	
	EventBits_t EventValue=0;
	
	EventBits_t SuspendValue = 0;//�ж������Ƿ񶼹���ı�־λ
	
	while(1)
	{	
		/* �ȴ�����ʶ��ɹ�*/
		EventValue=xEventGroupWaitBits(		(EventGroupHandle_t	)g_event_group,		
											(EventBits_t		)EVENT_GROUP_BLUE_USER_COMPARE_END|EVENT_GROUP_FRM_USER_COMPARE_END,
											(BaseType_t			)pdTRUE,				
											(BaseType_t			)pdFALSE,
											(TickType_t			)portMAX_DELAY);
			
		if(EventValue & EVENT_GROUP_FRM_USER_COMPARE_END)
		{	
			
			OLED_SAFE(
		
			OLED_Clear();
			OLED_DrawBMP(48, 0, 80, 4, frm_success_32x32);
			OLED_ShowCHinese(10, 6, 13);   //��
			OLED_ShowCHinese(28, 6, 14);   //��
			OLED_ShowCHinese(46, 6, 15);   //��
			OLED_ShowCHinese(64, 6, 16);   //֤
			OLED_ShowCHinese(82, 6, 17);   //��
			OLED_ShowCHinese(100, 6, 18);  //��		
			
			);
			
			//����������������֤�ɹ������ѿ������
			//asr_send_str("7#");
			
			//����
			dgb_printf_safe("[�������ģ��] ����!\r\n");
			motor_corotation_double_rev_180();
			delay_ms(2000);
			
			
			OLED_SAFE(
		
			OLED_Clear();
			OLED_ShowCHinese(18, 3, 21);   //��
			OLED_ShowCHinese(36, 3, 22);   //��
			OLED_ShowCHinese(54, 3, 23);   //��
			OLED_ShowCHinese(72, 3, 24);   //��
			OLED_ShowCHinese(90, 3, 25);   //��	
		
			);
				
			delay_ms(5000);
			
			//���ź����
			dgb_printf_safe("[�������ģ��] ����!\r\n");
			
			motor_corotation_double_pos_180();
			delay_ms(2000);		
			
			//�����������ѹ���
			//asr_send_str("9#");
			
			/* �����ɹ���֪ͨ�ȴ����¼������� */
			xEventGroupSetBits(g_event_group,EVENT_GROUP_MOTOR_USER_END);	
		}
		
		if(EventValue & EVENT_GROUP_BLUE_USER_COMPARE_END)
		{	
			if(task_suspend==0)
			{
				g_task_sta.task_rtc = 0;
				
				/* �߼���ȴ�������𣬱�����ŵ�OLED��������ʾ */
				SuspendValue = xEventGroupWaitBits(g_event_task_sta,
													   EVENT_TASK_RTC_SUSPEND,
													   pdTRUE,
													   pdTRUE,
													   portMAX_DELAY);
				
				if(SuspendValue & EVENT_TASK_RTC_SUSPEND)
				{
					OLED_SAFE(
			
					OLED_Clear();
					OLED_DrawBMP(48, 0, 80, 4, blue_unlocked_success_32x32);
					OLED_ShowCHinese(10, 6, 30);   //��
					OLED_ShowCHinese(28, 6, 31);   //��
					OLED_ShowCHinese(46, 6, 32);   //��
					OLED_ShowCHinese(64, 6, 33);   //��
					OLED_ShowCHinese(82, 6, 17);   //��
					OLED_ShowCHinese(100, 6, 18);  //��		
							
					);
					
					//����
					dgb_printf_safe("[�������ģ��] ����!\r\n");
					motor_corotation_double_rev_180();
					delay_ms(2000);
					
					
					OLED_SAFE(
				
					OLED_Clear();
					OLED_ShowCHinese(18, 3, 21);   //��
					OLED_ShowCHinese(36, 3, 22);   //��
					OLED_ShowCHinese(54, 3, 23);   //��
					OLED_ShowCHinese(72, 3, 24);   //��
					OLED_ShowCHinese(90, 3, 25);   //��	
				
					);
						
					delay_ms(5000);
					
					//���ź����
					dgb_printf_safe("[�������ģ��] ����!\r\n");
					motor_corotation_double_pos_180();
					delay_ms(2000);	
				}
				
				task_suspend = 1;
				
			}//if(task_suspend==0)��β	
			
			if(task_suspend)
			{
				/* �ָ�ָ�������� */				
				g_task_sta.task_rtc=1;				
				
				OLED_SAFE(
				
					OLED_Clear();
					OLED_ShowCHinese(2, 3, 0);  //��
					OLED_ShowCHinese(20, 3, 1); //��
					OLED_ShowCHinese(38, 3, 2); //��
					OLED_ShowCHinese(56, 3, 3); //��
					OLED_ShowCHinese(74, 3, 4); //��
					OLED_ShowCHinese(92, 3, 5); //��
					OLED_ShowCHinese(110, 3, 6);//��
				
				);	
					
				vTaskResume(app_task_rtc_handle);//�ָ�ʵʱʱ������
			}
		}
	}//while(1)��β
}

void app_task_usart(void* pvParameters)
{
	uint8_t buf[32]={0};
	
	uint8_t dht11_data[5]={0};
	
	uint32_t value = 0;
	
	char *p=NULL;
	
	while(1)
	{
		xQueueReceive(g_queue_usart,buf,portMAX_DELAY); 
		
		dgb_printf_safe((char *)buf);
		
		if(strstr((const char *)buf,"TS")) //��TS-10-20-30#��
		{
			p=strtok((char *)buf,"-");//TS
			p=strtok(NULL,"-");//10
	
			RTC_TimeStructure.RTC_Hours   = atoi(p);
			
			if(RTC_TimeStructure.RTC_Hours>=12)
				RTC_TimeStructure.RTC_H12 = RTC_H12_PM;
			else
				RTC_TimeStructure.RTC_H12 = RTC_H12_AM;

			p=strtok(NULL,"-");//20		
			RTC_TimeStructure.RTC_Minutes = atoi(p);
			

			p=strtok(NULL,"-");//30#
			RTC_TimeStructure.RTC_Seconds = atoi(p);
			
			RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure); 
		}
		
		if(strstr((const char *)buf,"OPENDOOR")) //��OPENDOOR#��
		{
			/* ��������������֪ͨ�ȴ����¼������� */
			xEventGroupSetBits(g_event_group,EVENT_GROUP_BLUE_USER_COMPARE_END);
		}
		
		if(strstr((const char *)buf,"TIME"))
		{
			RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);	

			value=(RTC_TimeStructure.RTC_Hours<<16) \
				|(RTC_TimeStructure.RTC_Minutes<<8) \
				|(RTC_TimeStructure.RTC_Seconds);
			
			sprintf((char *)buf,"%d#",value);
			
			asr_send_str((char *)buf);
			
			printf("%02d:%02d:%02d\r\n", RTC_TimeStructure.RTC_Hours,
					RTC_TimeStructure.RTC_Minutes,
					RTC_TimeStructure.RTC_Seconds);				
		}
		
		if(strstr((const char *)buf,"DATE"))
		{
			RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);	
			/*  bit[0 - 3]: RTC_WeekDay 1~7
				bit[4 - 8]: RTC_Date 1~31
				bit[9 - 12]: RTC_Month 1~12
				bit[13 - 23]: RTC_Year 0~99
				bit[24 - 31]: reserve
			*/
			value=(RTC_DateStructure.RTC_Year<<13) \
				|(RTC_DateStructure.RTC_Month<<9) \
				|(RTC_DateStructure.RTC_Date<<4)\
				|(RTC_DateStructure.RTC_WeekDay);
			
			sprintf((char *)buf,"%d#",value);
			
			asr_send_str((char *)buf);
		}
		
		if(strstr((const char *)buf,"LED ON"))
		{
			PFout(9)=0;
			asr_send_str("1#");
		}
		
		if(strstr((const char *)buf,"LED OFF"))
		{	
			PFout(9)=1;				
			asr_send_str("1#");
		}

		if(strstr((const char *)buf,"TEMP"))
		{
			dht11_read_data(dht11_data);
			sprintf((char *)buf,"%d#",dht11_data[2]);
			asr_send_str((char *)buf);
			printf("%s\r\n",buf);
		}			
		
		if(strstr((const char *)buf,"HUMI"))
		{
			dht11_read_data(dht11_data);
			sprintf((char *)buf,"%d#",dht11_data[0]);
			
			asr_send_str((char *)buf);	
			printf("%s\r\n",buf);
		}	
	}
	
}

/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}


void vApplicationTickHook( void )
{

}
