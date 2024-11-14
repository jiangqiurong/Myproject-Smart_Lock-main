#ifndef __FM225_H__
#define __FM225_H__

#define FR_DEBUG_EN		0
#include "includes.h"


extern int32_t fr_del_user_all(void);
extern int32_t fr_get_user_total(void);
extern int32_t fr_match(uint8_t *buf);
extern int32_t fr_power_down(void);
extern int32_t fr_reg_user(const char *name,uint8_t is_admin);
extern int32_t fr_state_get(void);
extern int32_t fr_reset(void);
extern int32_t fr_init(uint32_t baud);

#endif

