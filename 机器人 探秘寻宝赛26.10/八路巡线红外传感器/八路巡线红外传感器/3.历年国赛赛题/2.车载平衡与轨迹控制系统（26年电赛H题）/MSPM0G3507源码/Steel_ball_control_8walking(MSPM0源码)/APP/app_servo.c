#include "app_servo.h"
#include "ti_msp_dl_config.h"

/* 25us/tick: prescale=100 period=800
 * CC: 20=500us(0deg)  60=1500us(90deg)  100=2500us(180deg)
 * 保护: 死区±3tick(75us)防抖, 速率限±10tick(250us)防扫齿
 */
void Servo_Init(void)
{
    DL_TimerA_startCounter(PWM_Servo_INST);
    DL_TimerA_setCaptureCompareValue(PWM_Servo_INST, 60, DL_TIMER_CC_0_INDEX);
}

void Set_Servo_Angle(unsigned int angle)
{
    if (angle > 180) angle = 180;
    float cc = 20.0f + (((float)angle / 180.0f) * 80.0f);

    /* 保护: 死区+速率限制 */
    static float cc_last = 60.0f;
    float delta = cc - cc_last;
    if (delta > -3.0f && delta < 3.0f) return;       /* 变化太小, 不响应 */
    float max_step = 10.0f;
    if (delta >  max_step) cc = cc_last + max_step;
    if (delta < -max_step) cc = cc_last - max_step;
    cc_last = cc;

    if (cc < 20.0f) cc = 20.0f;
    if (cc > 100.0f) cc = 100.0f;
    DL_TimerA_setCaptureCompareValue(PWM_Servo_INST, (unsigned int)(cc + 0.5f), DL_TIMER_CC_0_INDEX);
    DL_TimerA_startCounter(PWM_Servo_INST);
}

void Servo_SetPos(int offset_us)
{
    int us = 1500 + offset_us;
    if (us < 500) us = 500;
    if (us > 2500) us = 2500;
    float cc = us / 25.0f;

    /* 保护: 死区+速率限制 */
    static float cc_last = 60.0f;
    float delta = cc - cc_last;
    if (delta > -3.0f && delta < 3.0f) return;
    float max_step = 10.0f;
    if (delta >  max_step) cc = cc_last + max_step;
    if (delta < -max_step) cc = cc_last - max_step;
    cc_last = cc;

    if (cc < 20.0f) cc = 20.0f;
    if (cc > 100.0f) cc = 100.0f;
    DL_TimerA_setCaptureCompareValue(PWM_Servo_INST, (unsigned int)(cc + 0.5f), DL_TIMER_CC_0_INDEX);
    DL_TimerA_startCounter(PWM_Servo_INST);
}