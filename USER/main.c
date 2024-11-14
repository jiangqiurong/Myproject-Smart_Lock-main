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


/* 任务:初始化 */ 
void app_task_init(void* pvParameters);  

/* 任务:实时时间显示 */ 
void app_task_rtc(void* pvParameters); 

/* 任务:按键 */  
void app_task_key(void* pvParameters); 

/* 任务:人脸识别 */  
void app_task_frm(void* pvParameters); 

/* 任务:超声波测距 */  
void app_task_sr04(void* pvParameters); 

/* 任务:电机驱动门锁开关 */  
void app_task_motor(void* pvParameters); 

/* 任务:app_task_usart */ 
void app_task_usart(void* pvParameters);  

/* 互斥型信号量句柄 */
SemaphoreHandle_t g_mutex_printf;

SemaphoreHandle_t g_mutex_oled;

SemaphoreHandle_t  g_sem_rtc;

/* 事件标志组句柄 */
EventGroupHandle_t g_event_group;	

EventGroupHandle_t g_event_task_sta;

/* 消息队列句柄 */
QueueHandle_t g_queue_frm;

QueueHandle_t g_queue_usart;

/* printf互斥锁高度封装 */
void dgb_printf_safe(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	
	/* 获取互斥信号量 */
	xSemaphoreTake(g_mutex_printf,portMAX_DELAY);
	
	vprintf(format, args);
			
	/* 释放互斥信号量 */
	xSemaphoreGive(g_mutex_printf);	

	va_end(args);
}

//创建一个任务状态结构体用于标识任务的状态
struct task_status g_task_sta;


int main(void)
{
	/* 设置系统中断优先级分组4 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	/* 系统定时器中断频率为configTICK_RATE_HZ */
	SysTick_Config(SystemCoreClock/configTICK_RATE_HZ);										
	
	/* 初始化串口1 */
	usart1_init(9600);     
	
	/* 初始化串口3 */
	usart3_init(9600); 

	
	/* 创建app_task_init任务 */
	xTaskCreate((TaskFunction_t )app_task_init,  		/* 任务入口函数 */
			  (const char*    )"app_task_init",			/* 任务名字 */
			  (uint16_t       )512,  				/* 任务栈大小 */
			  (void*          )NULL,				/* 任务入口函数参数 */
			  (UBaseType_t    )5, 					/* 任务的优先级 */
			  (TaskHandle_t*  )&app_task_init_handle);	/* 任务控制块指针 */ 
	
	
	/* 开启任务调度 */
	vTaskStartScheduler(); 
			  
	while(1);

}

void app_task_init(void* pvParameters)
{
	/* 初始化语音模块 */
	asr_init(9600);
	
	/* 蜂鸣器初始化 */	
	beep_init();
	
	/* 温湿度初始化 */
	dht11_init();
	
	/* rtc初始化 */
	rtc_init();
	
	/* 按键初始化 */
	key_init();
	
	/* LED初始化 */
	led_init();
	
	/* oled初始化 */
	OLED_Init();
	
	OLED_Clear();
	
	OLED_ShowCHinese(2, 3, 0);  //安
	OLED_ShowCHinese(20, 3, 1); //居
	OLED_ShowCHinese(38, 3, 2); //乐
	OLED_ShowCHinese(56, 3, 3); //智
	OLED_ShowCHinese(74, 3, 4); //能
	OLED_ShowCHinese(92, 3, 5); //门
	OLED_ShowCHinese(110, 3, 6);//锁
	
	/* SR04初始化 */
	sr04_init();	
	
	/* 电机初始化 */
	motor_init();
	
	
	/* 创建互斥锁 */	  
	g_mutex_printf = xSemaphoreCreateMutex();
	
	g_mutex_oled = xSemaphoreCreateMutex();	
	
	//创建二值信号量
	g_sem_rtc = xSemaphoreCreateBinary();
	
	/* 创建事件标志组 */
	g_event_group = xEventGroupCreate();
	
	g_event_task_sta = xEventGroupCreate();

	/* 创建消息队列 */
	g_queue_frm = xQueueCreate(16, sizeof(g_usart2_rx_buf));	
	
	//创建消息队列，队列长度为5，元素大小为32字节
	g_queue_usart = xQueueCreate(5,32);
	
	/* 进入临界区，实际上会停止内核调度还有关闭中断 */
	taskENTER_CRITICAL();
	
	//任务状态初始化
	g_task_sta.task_rtc = 1;
	g_task_sta.task_key = 1;
	g_task_sta.task_sr04 = 1;
	g_task_sta.task_frm = 1;
	g_task_sta.task_motor = 1;
	g_task_sta.task_usart = 1;
	
	/* 创建app_task_rtc任务 */		  
	xTaskCreate((TaskFunction_t )app_task_rtc,  		/* 任务入口函数 */
			  (const char*    )"app_task_rtc",			/* 任务名字 */
			  (uint16_t       )512,  					/* 任务栈大小 */
			  (void*          )NULL,					/* 任务入口函数参数 */
			  (UBaseType_t    )5, 						/* 任务的优先级 */
			  (TaskHandle_t*  )&app_task_rtc_handle);	/* 任务控制块指针 */	
			  
	/* 创建app_task_key任务 */		  
	xTaskCreate((TaskFunction_t )app_task_key,  		/* 任务入口函数 */
			  (const char*    )"app_task_key",			/* 任务名字 */
			  (uint16_t       )512,  					/* 任务栈大小 */
			  (void*          )NULL,					/* 任务入口函数参数 */
			  (UBaseType_t    )6, 						/* 任务的优先级 */
			  (TaskHandle_t*  )&app_task_key_handle);	/* 任务控制块指针 */  
			  
	/* 创建app_task_frm任务 */		  
	xTaskCreate((TaskFunction_t )app_task_frm,  		/* 任务入口函数 */
			  (const char*    )"app_task_frm",			/* 任务名字 */
			  (uint16_t       )1024,  					/* 任务栈大小 */
			  (void*          )NULL,					/* 任务入口函数参数 */
			  (UBaseType_t    )7, 						/* 任务的优先级 */
			  (TaskHandle_t*  )&app_task_frm_handle);	/* 任务控制块指针 */				  
	
	/* 创建app_task_sr04任务 */		  
	xTaskCreate((TaskFunction_t )app_task_sr04,  		/* 任务入口函数 */
			  (const char*    )"app_task_sr04",			/* 任务名字 */
			  (uint16_t       )512,  					/* 任务栈大小 */
			  (void*          )NULL,					/* 任务入口函数参数 */
			  (UBaseType_t    )7, 						/* 任务的优先级 */
			  (TaskHandle_t*  )&app_task_sr04_handle);	/* 任务控制块指针 */		
			  
	/* 创建app_task_motor任务 */		  
	xTaskCreate((TaskFunction_t )app_task_motor,  		/* 任务入口函数 */
			  (const char*    )"app_task_motor",			/* 任务名字 */
			  (uint16_t       )512,  					/* 任务栈大小 */
			  (void*          )NULL,					/* 任务入口函数参数 */
			  (UBaseType_t    )7, 						/* 任务的优先级 */
			  (TaskHandle_t*  )&app_task_motor_handle);	/* 任务控制块指针 */	
			  
	/* 创建app_task_usart任务 */		  
	xTaskCreate((TaskFunction_t )app_task_usart,  		/* 任务入口函数 */
			  (const char*    )"app_task_usart",			/* 任务名字 */
			  (uint16_t       )512,  					/* 任务栈大小 */
			  (void*          )NULL,					/* 任务入口函数参数 */
			  (UBaseType_t    )7, 						/* 任务的优先级 */
			  (TaskHandle_t*  )&app_task_usart_handle);	/* 任务控制块指针 */	
	
			  
	/* 退出临界区，实际上恢复内核调度还有打开中断 */
	taskEXIT_CRITICAL();
			  
	//当初始化所有完毕，当前任务要删除，释放占用的内存空间
	vTaskDelete(NULL);		  
}   

void app_task_rtc(void* pvParameters)
{	
	uint8_t buf[16]={0};

	while(1)
	{
		//如果g_task_sta.task_rtc为0，则挂起任务的本身
		if(g_task_sta.task_rtc == 0)
		{
			//设置事件，告诉等待该事件的函数，RTC任务准备挂起
			xEventGroupSetBits(g_event_task_sta, EVENT_TASK_RTC_SUSPEND);
			
			vTaskSuspend(NULL);
		}
		
		
		//等待二值信号量，唤醒rtc任务
		xSemaphoreTake(g_sem_rtc,portMAX_DELAY);
		
		RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
		
		memset(buf, 0, sizeof buf);

				/* 格式化字符串 */
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
		//等待事件组中的相应事件位，或同步
		EventValue=xEventGroupWaitBits((EventGroupHandle_t	)g_event_group,		
									   (EventBits_t			)0x0F,
									   (BaseType_t			)pdTRUE,				
									   (BaseType_t			)pdFALSE,
									   (TickType_t			)portMAX_DELAY);
		//延时消抖
		vTaskDelay(50);		
		
		if(EventValue & EVENT_GROUP_KEY1_DOWN)
		{
			//禁止EXTI0触发中断
			NVIC_DisableIRQ(EXTI0_IRQn);
			
			//确认是按下
			if(PAin(0) == 0)
			{
				dgb_printf_safe("S1 Press\r\n");
				
				//等待按键释放
				while(PAin(0)==0)
					vTaskDelay(20);	

				xEventGroupSetBits(g_event_group,EVENT_GROUP_FRM_USER_REG);
			
			}
				
			//允许EXTI0触发中断
			NVIC_EnableIRQ(EXTI0_IRQn);					
		}
		
		if(EventValue & EVENT_GROUP_KEY2_DOWN)
		{
			//禁止EXTI2触发中断
			NVIC_DisableIRQ(EXTI2_IRQn);

				
			if(PEin(2) == 0)
			{
				dgb_printf_safe("S2 Press\r\n");
				
				//等待按键释放
				while(PEin(2)==0)
					vTaskDelay(20);
			
				xEventGroupSetBits(g_event_group,EVENT_GROUP_FRM_USER_COMPARE);				
			
			}

			//允许EXTI2触发中断
			NVIC_EnableIRQ(EXTI2_IRQn);	
		}	
		
		if(EventValue & EVENT_GROUP_KEY3_DOWN)
		{
			//禁止EXTI3触发中断
			NVIC_DisableIRQ(EXTI3_IRQn);
			
				
			if(PEin(3) == 0)	
			{
				dgb_printf_safe("S3 Press\r\n");
				
				//等待按键释放
				while(PEin(3)==0)
					vTaskDelay(20);

				xEventGroupSetBits(g_event_group,EVENT_GROUP_FRM_USER_TOTAL);
			}
				
			//允许EXTI3触发中断
			NVIC_EnableIRQ(EXTI3_IRQn);	
		}
		
		if(EventValue & EVENT_GROUP_KEY4_DOWN)
		{
			//禁止EXTI4触发中断
			NVIC_DisableIRQ(EXTI4_IRQn);
				
			if(PEin(4) == 0)	
			{
				dgb_printf_safe("S4 Press\r\n");
				
				
				//等待按键释放
				while(PEin(4)==0)
					vTaskDelay(20);
			
				xEventGroupSetBits(g_event_group,EVENT_GROUP_FRM_USER_DEL_ALL);				
			}
				
			//允许EXTI4触发中断
			NVIC_EnableIRQ(EXTI4_IRQn);	
		}
	}
} 

void app_task_sr04(void* pvParameters)
{
	int32_t distance=0;	
	
	uint32_t task_suspend = 0;
	
	EventBits_t EventValue=0;	
	
	EventBits_t SuspendValue = 0;//判断任务是否都挂起的标志位
	
	for(;;)
	{
		distance = sr04_get_distance();//这里初始单位是毫米
		
		
		if(distance>=20 && distance<=500)
		{
			if(task_suspend==0)
			{			
				g_task_sta.task_rtc = 0;
				
				/* 逻辑与等待任务挂起，避免干扰到OLED屏正常显示 */
				SuspendValue = xEventGroupWaitBits(g_event_task_sta,
												   EVENT_TASK_RTC_SUSPEND,
												   pdTRUE,
												   pdTRUE,
												   portMAX_DELAY);
				
				if(SuspendValue & EVENT_TASK_RTC_SUSPEND)
				{
					dgb_printf_safe("[超声波模块] 探测到人在门前，执行人脸匹配操作,请将脸部对准摄像头区，距离 0.2m ~ 1m \r\n");
					
					OLED_SAFE(
					
					OLED_Clear();
					OLED_DrawBMP(48, 1, 80, 5, frm_icon_32x32);
					OLED_ShowCHinese(10, 6, 7);   //请
					OLED_ShowCHinese(28, 6, 8);   //对
					OLED_ShowCHinese(46, 6, 9);   //准
					OLED_ShowCHinese(64, 6, 10);  //摄
					OLED_ShowCHinese(82, 6, 11);  //像
					OLED_ShowCHinese(100, 6, 12); //头		
					
					);
					
					/* 触发人脸识别 */
					xEventGroupSetBits(g_event_group,EVENT_GROUP_FRM_USER_COMPARE);	
					
					delay_ms(500);
					
					/* 等待人脸识别完毕 */
					//等待事件组中的相应事件位，或同步
					EventValue=xEventGroupWaitBits(	(EventGroupHandle_t	)g_event_group,		
													(EventBits_t		)EVENT_GROUP_MOTOR_USER_END|EVENT_GROUP_FRM_USER_COMPARE_AGAIN,
													(BaseType_t			)pdTRUE,				
													(BaseType_t			)pdFALSE,
													(TickType_t			)portMAX_DELAY);

					if(EventValue & EVENT_GROUP_MOTOR_USER_END)
					{
						while(1)
						{
							distance = sr04_get_distance();	//这里初始单位是毫米

							delay_ms(500);
							
							if(distance>=20 && distance<=500)
							{
								dgb_printf_safe("[超声波模块] 人脸识别成功了，请离开门前...\r\n");
							}
							
							if(distance>400)
								break;
						}		
						
						delay_ms(1000);	
					}
			
					/* 人脸识别失败，需要重新触发人脸识别 */
					if(EventValue & EVENT_GROUP_FRM_USER_COMPARE_AGAIN)
					{
						//语音播报：人脸验证失败，请重新验证
						//asr_send_str("8#");
						
						OLED_SAFE(
						
						OLED_Clear();
						OLED_ShowCHinese(10, 3, 13);   //人
						OLED_ShowCHinese(28, 3, 14);   //脸
						OLED_ShowCHinese(46, 3, 15);   //验
						OLED_ShowCHinese(64, 3, 16);   //证
						OLED_ShowCHinese(82, 3, 19);   //失
						OLED_ShowCHinese(100, 3, 20);  //败		
						
						);
						
						delay_ms(1000);
						
						OLED_SAFE(
					
						OLED_Clear();
						OLED_ShowCHinese(18, 3, 24);   //请
						OLED_ShowCHinese(36, 3, 26);   //重
						OLED_ShowCHinese(54, 3, 27);   //新
						OLED_ShowCHinese(72, 3, 28);   //验
						OLED_ShowCHinese(90, 3, 29);   //证
								
						);
						
						delay_ms(1000);
						
						dgb_printf_safe("[超声波模块] 再次触发人脸识别...\r\n");
						
						delay_ms(1000);	
						
						/* 触发人脸识别 */
						//xEventGroupSetBits(g_event_group,EVENT_GROUP_FRM_USER_COMPARE);	
					}
					
				}
				
				task_suspend = 1;
				
			}//if(task_suspend==0)结尾
			
		}//if(distance>=20 && distance<=400)结尾
		
		if(distance>=500 && distance<=4000)
		{
			if(task_suspend)
			{
				/* 恢复指定的任务 */				
				g_task_sta.task_rtc=1;				
				
				OLED_SAFE(
				
					OLED_Clear();
					OLED_ShowCHinese(2, 3, 0);  //安
					OLED_ShowCHinese(20, 3, 1); //居
					OLED_ShowCHinese(38, 3, 2); //乐
					OLED_ShowCHinese(56, 3, 3); //智
					OLED_ShowCHinese(74, 3, 4); //能
					OLED_ShowCHinese(92, 3, 5); //门
					OLED_ShowCHinese(110, 3, 6);//锁
				
				);	
					
				vTaskResume(app_task_rtc_handle);//恢复实时时间任务
			
				task_suspend = 0;
			}
		}
		delay_ms(1000);
	}//for(;;)结尾
	
}

void app_task_frm(void* pvParameters)
{
	uint8_t buf[64]={0};
	int32_t user_total;
	EventBits_t EventValue=0;
	
	dgb_printf_safe("3D人脸识别智能门锁\r\n");
	
	/* 跟3D人脸识别模块进行握手 */
	while(fr_init(115200)!=0)
	{
		delay_ms(1000);
	
		dgb_printf_safe("[人脸识别模块] 连接中 ...\r\n");		
	}
	
	dgb_printf_safe("[人脸识别模块] 已连接上\r\n");	

	for(;;)
	{	
		//等待事件组中的相应事件位，或同步
		EventValue=xEventGroupWaitBits((EventGroupHandle_t	)g_event_group,		
									   (EventBits_t			)0xF0,
									   (BaseType_t			)pdTRUE,				
									   (BaseType_t			)pdFALSE,
									   (TickType_t			)portMAX_DELAY);

		/* 添加人脸 */
		if(EventValue & EVENT_GROUP_FRM_USER_REG)
		{
			dgb_printf_safe("\r\n\r\n=====================================\r\n\r\n");
			dgb_printf_safe("[人脸识别模块] 执行人脸添加操作,请将脸部对准摄像头区，距离 0.2m ~ 1m \r\n");
			
			if(0 == fr_reg_user(NULL,0))
			{
				dgb_printf_safe("[人脸识别模块] 注册用户成功!\r\n");
				beep_on();
				delay_ms(100);
				beep_off();
			}
			else
			{
				dgb_printf_safe("[人脸识别模块] 注册用户失败!\r\n");
			}
			
			delay_ms(500);	
			
			/* 进入掉电模式 */
			fr_power_down();
			
			delay_ms(500);							
		}
		
		/* 刷人脸 */
		if(EventValue & EVENT_GROUP_FRM_USER_COMPARE)
		{
			dgb_printf_safe("\r\n\r\n=====================================\r\n\r\n");
			dgb_printf_safe("[人脸识别模块] 执行人脸匹配操作,请将脸部对准摄像头区，距离 0.2m ~ 1m \r\n");
			
			memset(buf,0,sizeof buf);
			
			if(0 == fr_match(buf))
			{
				dgb_printf_safe("[人脸识别模块] 人脸匹配成功!\r\n");
				
				beep_on();
				delay_ms(100);
				beep_off();
				
				/* 人脸识别成功后，通知等待该事件的任务 */
				xEventGroupSetBits(g_event_group,EVENT_GROUP_FRM_USER_COMPARE_END);	
			}
			else
			{
				dgb_printf_safe("[人脸识别模块] 人脸匹配失败!\r\n");
				
				/* 人脸识别失败后，通知等待该事件的任务 */
				xEventGroupSetBits(g_event_group,EVENT_GROUP_FRM_USER_COMPARE_AGAIN);					
			}	
			
			delay_ms(500);	
			
			/* 进入掉电模式 */
			fr_power_down();
			
			delay_ms(500);				
		}		
		
		/* 获取用户总数 */
		if(EventValue & EVENT_GROUP_FRM_USER_TOTAL)
		{
			dgb_printf_safe("\r\n\r\n=====================================\r\n\r\n");
	
			user_total = fr_get_user_total();
			
			if(user_total < 0 )
			{
				dgb_printf_safe("[人脸识别模块] 获取已注册的用户总数失败!\r\n");
			}
			else
			{
				dgb_printf_safe("[人脸识别模块] 获取已注册的用户总数:%d\r\n",user_total);
				beep_on();
				delay_ms(100);
				beep_off();
			}
			
			delay_ms(500);	
			
			/* 进入掉电模式 */
			fr_power_down();
			
			delay_ms(500);			
		
		}	

		/* 删除所有人脸 */
		if(EventValue & EVENT_GROUP_FRM_USER_DEL_ALL)
		{
			dgb_printf_safe("\r\n\r\n=====================================\r\n\r\n");

			if(0 == fr_del_user_all())
			{
				dgb_printf_safe("[人脸识别模块] 删除所有用户成功!\r\n");
				beep_on();
				delay_ms(100);
				beep_off();
			
			}
			else
			{
				dgb_printf_safe("[人脸识别模块] 删除所有用户失败!\r\n");
			}
			
			delay_ms(500);	
			
			/* 进入掉电模式 */
			fr_power_down();
			
			delay_ms(500);				
		}			
	}
}

void app_task_motor(void* pvParameters)
{	
	uint32_t task_suspend = 0;
	
	EventBits_t EventValue=0;
	
	EventBits_t SuspendValue = 0;//判断任务是否都挂起的标志位
	
	while(1)
	{	
		/* 等待人脸识别成功*/
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
			OLED_ShowCHinese(10, 6, 13);   //人
			OLED_ShowCHinese(28, 6, 14);   //脸
			OLED_ShowCHinese(46, 6, 15);   //验
			OLED_ShowCHinese(64, 6, 16);   //证
			OLED_ShowCHinese(82, 6, 17);   //成
			OLED_ShowCHinese(100, 6, 18);  //功		
			
			);
			
			//语音播报：人脸验证成功，门已开，请进
			//asr_send_str("7#");
			
			//开门
			dgb_printf_safe("[步进电机模块] 开门!\r\n");
			motor_corotation_double_rev_180();
			delay_ms(2000);
			
			
			OLED_SAFE(
		
			OLED_Clear();
			OLED_ShowCHinese(18, 3, 21);   //门
			OLED_ShowCHinese(36, 3, 22);   //已
			OLED_ShowCHinese(54, 3, 23);   //开
			OLED_ShowCHinese(72, 3, 24);   //请
			OLED_ShowCHinese(90, 3, 25);   //进	
		
			);
				
			delay_ms(5000);
			
			//开门后关门
			dgb_printf_safe("[步进电机模块] 关门!\r\n");
			
			motor_corotation_double_pos_180();
			delay_ms(2000);		
			
			//语音播报：已关门
			//asr_send_str("9#");
			
			/* 开锁成功后，通知等待该事件的任务 */
			xEventGroupSetBits(g_event_group,EVENT_GROUP_MOTOR_USER_END);	
		}
		
		if(EventValue & EVENT_GROUP_BLUE_USER_COMPARE_END)
		{	
			if(task_suspend==0)
			{
				g_task_sta.task_rtc = 0;
				
				/* 逻辑与等待任务挂起，避免干扰到OLED屏正常显示 */
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
					OLED_ShowCHinese(10, 6, 30);   //蓝
					OLED_ShowCHinese(28, 6, 31);   //牙
					OLED_ShowCHinese(46, 6, 32);   //解
					OLED_ShowCHinese(64, 6, 33);   //锁
					OLED_ShowCHinese(82, 6, 17);   //成
					OLED_ShowCHinese(100, 6, 18);  //功		
							
					);
					
					//开门
					dgb_printf_safe("[步进电机模块] 开门!\r\n");
					motor_corotation_double_rev_180();
					delay_ms(2000);
					
					
					OLED_SAFE(
				
					OLED_Clear();
					OLED_ShowCHinese(18, 3, 21);   //门
					OLED_ShowCHinese(36, 3, 22);   //已
					OLED_ShowCHinese(54, 3, 23);   //开
					OLED_ShowCHinese(72, 3, 24);   //请
					OLED_ShowCHinese(90, 3, 25);   //进	
				
					);
						
					delay_ms(5000);
					
					//开门后关门
					dgb_printf_safe("[步进电机模块] 关门!\r\n");
					motor_corotation_double_pos_180();
					delay_ms(2000);	
				}
				
				task_suspend = 1;
				
			}//if(task_suspend==0)结尾	
			
			if(task_suspend)
			{
				/* 恢复指定的任务 */				
				g_task_sta.task_rtc=1;				
				
				OLED_SAFE(
				
					OLED_Clear();
					OLED_ShowCHinese(2, 3, 0);  //安
					OLED_ShowCHinese(20, 3, 1); //居
					OLED_ShowCHinese(38, 3, 2); //乐
					OLED_ShowCHinese(56, 3, 3); //智
					OLED_ShowCHinese(74, 3, 4); //能
					OLED_ShowCHinese(92, 3, 5); //门
					OLED_ShowCHinese(110, 3, 6);//锁
				
				);	
					
				vTaskResume(app_task_rtc_handle);//恢复实时时间任务
			}
		}
	}//while(1)结尾
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
		
		if(strstr((const char *)buf,"TS")) //“TS-10-20-30#”
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
		
		if(strstr((const char *)buf,"OPENDOOR")) //“OPENDOOR#”
		{
			/* 请求蓝牙开锁后，通知等待该事件的任务 */
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
