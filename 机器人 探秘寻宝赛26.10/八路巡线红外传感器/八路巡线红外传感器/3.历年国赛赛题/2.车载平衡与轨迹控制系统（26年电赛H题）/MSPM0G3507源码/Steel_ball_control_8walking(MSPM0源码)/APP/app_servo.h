/* app_servo.h - 舵机控制 TIMA0 CCP0 PB8 */
#ifndef __APP_SERVO_H
#define __APP_SERVO_H
#include <stdint.h>

void Servo_Init(void);           /* 初始化舵机(中位1500us) */
void Servo_SetPos(int offset);   /* 设置偏移量(中位+offset, 单位us) */

void Set_Servo_Angle(unsigned int angle);

#endif