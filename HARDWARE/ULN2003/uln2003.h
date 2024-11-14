#ifndef _ULN2003_H
#define _ULN2003_H

#include "includes.h"

#define MOTOR_IN1		PDout(15)
#define MOTOR_IN2		PDout(1)
#define MOTOR_IN3		PEout(8)
#define MOTOR_IN4		PEout(10)

extern void motor_init(void);//电机初始化
extern void motor_corotation_double_pos_180(void);//双四拍驱动方式正转表180度 AD-DC-CB-BA
extern void motor_corotation_double_rev_180(void);//双四拍驱动方式反转表180度 AB-BC-CD-DA

extern void motor_corotation_single_pos(void);//单四拍驱动方式正转表 D-C-B-A 360度
extern void motor_corotation_single_rev(void);//单四拍驱动方式反转表 A-B-C-D 360度
extern void motor_corotation_double_pos(void);//双四拍驱动方式正转表360度 AD-DC-CB-BA 
extern void motor_corotation_double_rev(void);//双四拍驱动方式反转表360度 AB-BC-CD-DA
extern void motor_corotation_eghit_pos(void);//八拍驱动方式正转表360度 AD-D-DC-C-CB-B-BA-A
extern void motor_corotation_eghit_rev(void);//八拍驱动方式反转表 A-AB-B-BC-C-CD-D-DA

#endif
