#ifndef _DELAY_H
#define _DELAY_H

#include <stdint.h>
#include "ti_msp_dl_config.h"



extern volatile uint32_t g_ms_counter;
	
void delay_us(unsigned long __us);
void delay_ms(unsigned long ms);

#endif