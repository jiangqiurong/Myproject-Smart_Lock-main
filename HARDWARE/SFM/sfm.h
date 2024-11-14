#ifndef __SFM_H__
#define __SFM_H__

#include "includes.h"
/*
【特别说明】
	当断掉电容指纹模块的3V3供电，该触摸感应才能生效，详细官方说明如下：
	为了达到最优的低功耗设计，实际应用时建议将模组的 VCC_3V3 保持断电状态，通过
	判断TOUCH_OUT 管脚信号控制该路电压开启或者关闭。即当TOUCH_OUT 呈现有效电平
	时，使能 VCC_3V3 电源，此时指纹模组进入到工作状态。
【硬件说明】	
   因此需要自行构建的一个三极管的控制电路
*/
#define SFM_TOUCH_ENABLE		0

#define SFM_ACK_SUCCESS 		0x00 //执行成功
#define SFM_ACK_FAIL 			0x01 //执行失败
#define SFM_ACK_FULL 			0x04 //数据库满
#define SFM_ACK_NOUSER 			0x05 //没有这个用户
#define SFM_ACK_USER_EXIST 		0x07 //用户已存在
#define SFM_ACK_TIMEOUT 		0x08 //图像采集超时
#define SFM_ACK_HARDWAREERROR 	0x0A //硬件错误
#define SFM_ACK_IMAGEERROR 		0x10 //图像错误
#define SFM_ACK_BREAK 			0x18 //终止当前指令
#define SFM_ACK_ALGORITHMFAIL 	0x11 //贴膜攻击检测
#define SFM_ACK_HOMOLOGYFAIL 	0x12 //同源性校验错误

extern int32_t sfm_init(uint32_t baud);

extern uint8_t bcc_check(uint8_t *buf,uint32_t len);
extern int32_t sfm_init(uint32_t baud);
extern int32_t sfm_ctrl_led(uint8_t led_start,uint8_t led_end,uint8_t period);
extern int32_t sfm_reg_user(uint16_t id);
extern int32_t sfm_compare_users(uint16_t *id);
extern int32_t sfm_get_user_total(uint16_t *user_total);
extern const char *sfm_error_code(uint8_t error_code);
extern void sfm_touch_init(void);
extern int32_t sfm_touch_check(void);
extern int32_t sfm_del_user_all(void);
extern int32_t sfm_get_unused_id(uint16_t *id);

#if SFM_TOUCH_ENABLE
extern uint32_t sfm_touch_sta(void);
extern void sfm_power_ctrl(uint32_t on);
#endif

#endif

