/* app_pid.c - PID 控制器 (位置式 + 增量式) */
#include "app_pid.h"
#include <string.h>

void PID_Init(PID_t *p)
{
    memset(p, 0, sizeof(*p));
}

void PID_SetGains(PID_t *p, float kp, float ki, float kd)
{
    p->KP = kp;
    p->KI = ki;
    p->KD = kd;
}

/* 位置式PID: u = Kp*Err + Ki*Sum(Err) + Kd*(Err-LastErr) */
int16_t PID_Calc(PID_t *p, float val)
{
    p->Err = p->Target - val;
    p->Integral += p->Err;
    if (p->Integral >  500.0f) p->Integral =  500.0f;
    if (p->Integral < -500.0f) p->Integral = -500.0f;
    float out = p->KP * p->Err
              + p->KI * p->Integral
              + p->KD * (p->Err - p->LastErr);
    p->LastErr = p->Err;
    if (out >  500.0f) out =  500.0f;
    if (out < -500.0f) out = -500.0f;
    p->out = (int16_t)out;
    return p->out;
}

/* 增量式PID: Du = Kp*(e-e1) + Ki*e + Kd*(e-2e1+e2) */
int16_t PID_Calc_Inc(PID_t *p, float val)
{
    p->Err = p->Target - val;
    float d_err  = p->Err - p->LastErr;
    float d2_err = p->Err - 2.0f*p->LastErr + p->LastErr2;
    float inc = p->KP*d_err + p->KI*p->Err + p->KD*d2_err;
    float out = (float)p->prev_out + inc;
    if (out >  500.0f) out =  500.0f;
    if (out < -500.0f) out = -500.0f;
    p->prev_out = (int16_t)out;
    p->LastErr2 = p->LastErr;
    p->LastErr  = p->Err;
    p->out = (int16_t)out;
    return p->out;
}

/* 位置式 -> CC (25us/tick, center=60, range=20~100) */
float PID_To_CC_Pos(PID_t *p, float val)
{
    int16_t out = PID_Calc(p, val);
    float cc = 60.0f + (float)out * 0.80f;
    if (cc < 20.0f) cc = 20.0f;
    if (cc > 100.0f) cc = 100.0f;
    return cc;
}

/* 增量式 -> CC */
float PID_To_CC_Inc(PID_t *p, float val)
{
    int16_t out = PID_Calc_Inc(p, val);
    float cc = 60.0f + (float)out * 0.80f;
    if (cc < 20.0f) cc = 20.0f;
    if (cc > 100.0f) cc = 100.0f;
    return cc;
}
