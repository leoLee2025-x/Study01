#ifndef	__BUZZER_H__
#define __BUZZER_H__

#include "ti_msp_dl_config.h"
#include "delay.h"
#include "led.h"



void Buzzer_Handle(void);
void PWM_Buzzer_Init(void);
void Buzzer_ON(void);
void Buzzer_OFF(void);
void Beep_Times(int times);
void Buzzer_Toggle(void);

#endif
