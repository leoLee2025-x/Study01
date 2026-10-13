#ifndef _APP_IRTRACKING_H_
#define _APP_IRTRACKING_H_


#include "ti_msp_dl_config.h"
#include "app_motor.h"
extern int g_line_speed;
#define RAMP_STOP_TIME_MS 1500  /* 停车缓减速时间(ms) */
extern volatile uint8_t g_slow_stop;
#include "usart.h"

#define BLACK       1        //黑线 black
#define WHITE       0        //白线 white


#define u8 uint8_t
#define u16 uint16_t

#define IR_I2C_ADDR 0x12
float PID_IR_Calc(int16_t actual_value);
void IRI2C_WriteByte(uint8_t addr, uint8_t dat);
uint8_t IRI2C_ReadByte(uint8_t addr);
void printf_i2c_data(void);
void LineWalking(void);
void LineWalking_WithBall(float ball_target_cm);
int LineCheck(void);
int LineCheckCount(void);
int LineCheckStop(void);

#endif
