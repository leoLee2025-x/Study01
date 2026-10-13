/* app_irtracking.c - 八路红外巡线传感器驱动 + 巡线PID + 组合任务
 * ============================================================
 * 硬件: 八路红外对管阵列, 通过 I2C (地址0x12) 读取
 *   x1 x2 x3 x4 x5 x6 x7 x8   (从左到右排列)
 *   0 = 检测到黑线, 1 = 白线(未检测到)
 *
 * 主要功能:
 *   1. I2C 读写红外传感器数据
 *   2. 巡线 PID 控制 (LineWalking)
 *   3. 巡线 + 钢珠控制组合任务 (LineWalking_WithBall)
 *   4. 黑线/白线状态检测 (LineCheck)
 *
 * 巡线 PID 参数 (在下方 #define 调节):
 *   IRTrack_Trun_KP = 250   比例: 越大纠偏越猛, 太大则蛇形
 *   IRTrack_Trun_KI = 0.2   积分: 消除稳态偏差
 *   IRTrack_Trun_KD = 2     微分: 抑制振荡, 增加阻尼
 *   IRR_SPEED = 300          巡线基础速度 (mm/s)
 *
 * 钢珠控制 PID 参数 (在 APP/app_k230.c 顶部调节):
 *   PID_KP / PID_KI / PID_KD  舵机PID参数
 *   K230_CENTER               图像中心像素
 *
 * 使用方法:
 *   纯巡线:     LineWalking();
 *   巡线+控球:  LineWalking_WithBall(0.0f);  // 0=钢珠在中心
 *
 * 注意:
 *   - 传感器返回 0=有黑线, 1=没黑线, 与直觉相反
 *   - 巡线PID输出经 Motion_Car_Control 控制电机
 *   - LineCheck() 返回 BLACK(1)=有黑线, WHITE(0)=全白
 *
 * 2026/8/7 - 添加详细注释 + LineWalking_WithBall
 */
#include "app_irtracking.h"
#include "app_k230.h"
#include "timer.h"

/* ===== 巡线 PID 参数 ===== */
#define IRTrack_Trun_KP   (150)    /* 比例系数: 偏差1个单位时输出250, 越大反应越猛 */
#define IRTrack_Trun_KI   (0.15)    /* 积分系数: 消除稳态偏差, 太大会过冲 */
#define IRTrack_Trun_KD   (1.5)      /* 微分系数: 抑制振荡, 太大会抖动 */
#define IRR_SPEED         250      /* 巡线基础速度(mm/s), 值越小越慢越稳 */
#define BALL_INERTIA_FWD  -0.60f     /* 钢珠前倾补偿(cm), 小车前进时球前倾, 目标后移抵消 */
#define BALL_INERTIA_TURN -1.0f     /* 钢珠转弯补偿(cm), 转弯时球向外甩, 目标内移抵消 */
#define RAMP_TIME_MS     3000    /* 起步缓加速时间(ms), 越大越缓 */
#define RAMP_STOP_TIME_MS 1500   /* 停车缓减速时间(ms), 越大停车越缓 */
#define BALL_START_COMP 	-5.0f       /* 起步补偿(cm), 前2s车加速球滞后 */
#define BALL_STOP_COMP  	5.0f       /* 停车补偿(cm), 车停球前冲 */
#define CHANGE_THRESHOLD  3        /* 传感器变化阈值(未使用, 预留) */

/* ===== PID 限幅 ===== */
const float pid_out_max   = 5000.0f;  /* PID 输出上限, 对应最大转弯力度 */
const float Integral_max  = 500.0f;   /* 积分项上限, 防止积分饱和(windup) */

/* ===== 全局变量 ===== */
int   pid_output_IRR = 0;
int   g_line_speed   = 211;            /* 巡线速度 */
volatile uint8_t g_slow_stop = 0;      /* 1=进入缓停车阶段 */
u8    trun_flag      = 0;            /* 转弯标志(预留) */
static int8_t err         = 0;            /* 巡线偏差值(-4~+4, 负=偏左, 正=偏右) */
static uint8_t prev0_sensor_data = 0;      /* 上一次传感器读数(用于变化检测) */




/* ============================================================
 * 黑线传感器计数
 * 返回: 检测到黑线的探头数量 (0~8)
 *
 * 用途: 判断停止线/途经点 (>=4个探头=在停止线上)
 * ============================================================ */


/* ============================================================
 * IR 巡线 PID 计算
 * 输入: actual_value = 当前偏差值(err), 范围约 -4 ~ +4
 * 输出: PID 计算结果, 用于 Motion_Car_Control 的 V_z 参数
 *       正数=右转, 负数=左转
 *
 * 公式(位置式PID):
 *   输出 = Kp*偏差 + Ki*∫偏差 + Kd*(偏差-上次偏差)
 * ============================================================ */
float PID_IR_Calc(int16_t actual_value)
{
    float pid_out = 0;
    int16_t error;
    static int16_t error_last = 0;       /* 上一帧的偏差值 */
    static float   Integral   = 0;       /* 积分累加值 */

    error = actual_value;

    /* 积分累加 + 限幅 */
    Integral += error;
    if (Integral >  Integral_max) Integral =  Integral_max;
    if (Integral < -Integral_max) Integral = -Integral_max;

    /* 位置式 PID: P + I + D */
    pid_out = error * IRTrack_Trun_KP
            + IRTrack_Trun_KI * Integral
            + (error - error_last) * IRTrack_Trun_KD;

    error_last = error;                  /* 保存本次偏差, 供下一帧D项使用 */

    /* 输出限幅 */
    if (pid_out >  pid_out_max) pid_out =  pid_out_max;
    if (pid_out < -pid_out_max) pid_out = -pid_out_max;

    return pid_out;
}


/* ============================================================
 * I2C 写一个字节到红外传感器
 * addr: 寄存器地址
 * dat:  要写入的数据
 * ============================================================ */
void IRI2C_WriteByte(uint8_t addr, uint8_t dat)
{
    uint8_t temp[2];
    temp[0] = addr;
    temp[1] = dat;

    DL_I2C_fillControllerTXFIFO(Sensor_INST, temp, 2);
    while (!(DL_I2C_getControllerStatus(Sensor_INST) & DL_I2C_CONTROLLER_STATUS_IDLE));

    DL_I2C_startControllerTransfer(Sensor_INST, IR_I2C_ADDR, DL_I2C_CONTROLLER_DIRECTION_TX, 2);
    while (DL_I2C_getControllerStatus(Sensor_INST) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS);
    while (!(DL_I2C_getControllerStatus(Sensor_INST) & DL_I2C_CONTROLLER_STATUS_IDLE));
    DL_I2C_flushControllerTXFIFO(Sensor_INST);
}


/* ============================================================
 * I2C 从红外传感器读一个字节
 * addr: 寄存器地址
 * 返回: 读到的数据
 * ============================================================ */
uint8_t IRI2C_ReadByte(uint8_t addr)
{
    uint8_t data;

    /* 发送寄存器地址 */
    DL_I2C_fillControllerTXFIFO(Sensor_INST, &addr, 1);
    DL_I2C_disableInterrupt(Sensor_INST, DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER);

    while (!(DL_I2C_getControllerStatus(Sensor_INST) & DL_I2C_CONTROLLER_STATUS_IDLE));
    DL_I2C_startControllerTransfer(Sensor_INST, IR_I2C_ADDR, DL_I2C_CONTROLLER_DIRECTION_TX, 1);
    while (DL_I2C_getControllerStatus(Sensor_INST) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS);
    while (!(DL_I2C_getControllerStatus(Sensor_INST) & DL_I2C_CONTROLLER_STATUS_IDLE));

    /* 读取数据 */
    DL_I2C_startControllerTransfer(Sensor_INST, IR_I2C_ADDR, DL_I2C_CONTROLLER_DIRECTION_RX, 1);
    while (DL_I2C_getControllerStatus(Sensor_INST) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS);
    while (!(DL_I2C_getControllerStatus(Sensor_INST) & DL_I2C_CONTROLLER_STATUS_IDLE));
    data = DL_I2C_receiveControllerData(Sensor_INST);

    return data;
}


/* ============================================================
 * 读取八路红外传感器数据
 * 从 I2C 地址 0x30 读取一个字节, 按位拆分为8路
 *
 * 输出:
 *   x1(MSB) x2 x3 x4 x5 x6 x7 x8(LSB)
 *   每一路: 0=检测到黑线, 1=白线(未检测到)
 *
 * 传感器物理排列(俯视):
 *   [x1] [x2] [x3] [x4] [x5] [x6] [x7] [x8]
 *    <-------------- 车头前方 -------------->
 * ============================================================ */
void deal_IRdata(u8 *x1, u8 *x2, u8 *x3, u8 *x4,
                 u8 *x5, u8 *x6, u8 *x7, u8 *x8)
{
    u8 IRbuf = 0xFF;
    IRbuf = IRI2C_ReadByte(0x30);        /* 寄存器0x30 = 八路传感器状态 */

    *x1 = (IRbuf >> 7) & 0x01;           /* 最左 */
    *x2 = (IRbuf >> 6) & 0x01;
    *x3 = (IRbuf >> 5) & 0x01;
    *x4 = (IRbuf >> 4) & 0x01;           /* 中间偏左 */
    *x5 = (IRbuf >> 3) & 0x01;           /* 中间偏右 */
    *x6 = (IRbuf >> 2) & 0x01;
    *x7 = (IRbuf >> 1) & 0x01;
    *x8 = (IRbuf >> 0) & 0x01;           /* 最右 */
}


/* ============================================================
 * 打印八路红外传感器原始数据 (调试用)
 * 输出格式: x1:0,x2:1,x3:1,x4:1,x5:1,x6:1,x7:1,x8:1
 * ============================================================ */
void printf_i2c_data(void)
{
    static uint8_t ir_x1, ir_x2, ir_x3, ir_x4,
                   ir_x5, ir_x6, ir_x7, ir_x8;
    deal_IRdata(&ir_x1, &ir_x2, &ir_x3, &ir_x4,
                &ir_x5, &ir_x6, &ir_x7, &ir_x8);
    printf("x1:%d,x2:%d,x3:%d,x4:%d,x5:%d,x6:%d,x7:%d,x8:%d\r\n",
           ir_x1, ir_x2, ir_x3, ir_x4, ir_x5, ir_x6, ir_x7, ir_x8);
}


/* ============================================================
 * 巡线行走 (纯巡线, 不控钢珠)
 *
 * 逻辑:
 *   1. 读取八路红外传感器
 *   2. 根据传感器状态计算偏差值 err (-4 ~ +4)
 *      err < 0 = 车身偏左, 需要右转
 *      err > 0 = 车身偏右, 需要左转
 *   3. PID 计算转弯量
 *   4. Motion_Car_Control 控制电机
 *
 * 偏差值映射表:
 *   x4或x5=0(中间有黑线) → err=0    直行
 *   x3=0(左偏一格)        → err=-1   微右转
 *   x3x4=0(左偏两格)      → err=-2   右转
 *   x2=0(更左偏)          → err=-2
 *   x2x3=0(大幅左偏)      → err=-3   急右转
 *   x6=0(右偏一格)        → err=+1   微左转
 *   x5x6=0(右偏两格)      → err=+2   左转
 *   x7=0(更右偏)          → err=+2
 *   x6x7=0(大幅右偏)      → err=+3   急左转
 *
 * 调节方向:
 *   转弯不够 → 加大 IRTrack_Trun_KP
 *   蛇形走位 → 加大 IRTrack_Trun_KD, 减小 IRTrack_Trun_KP
 *   偏一边  → 加大 IRTrack_Trun_KI
 * ============================================================ */
void LineWalking(void)
{
    static int8_t err = 0;
    static u8 x1, x2, x3, x4, x5, x6, x7, x8;

    /* 第1步: 读取传感器 */
    deal_IRdata(&x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8);

    /* 第2步: 根据传感器状态计算偏差值 */
    /* 优先级: 中间直行 > 偏左 > 偏右 > 保持上次 */

    /* --- 中间有黑线: 直行 --- */
    if (x4 == 0 || x5 == 0) {
        err = 0;
    }
    /* --- 车身偏左, 需要右转 (err为负) --- */
    else if (x1 == 1 && x2 == 1 && x3 == 1 && x4 == 0 && x5 == 1 && x6 == 1 && x7 == 1 && x8 == 1) {
        err = -1;                        /* 1110_1111: 微偏左 */
    }
    else if (x1 == 1 && x2 == 1 && x3 == 0 && x4 == 0 && x5 == 1 && x6 == 1 && x7 == 1 && x8 == 1) {
        err = -2;                        /* 1100_1111: 偏左 */
    }
    else if (x1 == 1 && x2 == 1 && x3 == 0 && x4 == 1 && x5 == 1 && x6 == 1 && x7 == 1 && x8 == 1) {
        err = -2;                        /* 1101_1111: 偏左 */
    }
    else if (x1 == 1 && x2 == 0 && x3 == 0 && x4 == 1 && x5 == 1 && x6 == 1 && x7 == 1 && x8 == 1) {
        err = -3;                        /* 1001_1111: 大幅偏左 */
    }
    /* --- 车身偏右, 需要左转 (err为正) --- */
    else if (x1 == 1 && x2 == 1 && x3 == 1 && x4 == 1 && x5 == 0 && x6 == 1 && x7 == 1 && x8 == 1) {
        err = 1;                         /* 1111_0111: 微偏右 */
    }
    else if (x1 == 1 && x2 == 1 && x3 == 1 && x4 == 1 && x5 == 0 && x6 == 0 && x7 == 1 && x8 == 1) {
        err = 2;                         /* 1111_0011: 偏右 */
    }
    else if (x1 == 1 && x2 == 1 && x3 == 1 && x4 == 1 && x5 == 1 && x6 == 0 && x7 == 1 && x8 == 1) {
        err = 2;                         /* 1111_1011: 偏右 */
    }
    else if (x1 == 1 && x2 == 1 && x3 == 1 && x4 == 1 && x5 == 1 && x6 == 0 && x7 == 0 && x8 == 1) {
        err = 3;                         /* 1111_1001: 大幅偏右 */
    }
    /* --- 兜底: 单探头判断 --- */
    else if (x3 == 0) {
        err = -1;                        /* x3=0: 偏左 */
    }
    else if (x2 == 0) {
        err = -2;                        /* x2=0: 更偏左 */
    }
    else if (x7 == 0) {
        err = 4;                         /* x7=0: 偏右(数值较大, 急转) */
    }
    /* --- 其他情况: 保持上一次偏差值不变 --- */

    /* 第3步: PID 计算转弯量 */
    pid_output_IRR = (int)(PID_IR_Calc(err));

    /* 第4步: 控制电机 (V_x=速度, V_y=0, V_z=转弯量) */
    Motion_Car_Control(g_line_speed, 0, pid_output_IRR);
}


/* ============================================================
 * 黑线检测
 * 返回: BLACK(1) = 至少一个探头检测到黑线
 *       WHITE(0) = 全部探头都在白线上(全1)
 *
 * 用途: 判断是否到达十字/丁字路口(途经点)
 * ============================================================ */
int LineCheck(void)
{
    static u8 x1, x2, x3, x4, x5, x6, x7, x8;
    deal_IRdata(&x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8);

    /* 全部为1 = 都在白线上 = 未检测到黑线 */
    if (x1 && x2 && x3 && x4 && x5 && x6 && x7 && x8) {
        return WHITE;
    } else {
        return BLACK;
    }
}



/* ============================================================
 * 黑线传感器计数 (0~8)
 * 5cm 停止线覆盖 4~5 个探头, >=4 判定到达
 * ============================================================ */
int LineCheckCount(void)
{
    static u8 x1, x2, x3, x4, x5, x6, x7, x8;
    deal_IRdata(&x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8);
    return (1-x1)+(1-x2)+(1-x3)+(1-x4)+(1-x5)+(1-x6)+(1-x7)+(1-x8);
}

int LineCheckStop(void)
{
    return (LineCheckCount() >= 4) ? BLACK : WHITE;
}


/* ============================================================
 * 循迹 + 钢珠控制组合任务
 * 同时IR巡线 + K230舵机控球 + 起步/转弯惯性补偿
 * ============================================================ */
void LineWalking_WithBall(float ball_target_cm)
{
    float compensated = ball_target_cm;
    compensated -= BALL_INERTIA_FWD;
    if (pid_output_IRR > 0)
        compensated -= BALL_INERTIA_TURN * (float)pid_output_IRR / 5000.0f;
    else if (pid_output_IRR < 0)
        compensated += BALL_INERTIA_TURN * (float)(-pid_output_IRR) / 5000.0f;

    /* 起步缓加速 + 停车缓减速 */
    {   static uint32_t lt0 = 0;
        static int saved_spd = 0;
        static uint32_t st0 = 0;
        static uint8_t last_stop = 0;
        extern volatile uint32_t g_task_start_ms;
        extern volatile uint8_t g_slow_stop;
        if (g_task_start_ms != 0 && g_task_start_ms != lt0) {
            lt0 = g_task_start_ms;
            saved_spd = g_line_speed;
            st0 = 0;
            last_stop = 0;
        }
        if (g_slow_stop && !last_stop) {
            st0 = Get_Time();
        }
        last_stop = g_slow_stop;

        uint32_t et = Get_Time() - lt0;
        if (g_slow_stop) {
            uint32_t st = Get_Time() - st0;
            if (st < RAMP_STOP_TIME_MS) {
                float r = 1.0f - (float)st / (float)RAMP_STOP_TIME_MS;
                if (r < 0.1f) r = 0.1f;
                g_line_speed = (int)((float)saved_spd * r);
            } else {
                g_line_speed = (int)((float)saved_spd * 0.1f);
            }
        } else if (et < RAMP_TIME_MS) {
            float ramp = (float)et / (float)RAMP_TIME_MS;
            if (ramp < 0.15f) ramp = 0.15f;
            if (ramp > 1.0f) ramp = 1.0f;
            g_line_speed = (int)((float)saved_spd * ramp);
        }
    }

    LineWalking();
    K230_Receive_Process();
    K230_Servo_Control(compensated);
}

