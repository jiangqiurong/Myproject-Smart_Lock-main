#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "includes.h"
#include "sys.h" 

#define USART_PACKET_SIZE 256

struct usart_packet_t
{
	uint8_t rx_buf[USART_PACKET_SIZE];
	uint8_t rx_len;
};

extern volatile uint8_t  g_usart1_rx_buf[USART_PACKET_SIZE];
extern volatile uint32_t g_usart1_rx_cnt;
extern volatile uint32_t g_usart1_rx_end;

extern volatile uint8_t  g_usart2_rx_buf[USART_PACKET_SIZE];
extern volatile uint32_t g_usart2_rx_cnt;
extern volatile uint32_t g_usart2_rx_end;

//如果想串口中断接收，请不要注释以下宏定义
void usart1_init(u32 baud);

void usart2_init(uint32_t baud);

void usart3_init(uint32_t baud);

void usart_send_str(USART_TypeDef* USARTx,char *str);

void usart_send_bytes(USART_TypeDef* USARTx,uint8_t *buf,uint32_t len);

#endif




