/* main.c - MSPM0G3507 竞赛小车主程序
 * ============================================================
 * 功能:
 *   1. 上电初始化所有外设 (串口/舵机/K230/电机/OLED/状态机/定时器)
 *   2. 等待 K230 摄像头启动 (4秒)
 *   3. 主循环: 调度器 + 状态机分发
 *
 * 状态机分发逻辑:
 *   STOP_STATE      -> 停车, 只接收 K230 数据
 *   TASK_LINE_LAP   -> 任务1: 纯循迹 (调用 Question_Task_Run)
 *   TASK_BALL_CTRL  -> 任务2: 钢珠控制
 *   TASK_AB_STAB    -> 任务3: A->B + 控球
 *   TASK_LAP_STAB   -> 任务4: 一圈 + 控球
 *   TASK_LAP_POS    -> 任务5: 指定位置 + 控球
 *
 * 调度器 (Scheduler_Run):
 *   - 每 100ms 调用 Key_Handle() 检测按键
 *   - 每 200ms 调用 OLED_time() 刷新显示
 *
 * GBK 编码 (Keil 可读)
 */
#include "ti_msp_dl_config.h"
#include "delay.h"
#include "usart.h"
#include "app_motor_usart.h"
#include "app_motor.h"
#include "bsp_mpu6050.h"
#include "app_mpu6050.h"
#include "inv_mpu.h"
#include "app_irtracking.h"
#include "key.h"
#include "led.h"
#include "task.h"
#include "questions.h"
#include "app_servo.h"
#include "app_k230.h"
#include "oled.h"

/* 电机类型: 5 = 特定电机型号的配置参数 */
#define MOTOR_TYPE 5

/* 蜂鸣器相关 (保留接口, 实际未使用) */
volatile int bee_time = 0;
void Beep_Times(int n) { (void)n; }
void Buzzer_Handle(void) {}

/* 全局标志 (保留) */
int g_LinePortal_flag = 0;
char buffer[80];


int main(void)
{
    /* ===== 第1步: 外设初始化 ===== */
    USART_Init();                   /* 串口初始化: UART0(debug), UART1(电机), UART3(K230) */
    Servo_Init();                   /* 舵机 PWM 初始化 (TIMA0, 50Hz, 中心 1500us) */
    K230_Init();                    /* K230 UART3 RX 中断初始化 */
    Set_Motor(MOTOR_TYPE);          /* 配置电机驱动板参数 (类型/减速比/轮径等) */
    send_upload_data(false, false, true);  /* 启用电机速度上报 */
    OLED_Init();                    /* OLED 128x32 初始化 */
    delay_ms(10);

    /* ===== 第2步: 等待 K230 启动 ===== */
    /* K230 需要约 4 秒启动 AI 模型, 期间无数据发送 */
    OLED_ShowString(0, 1, (uint8_t*)"K230 Init...", 16, 1);
    OLED_Refresh();
    delay_ms(4000);
    OLED_Clear();
    OLED_ShowString(0, 1, (uint8_t*)"Ready", 16, 1);
    OLED_Refresh();
    delay_ms(500);

    /* ===== 第3步: 状态机初始化 ===== */
    State_Machine_init();           /* Main_State=STOP, Sub_State=IDLE */

    /* ===== 第4步: 启动 1ms 系统定时器 ===== */
    /* 提供 Get_Time() 毫秒时钟, 用于任务计时和 OLED 显示 */
    NVIC_ClearPendingIRQ(TIMER_0_INST_INT_IRQN);
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
    DL_TimerA_startCounter(TIMER_0_INST);

    /* ===== 第5步: 主循环 ===== */
    while (1) {
        /* 调度器: 运行定时任务 (按键检测 100ms + OLED 刷新 200ms) */
        Scheduler_Run();

        /* 状态机分发: 根据 Main_State 执行对应任务 */
        switch (State_Machine.Main_State) {

        case STOP_STATE:
            /* 停止状态: 电机停车, 只接收 K230 数据保持缓冲 */
            Contrl_Pwm(0, 0, 0, 0);
            K230_Receive_Process();
            break;

        case TASK_LINE_LAP:
            /* 任务1: 纯循迹 (不控球) */
            Question_Task_Run();
            break;

        case TASK_BALL_CTRL:
            /* 任务2: 钢珠控制 (不走) */
            Question_Task_Run();
            break;

        case TASK_AB_STAB:
        case TASK_LAP_STAB:
        case TASK_LAP_POS:
            /* 任务3/4/5: 巡线 + 控球组合 */
            Question_Task_Run();
            break;

        default:
            break;
        }
    }
}
