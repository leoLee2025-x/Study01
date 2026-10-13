/* questions.h - 钢珠跟踪竞赛任务状态机定义
 * ============================================================
 * 赛道: 矩形ABCD, 两边用圆弧替代, 顺时针行驶
 *   A --(直道)--> B
 *   |             |
 *  (圆弧)       (圆弧)
 *   |             |
 *   D <--(直道)-- C
 *
 * 任务:
 *   任务1: 图传装置安装(硬件, 不涉及代码)
 *   任务2: 纯循迹一圈(≤20s, 停车偏差≤2cm)
 *   任务3: 摆杆控制钢珠O→+5cm→-5cm(静止, ≤5s, 误差≤1cm)
 *   任务4: 循迹A→B + 钢珠稳定在中心(≤8s, 误差≤1cm)
 *   任务5: 循迹一圈 + 钢珠稳定在中心(≤30s, 误差≤1cm)
 *   任务6: 循迹一圈 + 钢珠稳定在任意指定位置(≤30s, 误差≤1cm)
 *
 * 按键K1长按: 循环切换任务1~6
 * 按键K1短按: 启动/停止当前任务
 */
#ifndef __QUESTIONS_H__
#define __QUESTIONS_H__

#include "ti_msp_dl_config.h"
#include "app_irtracking.h"
#include "app_motor.h"
#include "led.h"

/* ---- 主状态 ---- */
#define STOP_STATE      0   /* 停止 */
#define TASK_LINE_LAP   1   /* 任务2: 纯循迹一圈 */
#define TASK_BALL_CTRL  2   /* 任务3: 钢珠往复控制 */
#define TASK_AB_STAB    3   /* 任务4: A→B + 球稳定 */
#define TASK_LAP_STAB   4   /* 任务5: 一圈 + 球稳定 */
#define TASK_LAP_POS    5   /* 任务6: 一圈 + 球指定位置 */

/* ---- 任务子状态 ---- */
#define SUB_IDLE        0   /* 空闲等待 */
#define SUB_INIT        1   /* 初始化 */
#define SUB_RUNNING     2   /* 运行中 */
#define SUB_DONE        3   /* 完成 */

/* ---- 赛道检测点计数 ---- */
#define LAP_POINTS      4   /* A/B/C/D共4个检测点 */

/* ---- 状态机结构体 ---- */
struct state_machine {
    int Main_State;          /* 主状态 */
    int Sub_State;           /* 子状态 */
    int Lap_Count;           /* 已通过检测点数(一圈=4) */
    int Target_Position;     /* 目标位置(任务6用, 单位mm, 相对中心) */
};
extern volatile struct state_machine State_Machine;

extern uint8_t  running    ;
extern uint32_t start_ms   ;
extern uint32_t elapsed_ms ;
extern uint8_t  last_key   ;
extern uint8_t  last_key4  ;


/* ---- 里程计变量 ---- */
extern volatile int odometry_sum;
extern int encoder_odometry_flag;
extern float Servo_error;
extern volatile int PID_Value;
extern volatile uint32_t g_task_start_ms;
extern volatile uint32_t g_task_end_ms;
extern volatile uint32_t g_stop_t0;

//void OLED_time();
void State_Machine_init(void);
void Question_Task_Run(void);  /* 任务调度入口(替代原Q1-Q4) */

#endif