#ifndef _KEY_H
#define _KEY_H

#include "ti_msp_dl_config.h"
#include "timer.h"
#include "led.h"
#include "usart.h"
#include "app_motor_usart.h"
#include "buzzer.h"
#include "questions.h"

/* 按键事件类型定义 */
typedef enum {
    KEY_EVENT_NONE,     // 无事件
    KEY_EVENT_SHORT,    // 短按事件
    KEY_EVENT_LONG      // 长按事件
} KeyEvent;

/* 按键状态机状态 */
typedef enum {
    KEY_STATE_RELEASED,     // 按键释放状态
    KEY_STATE_DEBOUNCE,     // 消抖检测状态
    KEY_STATE_PRESSED,      // 确认按下状态
    KEY_STATE_LONG          // 长按已触发状态
} KeyState;

/* 按键句柄结构体 */
typedef struct {
    GPIO_Regs* GPIOx;       // GPIO端口
    uint32_t GPIO_Pin;      // GPIO引脚
    KeyState state;          // 当前状态
    uint32_t pressTime;     // 按下时间戳(ms)
    uint32_t debounceTime;  // 消抖计时起点(ms)
} Key_t;

/* ---- 全局状态变量 (外部引用) ---- */
extern uint8_t g_mode_select;    /* 1=模式选择界面, 0=任务运行中 */
extern uint8_t g_selected_mode;  /* 0~4 对应任务2~6 */
extern uint8_t g_task_paused;    /* 1=暂停, 0=运行中 */
extern int g_prev_main_state;    /* 暂停前的主状态 */

KeyEvent Key_Scan(Key_t* key, uint32_t currentTime, uint32_t longPressThreshold);
void Key_Handle(void);

#endif
