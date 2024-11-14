#ifndef __INCLUDES_H__
#define __INCLUDES_H__



/* �1�7�1�7�0�6C�1�7�1�7*/
#include <stdio.h>	
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "stack_macros.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"

/* �1�7�1�7�1�7�1�7�1�7�1�7�1�7 */
#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "oled.h"
#include "oledfont.h"
#include "bmp.h"
#include "dht11.h"
#include "key.h"
#include "beep.h"
#include "fm225.h"
#include "sr04.h"
#include "rtc.h"
#include "adc.h"
#include "asrpro.h"
#include "uln2003.h"

/*�1�7�1�7�0�3�1�7�1�7�1�7�5�5�1�7�1�7*/
extern GPIO_InitTypeDef GPIO_InitStructure;
extern EXTI_InitTypeDef	EXTI_InitStructure;
extern USART_InitTypeDef USART_InitStructure;
extern NVIC_InitTypeDef NVIC_InitStructure;
extern RTC_TimeTypeDef RTC_TimeStructure;
extern RTC_DateTypeDef  RTC_DateStructure;
extern RTC_InitTypeDef  RTC_InitStructure;
extern RTC_AlarmTypeDef RTC_AlarmStructure;
extern TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;


/* �1�7�1�7�1�7�1�7�0�8�0�0�1�7�5�5�1�7�؄1�7�1�7�0�7�1�7�1�7�1�7�1�7�1�7�0�8�0�0 */
struct task_status
{
	uint32_t task_rtc:1;
	uint32_t task_key:1;	
	uint32_t task_frm:1;
	uint32_t task_sr04:1;	
	uint32_t task_motor:1;	
	uint32_t task_usart:1;	
	//�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�0�0�1�7�1�7�1�7�1�7...
};

/* �1�7�7�9�1�7�1�7 */
#define EVENT_GROUP_KEY1_DOWN		0x01//bit0�1�7�1�71
#define EVENT_GROUP_KEY2_DOWN		0x02//bit1�1�7�1�71
#define EVENT_GROUP_KEY3_DOWN		0x04//bit2�1�7�1�71
#define EVENT_GROUP_KEY4_DOWN		0x08//bit3�1�7�1�71
#define EVENT_GROUP_KEYALL_DOWN		0x0F//bit0~3�1�7�1�71

#define EVENT_GROUP_FRM_USER_REG		0x10//bit4�1�7�1�71
#define EVENT_GROUP_FRM_USER_COMPARE	0x20//bit5�1�7�1�71
#define EVENT_GROUP_FRM_USER_TOTAL		0x40//bit6�1�7�1�71
#define EVENT_GROUP_FRM_USER_DEL_ALL	0x80//bit7�1�7�1�71


#define EVENT_GROUP_FRM_USER_COMPARE_END	0x100//bit8�1�7�1�71
#define EVENT_GROUP_FRM_USER_COMPARE_AGAIN	0x200//bit9�1�7�1�71
#define EVENT_GROUP_BLUE_USER_COMPARE_END 	0x400//bit10�1�7�1�71
#define EVENT_GROUP_MOTOR_USER_END 			0x800//bit11�1�7�1�71

#define EVENT_TASK_RTC_SUSPEND   0x01//bit0�1�7�1�71
#define EVENT_TASK_KEY_SUSPEND   0x02//bit1�1�7�1�71
#define EVENT_TASK_FRM_SUSPEND   0x04//bit2�1�7�1�71
#define EVENT_TASK_SR04_SUSPEND  0x08//bit3�1�7�1�71
#define EVENT_TASK_MOTOR_SUSPEND 0x10//bit4�1�7�1�71
#define EVENT_TASK_USART_SUSPEND 0x12//bit5�1�7�1�71

/* �1�7�1�7�1�7�1�7�1�7�1�7�1�7�0�2�1�7�1�7�1�7�1�7�1�7�1�7 */
extern SemaphoreHandle_t g_mutex_printf;
extern SemaphoreHandle_t g_mutex_oled;
extern SemaphoreHandle_t  g_sem_rtc;

/* �1�7�0�4�1�7�1�7�1�7�0�4�1�7�1�7�1�7�1�7 */
extern EventGroupHandle_t g_event_group;
extern EventGroupHandle_t g_event_task_sta;

/* �1�7�1�7�0�4�1�7�1�7�1�7���1�7�1�7 */
extern QueueHandle_t g_queue_frm;
extern QueueHandle_t g_queue_usart;



/* �1�7�1�7�1�7�1�7:�1�7�1�7�0�3�1�7�1�7 */ 
extern void app_task_init(void* pvParameters);  

/* �1�7�1�7�1�7�1�7:rtc�0�6�0�2�0�2�1�7�1�7�1�7�1�7�0�5 */ 
extern void app_task_rtc(void* pvParameters); 

/* �1�7�1�7�1�7�1�7:�1�7�1�7�1�7�1�7 */  
extern void app_task_key(void* pvParameters); 

/* �1�7�1�7�1�7�1�7:�1�7�1�7�1�7�1�7�0�7�1�7�1�7 */  
extern void app_task_frm(void* pvParameters); 

/* �1�7�1�7�1�7�1�7:�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7 */  
extern void app_task_sr04(void* pvParameters); 

/* �1�7�1�7�1�7�1�7:�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7 */  
extern void app_task_motor(void* pvParameters); 

/* �1�7�1�7�1�7�1�7:app_task_usart */ 
extern void app_task_usart(void* pvParameters);    


/* �1�7�1�7�1�7�1�7 */

/* OLED�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�2�0�2�1�7�0�4 */
#define OLED_SAFE(__CODE)                        \
do                                               \
{   /* �1�7�1�7�0�0�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7OLED�1�7�1�7*/                   \
	xSemaphoreTake(g_mutex_oled, portMAX_DELAY); \
												 \
	__CODE;  									 \
												 \
	/* �1�7�1�7�1�7�1�7�1�7�0�5�0�3�1�7�1�7�1�7�1�7�0�2�1�7�1�7�1�7 */						 \
	xSemaphoreGive(g_mutex_oled);			 	 \
} while (0)                           			 \


/* �1�7�1�7�1�7�1�7 */
extern void dgb_printf_safe(const char *format, ...);

#endif

