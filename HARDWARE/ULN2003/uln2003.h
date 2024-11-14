#ifndef _ULN2003_H
#define _ULN2003_H

#include "includes.h"

#define MOTOR_IN1		PDout(15)
#define MOTOR_IN2		PDout(1)
#define MOTOR_IN3		PEout(8)
#define MOTOR_IN4		PEout(10)

extern void motor_init(void);//�����ʼ��
extern void motor_corotation_double_pos_180(void);//˫����������ʽ��ת��180�� AD-DC-CB-BA
extern void motor_corotation_double_rev_180(void);//˫����������ʽ��ת��180�� AB-BC-CD-DA

extern void motor_corotation_single_pos(void);//������������ʽ��ת�� D-C-B-A 360��
extern void motor_corotation_single_rev(void);//������������ʽ��ת�� A-B-C-D 360��
extern void motor_corotation_double_pos(void);//˫����������ʽ��ת��360�� AD-DC-CB-BA 
extern void motor_corotation_double_rev(void);//˫����������ʽ��ת��360�� AB-BC-CD-DA
extern void motor_corotation_eghit_pos(void);//����������ʽ��ת��360�� AD-D-DC-C-CB-B-BA-A
extern void motor_corotation_eghit_rev(void);//����������ʽ��ת�� A-AB-B-BC-C-CD-D-DA

#endif
