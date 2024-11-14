#ifndef __RTC_H__
#define __RTC_H__

#include "includes.h"

#define RTC_CLK_SRC_IS_LSE  0
#define RTC_CLK_SRC_IS_LSI  1

#define RTC_CLK_SRC_SEL     RTC_CLK_SRC_IS_LSI

extern void rtc_init(void);
extern void rtc_alarm_init(void);
extern void rtc_alarm_set(RTC_AlarmTypeDef rtc_alarm_structure);
extern void rtc_alarm_set_in_second(uint32_t second);

#endif

