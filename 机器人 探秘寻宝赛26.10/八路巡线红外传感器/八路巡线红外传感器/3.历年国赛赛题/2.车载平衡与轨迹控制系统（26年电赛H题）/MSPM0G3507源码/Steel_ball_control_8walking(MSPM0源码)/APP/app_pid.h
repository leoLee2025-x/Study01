/* app_pid.h - PID 控制器 (位置式 + 增量式) */
#ifndef __APP_PID_H
#define __APP_PID_H
#include <stdint.h>

typedef struct {
    float KP, KI, KD;         /* P=比例 I=积分 D=微分系数 */
    float Err, LastErr;       /* 当前误差 / 上次误差 */
    float LastErr2;           /* 上上次误差 (增量式用) */
    float Integral;           /* 积分累加值 (带限幅) */
    float Target;             /* 目标值 (设定值) */
    int16_t out;              /* 控制输出 */
    int16_t prev_out;         /* 上次输出 (增量式用) */
} PID_t;

void PID_Init(PID_t *p);
void PID_SetGains(PID_t *p, float kp, float ki, float kd);
int16_t PID_Calc(PID_t *p, float val);         /* 位置式 */
int16_t PID_Calc_Inc(PID_t *p, float val);     /* 增量式 */
float PID_To_CC_Pos(PID_t *p, float val);      /* 位置式PID -> CC值 */
float PID_To_CC_Inc(PID_t *p, float val);      /* 增量式PID -> CC值 */

#endif
