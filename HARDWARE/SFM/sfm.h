#ifndef __SFM_H__
#define __SFM_H__

#include "includes.h"
/*
���ر�˵����
	���ϵ�����ָ��ģ���3V3���磬�ô�����Ӧ������Ч����ϸ�ٷ�˵�����£�
	Ϊ�˴ﵽ���ŵĵ͹�����ƣ�ʵ��Ӧ��ʱ���齫ģ��� VCC_3V3 ���ֶϵ�״̬��ͨ��
	�ж�TOUCH_OUT �ܽ��źſ��Ƹ�·��ѹ�������߹رա�����TOUCH_OUT ������Ч��ƽ
	ʱ��ʹ�� VCC_3V3 ��Դ����ʱָ��ģ����뵽����״̬��
��Ӳ��˵����	
   �����Ҫ���й�����һ�������ܵĿ��Ƶ�·
*/
#define SFM_TOUCH_ENABLE		0

#define SFM_ACK_SUCCESS 		0x00 //ִ�гɹ�
#define SFM_ACK_FAIL 			0x01 //ִ��ʧ��
#define SFM_ACK_FULL 			0x04 //���ݿ���
#define SFM_ACK_NOUSER 			0x05 //û������û�
#define SFM_ACK_USER_EXIST 		0x07 //�û��Ѵ���
#define SFM_ACK_TIMEOUT 		0x08 //ͼ��ɼ���ʱ
#define SFM_ACK_HARDWAREERROR 	0x0A //Ӳ������
#define SFM_ACK_IMAGEERROR 		0x10 //ͼ�����
#define SFM_ACK_BREAK 			0x18 //��ֹ��ǰָ��
#define SFM_ACK_ALGORITHMFAIL 	0x11 //��Ĥ�������
#define SFM_ACK_HOMOLOGYFAIL 	0x12 //ͬԴ��У�����

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

