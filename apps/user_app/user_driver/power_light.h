#ifndef __POWER_LIGHT_H__
#define __POWER_LIGHT_H__

#include "includes.h"

#define POWER_LIGHT_PIN IO_PORTA_12

void power_light_init(void);
void power_light_on(void);
void power_light_off(void);
void power_light_toggle(void);



#endif

