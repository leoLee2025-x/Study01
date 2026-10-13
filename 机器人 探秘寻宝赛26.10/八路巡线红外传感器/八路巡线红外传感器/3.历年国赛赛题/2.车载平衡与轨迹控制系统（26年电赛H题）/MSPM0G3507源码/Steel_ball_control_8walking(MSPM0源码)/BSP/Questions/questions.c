/* questions.c - 竞赛任务状态机调度 (GBK) */
#include "questions.h"
#include "math.h"
#include "app_k230.h"
#include "app_servo.h"
#include "app_kalman.h"
#include "app_pid.h"
#include "delay.h"
#include "timer.h"
#include <stdio.h>

volatile struct state_machine State_Machine;
extern volatile int bee_time;
volatile int odometry_sum = 0;
int encoder_odometry_flag = 0;
float Servo_error = 0;
volatile int PID_Value = 0;
volatile uint32_t g_task_start_ms = 0;
volatile uint32_t g_task_end_ms   = 0;

#define LINE_SPEED      400
#define TURN_SPEED      250
#define LAP_ODOM_MIN    500
#define TASK1_SPEED      316
#define TASK3_SPEED      260
#define TASK4_SPEED      232           
#define TASK5_SPEED      232
#define RAMP_TIME_MS     3000
#define RAMP_STOP_TIME_MS 1500
#define TASK1_MAX_MS   20000
#define TASK2_MAX_MS    5000
#define TASK3_MAX_MS    8000
#define TASK4_MAX_MS   30000
#define TASK5_MAX_MS   30000
#define BALL_POS_5CM     5.0f
#define BALL_POS_N5CM   -5.0f
#define BALL_STABLE_CNT  60
#define BALL_TARGET_CENTER  0.0f

void State_Machine_init(void)
{
    State_Machine.Main_State = STOP_STATE;
    State_Machine.Sub_State = SUB_IDLE;
    State_Machine.Lap_Count = 0;
    State_Machine.Target_Position = 0;
}

static int CheckWaypoint(void)
{
    return (LineCheckStop() == 1);
}

void Question_Task_Run(void)
{
    switch (State_Machine.Main_State) {

    case TASK_LINE_LAP:
        switch (State_Machine.Sub_State) {
        case SUB_IDLE:
            State_Machine.Sub_State = SUB_INIT;
            break;
        case SUB_INIT:
            encoder_odometry_flag = 1;
            odometry_sum = 0;
            State_Machine.Lap_Count = 0;
            State_Machine.Sub_State = SUB_RUNNING;
            g_line_speed = TASK1_SPEED;
            g_task_start_ms = Get_Time();
            g_task_end_ms = 0;
            printf("Task1: Line lap\r\n");
            break;
        case SUB_RUNNING:
            LineWalking();
            if (Get_Time() - g_task_start_ms >= TASK1_MAX_MS) {
                Contrl_Pwm(0,0,0,0);
                State_Machine.Sub_State = SUB_DONE;
                g_task_end_ms = Get_Time();
                printf("Task1: timeout\r\n");
            }
            if (CheckWaypoint() && odometry_sum > LAP_ODOM_MIN) {
                State_Machine.Lap_Count++;
                odometry_sum = 0;
                bee_time = 100;
            }
            if (State_Machine.Lap_Count >= LAP_POINTS) {
                State_Machine.Sub_State = SUB_DONE;
                g_task_end_ms = Get_Time();
            }
            break;
        case SUB_DONE:
            Contrl_Pwm(0,0,0,0);
            encoder_odometry_flag = 0;
            State_Machine.Main_State = STOP_STATE;
            printf("Task1: Done\r\n");
            break;
        }
        break;

    case TASK_BALL_CTRL:
        switch (State_Machine.Sub_State) {
        case SUB_IDLE:
            State_Machine.Sub_State = SUB_INIT;
            break;
        case SUB_INIT:
            State_Machine.Sub_State = SUB_RUNNING;
            g_task_start_ms = Get_Time();
            g_task_end_ms = 0;
            printf("Task2: Ball +5->0->-5\r\n");
            break;
        case SUB_RUNNING: {
            static int phase = 0;
            static uint32_t t0 = 0;
            uint32_t now = Get_Time();
            if (t0 == 0) t0 = now;
            float target_cm;
            if (phase == 0) target_cm = +6.0f;
            else if (phase == 1) target_cm = 0.0f;
            else target_cm = -6.0f;
            K230_Receive_Process();
            K230_Servo_Control(target_cm);
            if (now - t0 >= 1800) {
                t0 = now;
                if (phase == 0) { phase = 1; printf("+5->0\r\n"); }
                else if (phase == 1) { phase = 2; printf("0->-5\r\n"); }
                else {
                    phase = 0; t0 = 0;
                    State_Machine.Sub_State = SUB_DONE;
                    g_task_end_ms = Get_Time();
                    printf("-5 DONE\r\n");
                }
            }
            break;
        }
        case SUB_DONE:
            K230_Receive_Process();
            K230_Servo_Control(-5.0f);
            break;
        }
        break;

    case TASK_AB_STAB:
        switch (State_Machine.Sub_State) {
        case SUB_IDLE:
            State_Machine.Sub_State = SUB_INIT;
            break;
        case SUB_INIT:
            encoder_odometry_flag = 1;
            odometry_sum = 0;
            State_Machine.Lap_Count = 0;
            State_Machine.Sub_State = SUB_RUNNING;
            g_line_speed = TASK3_SPEED;
            g_slow_stop = 0;
            g_task_start_ms = Get_Time();
            g_task_end_ms = 0;
            printf("Task3: A->B + Ball\r\n");
            break;
        case SUB_RUNNING:
            LineWalking_WithBall(BALL_TARGET_CENTER);
            if (Get_Time() - g_task_start_ms >= TASK3_MAX_MS - RAMP_STOP_TIME_MS) g_slow_stop = 1;
            if (Get_Time() - g_task_start_ms >= TASK3_MAX_MS) {
                Contrl_Pwm(0,0,0,0);
                State_Machine.Sub_State = SUB_DONE;
                g_task_end_ms = Get_Time();
                printf("Task3: timeout\r\n");
            }
            if (CheckWaypoint() && odometry_sum > LAP_ODOM_MIN) {
                State_Machine.Lap_Count++;
                odometry_sum = 0;
            }
            if (State_Machine.Lap_Count >= 1) {
                State_Machine.Sub_State = SUB_DONE;
                g_task_end_ms = Get_Time();
            }
            break;
        case SUB_DONE:
            Contrl_Pwm(0,0,0,0);
            K230_Receive_Process();
            K230_Servo_Control(BALL_TARGET_CENTER);
            encoder_odometry_flag = 0;
            break;
        }
        break;

    case TASK_LAP_STAB:
        switch (State_Machine.Sub_State) {
        case SUB_IDLE:
            State_Machine.Sub_State = SUB_INIT;
            break;
        case SUB_INIT:
            encoder_odometry_flag = 1;
            odometry_sum = 0;
            State_Machine.Lap_Count = 0;
            State_Machine.Sub_State = SUB_RUNNING;
            g_line_speed = TASK4_SPEED;
            g_slow_stop = 0;
            g_task_start_ms = Get_Time();
            g_task_end_ms = 0;
            printf("Task4: Lap + Ball\r\n");
            break;
        case SUB_RUNNING:
            LineWalking_WithBall(BALL_TARGET_CENTER);
            if (Get_Time() - g_task_start_ms >= TASK4_MAX_MS - RAMP_STOP_TIME_MS) g_slow_stop = 1;
            if (Get_Time() - g_task_start_ms >= TASK4_MAX_MS) {
                Contrl_Pwm(0,0,0,0);
                State_Machine.Sub_State = SUB_DONE;
                g_task_end_ms = Get_Time();
                printf("Task4: timeout\r\n");
            }
            if (CheckWaypoint() && odometry_sum > LAP_ODOM_MIN) {
                State_Machine.Lap_Count++;
                odometry_sum = 0;
                bee_time = 100;
            }
            if (State_Machine.Lap_Count >= LAP_POINTS) {
                State_Machine.Sub_State = SUB_DONE;
                g_task_end_ms = Get_Time();
            }
            break;
        case SUB_DONE:
            Contrl_Pwm(0,0,0,0);
            K230_Receive_Process();
            K230_Servo_Control(BALL_TARGET_CENTER);
            encoder_odometry_flag = 0;
            break;
        }
        break;

    case TASK_LAP_POS:
        switch (State_Machine.Sub_State) {
        case SUB_IDLE:
            State_Machine.Sub_State = SUB_INIT;
            break;
        case SUB_INIT:
            encoder_odometry_flag = 1;
            odometry_sum = 0;
            State_Machine.Lap_Count = 0;
            State_Machine.Sub_State = SUB_RUNNING;
            g_line_speed = TASK5_SPEED;
            g_slow_stop = 0;
            g_task_start_ms = Get_Time();
            g_task_end_ms = 0;
            printf("Task5: Lap + Pos(%d)\r\n", State_Machine.Target_Position);
            break;
        case SUB_RUNNING:
            LineWalking_WithBall((float)State_Machine.Target_Position);
            if (Get_Time() - g_task_start_ms >= TASK5_MAX_MS - RAMP_STOP_TIME_MS) g_slow_stop = 1;
            if (Get_Time() - g_task_start_ms >= TASK5_MAX_MS) {
                Contrl_Pwm(0,0,0,0);
                State_Machine.Sub_State = SUB_DONE;
                g_task_end_ms = Get_Time();
                printf("Task5: timeout\r\n");
            }
            if (CheckWaypoint() && odometry_sum > LAP_ODOM_MIN) {
                State_Machine.Lap_Count++;
                odometry_sum = 0;
                bee_time = 100;
            }
            if (State_Machine.Lap_Count >= LAP_POINTS) {
                State_Machine.Sub_State = SUB_DONE;
                g_task_end_ms = Get_Time();
            }
            break;
        case SUB_DONE:
            Contrl_Pwm(0,0,0,0);
            K230_Receive_Process();
            K230_Servo_Control((float)State_Machine.Target_Position);
            encoder_odometry_flag = 0;
            break;
        }
        break;

    default:
        break;
    }
}