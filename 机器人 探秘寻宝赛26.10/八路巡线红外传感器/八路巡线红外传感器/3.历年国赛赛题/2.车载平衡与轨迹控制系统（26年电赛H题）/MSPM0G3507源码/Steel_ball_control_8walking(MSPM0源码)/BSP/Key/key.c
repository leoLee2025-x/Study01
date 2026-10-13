/* key.c - 按键驱动 + 模式选择状态机
 * ============================================================
 * K1 按键 (PA18) 功能:
 *   模式选择界面:
 *     长按 = 切换模式 (1->2->3->4->5->1), OLED 同步更新
 *     短按 = 确认进入当前模式, 开始执行任务
 *   任务运行中:
 *     短按 = 暂停 / 继续 (切换 STOP_STATE)
 *     长按 = 退出任务, 回到模式选择界面
 *   暂停状态:
 *     短按 = 继续执行
 *     长按 = 退出回模式选择
 *
 * 全局状态变量 (被 oled.c 和 main.c 引用):
 *   g_mode_select    = 1 模式选择中 / 0 任务运行中
 *   g_selected_mode  = 0~4 对应显示模式 1~5
 *   g_task_paused    = 1 暂停中 / 0 运行中
 *   g_prev_main_state = 暂停前的主状态, 用于恢复
 *
 * GBK 编码 (Keil 可读)
 */
#include "key.h"
#include "oled.h"
#include <stdio.h>

/* ---- 全局状态变量 ---- */
uint8_t g_mode_select    = 1;   /* 1=模式选择界面, 0=任务运行中 */
uint8_t g_selected_mode  = 0;   /* 0=Mode1, 1=Mode2, 2=Mode3, 3=Mode4, 4=Mode5 */
uint8_t g_task_paused    = 0;   /* 1=暂停, 0=运行中 */
int    g_prev_main_state = 0;   /* 暂停前的主状态, 用于恢复 */

extern int g_LinePortal_flag;
extern volatile struct state_machine State_Machine;


/* ---- 按键句柄: K1 绑定 PA18 ---- */
Key_t key1 = {
    .GPIOx = KEY_PORT,              /* GPIOA */
    .GPIO_Pin = KEY_K1_PIN,         /* PA18 */
    .state = KEY_STATE_RELEASED,    /* 初始状态: 释放 */
    .pressTime = 0,
    .debounceTime = 0
};

#define DEBOUNCE_DELAY 20           /* 消抖时间 20ms */
#define LONG_PRESS_MS  700          /* 长按阈值 700ms */


/* ---- 任务状态映射表 ---- */
/* g_selected_mode 0~4 -> Main_State 枚举值 */
static const int task_state_map[5] = {
    TASK_LINE_LAP,      /* Mode 1 = 纯循迹 */
    TASK_BALL_CTRL,     /* Mode 2 = 钢珠控制 */
    TASK_AB_STAB,       /* Mode 3 = A->B + 控球 */
    TASK_LAP_STAB,      /* Mode 4 = 一圈 + 控球 */
    TASK_LAP_POS        /* Mode 5 = 指定位置 */
};


/* ============================================================
 * Key_Scan - 按键扫描 (非阻塞)
 *
 * 参数:
 *   key: 按键句柄指针
 *   currentTime: 当前系统时间 (ms, 由 Get_Time() 提供)
 *   longPressThreshold: 长按时间阈值 (ms)
 *
 * 返回:
 *   KEY_EVENT_NONE   = 无事件
 *   KEY_EVENT_SHORT  = 短按 (按下后释放)
 *   KEY_EVENT_LONG   = 长按 (按下超过阈值)
 *
 * 状态机: RELEASED -> DEBOUNCE -> PRESSED -> LONG
 * ============================================================ */
KeyEvent Key_Scan(Key_t* key, uint32_t currentTime, uint32_t longPressThreshold)
{
    /* GPIO 低电平 = 按下 */
    uint8_t isPressed = (DL_GPIO_readPins(key->GPIOx, key->GPIO_Pin) == 0);

    switch (key->state) {

    case KEY_STATE_RELEASED:
        /* 按键释放中 -> 检测到按下 -> 进入消抖 */
        if (isPressed) {
            key->state = KEY_STATE_DEBOUNCE;
            key->debounceTime = currentTime;
        }
        break;

    case KEY_STATE_DEBOUNCE:
        /* 消抖中 -> 20ms 后确认状态 */
        if (currentTime - key->debounceTime >= DEBOUNCE_DELAY) {
            if (isPressed) {
                key->state = KEY_STATE_PRESSED;     /* 确认按下 */
                key->pressTime = currentTime;
            } else {
                key->state = KEY_STATE_RELEASED;    /* 抖动误触发 */
            }
        }
        break;

    case KEY_STATE_PRESSED:
        /* 已按下 -> 释放 = 短按, 超时 = 长按 */
        if (!isPressed) {
            key->state = KEY_STATE_RELEASED;
            return KEY_EVENT_SHORT;                 /* 释放: 短按事件 */
        } else if (currentTime - key->pressTime >= longPressThreshold) {
            key->state = KEY_STATE_LONG;
            return KEY_EVENT_LONG;                  /* 超时: 长按事件 */
        }
        break;

    case KEY_STATE_LONG:
        /* 长按已触发 -> 等待释放后重置 */
        if (!isPressed) {
            key->state = KEY_STATE_RELEASED;
        }
        break;
    }

    return KEY_EVENT_NONE;
}


/* ============================================================
 * Key_Handle - 按键处理 (每 100ms 由 Scheduler_Run 调用)
 *
 * 模式选择流程:
 *   长按 -> g_selected_mode 循环 (0->1->2->3->4->0)
 *   短按 -> 设置 State_Machine.Main_State, 进入 SUB_INIT
 *
 * 任务运行流程:
 *   短按 -> 暂停/继续 (切换 STOP_STATE)
 *   长按 -> 退出回模式选择
 * ============================================================ */
void Key_Handle(void)
{
    static uint32_t lastTick = 0;
    uint32_t currentTick = Get_Time();

    /* 节流: 每 100ms 检测一次 */
    if (currentTick - lastTick < 100) return;
    lastTick = currentTick;

    KeyEvent event = Key_Scan(&key1, currentTick, LONG_PRESS_MS);

    /* ========== 模式选择界面 ========== */
    if (g_mode_select) {
        switch (event) {
        case KEY_EVENT_SHORT:
            /* 短按: 确认进入当前选中的模式 */
            g_mode_select = 0;                          /* 退出选择模式 */
            g_task_paused = 0;
            State_Machine.Main_State = task_state_map[g_selected_mode];
            State_Machine.Sub_State = SUB_IDLE;         /* 触发 SUB_INIT */
            Contrl_Pwm(0, 0, 0, 0);                    /* 确保停车 */
            printf("Enter Mode: %d\r\n", g_selected_mode + 1);
            break;

        case KEY_EVENT_LONG:
            /* 长按: 切换模式 (循环) */
            g_selected_mode = (g_selected_mode + 1) % 5;
            printf("Select Mode: %d\r\n", g_selected_mode + 1);
            break;

        default: break;
        }
        return;
    }

    /* ========== 任务运行中 ========== */
    if (!g_task_paused) {
        switch (event) {
        case KEY_EVENT_SHORT:
            /* 短按: 暂停任务, 保存当前状态 */
            g_task_paused = 1;
            g_prev_main_state = State_Machine.Main_State;
            State_Machine.Main_State = STOP_STATE;
            Contrl_Pwm(0, 0, 0, 0);
            printf("Paused\r\n");
            break;

        case KEY_EVENT_LONG:
            /* 长按: 退出任务, 回到模式选择 */
            g_mode_select = 1;
            g_task_paused = 0;
            State_Machine.Main_State = STOP_STATE;
            State_Machine.Sub_State = SUB_IDLE;
            Contrl_Pwm(0, 0, 0, 0);
            printf("Exit to Mode Select\r\n");
            break;

        default: break;
        }
        return;
    }

    /* ========== 暂停状态 ========== */
    if (g_task_paused) {
        switch (event) {
        case KEY_EVENT_SHORT:
            /* 短按: 恢复执行 */
            g_task_paused = 0;
            State_Machine.Main_State = g_prev_main_state;
            State_Machine.Sub_State = SUB_IDLE;         /* 重新初始化 */
            printf("Resumed\r\n");
            break;

        case KEY_EVENT_LONG:
            /* 长按: 退出 */
            g_mode_select = 1;
            g_task_paused = 0;
            State_Machine.Main_State = STOP_STATE;
            State_Machine.Sub_State = SUB_IDLE;
            Contrl_Pwm(0, 0, 0, 0);
            printf("Exit to Mode Select\r\n");
            break;

        default: break;
        }
    }
}
