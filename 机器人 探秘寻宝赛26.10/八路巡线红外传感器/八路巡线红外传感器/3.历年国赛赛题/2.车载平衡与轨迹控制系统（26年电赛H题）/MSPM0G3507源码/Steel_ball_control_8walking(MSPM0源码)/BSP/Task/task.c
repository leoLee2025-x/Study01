/* task.c - 任务调度器
 * ============================================================
 * 定时调用各个任务函数:
 *   Key_Handle   每100ms  按键检测 + 模式切换
 *   OLED_time    每200ms  OLED 显示更新
 *
 * 使用方法: 在 main.c 循环中调用 Scheduler_Run()
 */
#include "task.h"
#include "key.h"
#include "oled.h"

Task tasks[] = {
    {100, 0, Key_Handle},    /* 每100ms 执行按键检测 */
    {200, 0, OLED_time},     /* 每200ms 刷新 OLED 显示 */
};

void Scheduler_Run(void)
{
    uint32_t now = Get_Time();
    for (int i = 0; i < sizeof(tasks) / sizeof(Task); i++) {
        if (now - tasks[i].last_call >= tasks[i].interval) {
            tasks[i].task();
            tasks[i].last_call = now;
        }
    }
}
