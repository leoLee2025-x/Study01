/* app_k230.c - K230 二进制协议解析 + PID 舵机控制 (中断接收版)
 * ============================================================
 * K230 摄像头通过 UART3 发送钢珠 X 坐标数据，本模块：
 *   1. UART3 中断自动接收数据到环形缓冲区 (后台, 不丢帧)
 *   2. 接收并解析 0x55 0xAA 帧头的二进制协议
 *   3. 使用卡尔曼滤波器平滑原始 X 坐标
 *   4. 使用位置式 PID 控制舵机将钢珠推到目标位置
 *
 * 协议格式:
 *   Test:  0x55 0xAA 0x01 xh    0    0    0xFA   (7 bytes, cx=xh*10)
 *   Coord: 0x55 0xAA 0x02 xh    xl   yh   yl   conf 0xFA (9 bytes)
 *
 * 中断接收说明:
 *   UART3 RX 中断在后台持续接收 K230 数据到 g_buf 环形缓冲区,
 *   即使主循环在巡线(I2C阻塞)也不会丢帧.
 *   K230_Receive_Process() 和 K230_Servo_Control() 直接解析缓冲区.
 *
 * 硬件参数:
 *   K230 像素范围:  99 ~ 599 px
 *   K230 视野:      28 cm
 *   舵机 PWM:       25us/tick, 20~100 CC (500~2500us, 0~180度)
 *   PID 输出缩放:   0.80 (PID out -> CC offset)
 *
 * PID 调参说明:
 *   PID_KP = 比例: 越大响应越快, 太大则振荡 (位置式PID)
 *   PID_KI = 积分: 消除稳态误差, 太大则过冲 (仅用极小值防偏移)
 *   PID_KD = 微分: 抑制振荡, 增加阻尼 (主要阻尼来源)
 *   如果钢珠运动方向相反, 将 KP/KI/KD 全部取反即可
 * ============================================================ */
#include "app_k230.h"
#include "app_servo.h"
#include "app_pid.h"
#include "app_kalman.h"
#include "ti_msp_dl_config.h"
#include <stdio.h>
#include <string.h>

/* ===== PID 参数 (位置式PID) ===== */
#define PID_KP        0.08f    /* 比例系数: 主导响应力度 */
#define PID_KI        0.003f   /* 积分系数: 极小值, 仅消除稳态偏移 */
#define PID_KD        2.86f     /* 微分系数: 主要阻尼, 抑制过冲和振荡 */

#define K230_CENTER    360.0f   /* K230 图像中心 X 像素 */
#define K230_MIN        99.0f   /* K230 图像左边界 X 像素 */
#define K230_MAX       599.0f   /* K230 图像右边界 X 像素 */
#define FOV_CM          28.0f   /* 摄像头视野宽度(cm) */
#define PIPE_HALF_CM    12.5f   /* 水管半长(cm), 25cm水管的一半 */
#define PX_PER_CM_LEFT  18.52f  /* 左侧 (330-99)/12.5 */
#define PX_PER_CM_RIGHT 21.48f  /* 右侧 (599-330)/12.5 */

#define PWM_CC_CENTER   60.0f   /* 舵机 CC 中心值(1500us) */
#define PWM_CC_MIN      20.0f   /* 舵机 CC 最小值(500us) */
#define PWM_CC_MAX     100.0f   /* 舵机 CC 最大值(2500us) */
#define PWM_CC_SCALE     0.80f  /* PID 输出 -> CC 缩放系数 */

#define DEADBAND_PX      3.0f   /* 死区(像素), 在此范围内不动作 (约1.5mm) */
#define RATE_LIMIT       6.0f   /* 单帧 CC 变化率限制, 防止舵机猛转 */
#define KALMAN_Q         1.0f   /* K230 坐标滤波: 过程噪声(增大则响应更快) */
#define KALMAN_R         0.3f   /* K230 坐标滤波: 测量噪声(增大则更平滑) */

/* ================================================= */
#define K230_BUF_SIZE 256
static uint8_t  g_buf[K230_BUF_SIZE];
static volatile uint16_t g_wr;   /* volatile: ISR 和主循环共享 */
static float    g_x;
static uint8_t  g_valid;

/* ============================================================
 * UART3 中断服务函数 (K230 数据接收)
 * 后台自动接收 K230 串口数据到环形缓冲区,
 * 不依赖主循环轮询, LineWalking 阻塞时也不会丢帧
 * ============================================================ */
void UART3_IRQHandler(void)
{
    switch (DL_UART_getPendingInterrupt(UART_K230_INST)) {
    case DL_UART_IIDX_RX:                              /* RX 接收中断 */
        while (DL_UART_Main_isRXFIFOEmpty(UART_K230_INST) == false) {
            uint8_t byte = DL_UART_Main_receiveData(UART_K230_INST);
            if (g_wr < K230_BUF_SIZE - 1)
                g_buf[g_wr++] = byte;                  /* 写入缓冲区 */
            else
                g_wr = 0;                              /* 缓冲区满, 从头覆盖 */
        }
        break;
    default:
        break;
    }
}


/* ---- 查找 0x55 0xAA 帧头 ---- */
static int find_hdr(uint8_t *buf, int len, int start)
{
    for (int i = start; i < len - 1; i++)
        if (buf[i] == 0x55 && buf[i + 1] == 0xAA) return i;
    return -1;
}

/* ---- 解析一帧数据 ---- */
/* 返回值: >0=已解析字节数, 0=数据不足, -1=未知帧ID */
static int parse_one(uint8_t *buf, int len)
{
    if (len < 7) return 0;
    int h = find_hdr(buf, len, 0);
    if (h < 0) return 0;
    uint8_t id = buf[h + 2];
    int n = 0;
    if (h + 6 < len && id == 0x01 && buf[h + 6] == 0xFA) {
        g_x = (float)(buf[h + 3] * 10); g_valid = 1; n = 7;
    } else if (h + 8 < len && id == 0x02 && buf[h + 8] == 0xFA) {
        g_x = (float)(buf[h + 3] * 10 + buf[h + 4]); g_valid = 1; n = 9;
    } else {
        return (id == 0x01 || id == 0x02) ? 0 : -1;
    }
    memmove(buf, buf + h + n, len - h - n);
    g_wr -= (h + n);
    return n;
}


/* ---- 初始化 (开启 UART3 RX 中断) ---- */
void K230_Init(void)
{
    NVIC_ClearPendingIRQ(UART_K230_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_K230_INST_INT_IRQN);
    DL_UART_Main_enableInterrupt(UART_K230_INST, DL_UART_MAIN_INTERRUPT_RX);
}


/* ---- 接收处理: 只解析缓冲区, 不收数据 (ISR 已收好) ---- */
void K230_Receive_Process(void)
{
    DL_UART_Main_disableInterrupt(UART_K230_INST, DL_UART_MAIN_INTERRUPT_RX);
    int r;
    do { r = parse_one(g_buf, g_wr); } while (r > 0);
    if (g_wr > 120) g_wr = 0;          /* 缓冲区异常, 重置 */
    DL_UART_Main_enableInterrupt(UART_K230_INST, DL_UART_MAIN_INTERRUPT_RX);
}


/* ---- 共享 PID/卡尔曼状态 ---- */
static uint8_t g_inited = 0;
static PID_t    g_pid;
static Kalman_t kf_x;
static float    cc_last = PWM_CC_CENTER;  /* 上一次 CC 值(用于速率限制) */

/* ================================================================
 * K230 PID 舵机控制 (位置式PID + 中断接收)
 *
 * PID 类型: 位置式 (positional)
 *   u = Kp*e + Ki*Sum(e) + Kd*(e - e_last)
 *
 * 保护机制:
 *   - UART3 中断自动收数据, 不丢帧
 *   - 死区 +/-3px: 误差过小不动作
 *   - 速率限制: 单帧 CC 变化不超过 RATE_LIMIT
 *   - 卡尔曼滤波: 平滑 X 坐标
 *   - 积分方向复位: 误差改变方向时清零积分
 *
 * 调参 (修改文件顶部 #define):
 *   PID_KP: 增大 = 响应更快
 *   PID_KI: 增大 = 收敛更快
 *   PID_KD: 增大 = 阻尼更强
 *   如果钢珠往反方向跑, 将 KP/KI/KD 全部取反
 *
 * 校准:
 *   K230: 99~599px 对应 FOV_CM cm 视野
 *   PWM:  20~100 CC 对应 500~2500us (0~180度)
 *   target_cm: 0=中心, 正=右侧, 负=左侧
 * ================================================================ */
void K230_Servo_Control(float target_cm)
{
    DL_UART_Main_disableInterrupt(UART_K230_INST, DL_UART_MAIN_INTERRUPT_RX);
    /* 只解析缓冲区, 不收数据 (ISR 已收好) */
    int r;
    do { r = parse_one(g_buf, g_wr); } while (r > 0);
    if (g_wr > 120) { g_wr = 0; }
    DL_UART_Main_enableInterrupt(UART_K230_INST, DL_UART_MAIN_INTERRUPT_RX);
    if (!g_valid) return;

    /* 一次初始化 */
    if (!g_inited) {
        PID_Init(&g_pid);
        Kalman_Init(&kf_x, KALMAN_Q, KALMAN_R);
        g_inited = 1;
    }

    /* 坐标转换: cm 目标 -> K230 像素目标 */
    float center    = K230_CENTER;
    /* 左右两边 px/cm 不同(摄像头不完全居中), 分开计算 */
        /* 左右两边 px/cm 不同, 分开计算 */
    float px_per_cm;
    if (target_cm >= 0.0f)
        px_per_cm = (K230_MAX - K230_CENTER) / PIPE_HALF_CM;  /* 右侧 */
    else
        px_per_cm = (K230_CENTER - K230_MIN) / PIPE_HALF_CM;  /* 左侧 */
    float target_px = center + target_cm * px_per_cm;
    float x_flt     = Kalman_Filter(&kf_x, g_x);     /* 卡尔曼滤波平滑 */
    float error     = target_px - x_flt;              /* >0=球偏左, 需右推 */

    /* 死区检测 */
    if (error > -DEADBAND_PX && error < DEADBAND_PX) {
        g_valid = 0;
        return;
    }

    /* PID 参数实时更新 */
    g_pid.KP = PID_KP;
    g_pid.KI = PID_KI;
    g_pid.KD = PID_KD;
    g_pid.Target = target_px;


    /* 位置式 PID 计算 */
    float cc = PID_To_CC_Pos(&g_pid, x_flt);

    /* 速率限制 */
    float delta = cc - cc_last;
    float max_step = RATE_LIMIT;
    if (delta >  max_step) cc = cc_last + max_step;
    if (delta < -max_step) cc = cc_last - max_step;
    cc_last = cc;

    /* CC 限幅 + 写入 PWM (TIMA0 PB8) */
    if (cc < PWM_CC_MIN) cc = PWM_CC_MIN;
    if (cc > PWM_CC_MAX) cc = PWM_CC_MAX;
    DL_TimerA_setCaptureCompareValue(PWM_Servo_INST, (uint16_t)(cc + 0.5f), DL_TIMER_CC_0_INDEX);

    /* 调试输出: 每15帧打印一次 (cc 放大10倍显示, 如 605=60.5) */
    static uint16_t _d = 0;
    if (++_d >= 15) {
        printf("x=%d e=%d cc=%d\r\n", (int)x_flt, (int)error, (int)(cc * 10));
        _d = 0;
    }
    g_valid = 0;
}


/* ---- 调试: UART3 回显测试 ---- */
void K230_Echo_Test(void)
{
    if (g_wr > 0 && g_buf[0] != 0) {
        printf("%02X\r\n", g_buf[0]);
        memmove(g_buf, g_buf + 1, g_wr - 1);
        g_wr--;
    }
}


/* ---- 访问器 ---- */
float   K230_GetX(void)   { return g_x; }
uint8_t K230_IsValid(void) { return g_valid; }
void    K230_Clear(void)   { g_valid = 0; }
void    K230_UART_Init(void){ K230_Init(); }
void    K230_Parse(uint8_t byte) { (void)byte; }
