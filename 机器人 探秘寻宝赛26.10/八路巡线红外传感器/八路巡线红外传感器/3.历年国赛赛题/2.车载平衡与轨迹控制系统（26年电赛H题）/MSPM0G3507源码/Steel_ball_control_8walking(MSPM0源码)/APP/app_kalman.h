/* app_kalman.h - 一维卡尔曼滤波器 */
#ifndef __APP_KALMAN_H
#define __APP_KALMAN_H
#include <stdint.h>

/* 卡尔曼滤波器结构体 */
typedef struct {
    float prev;      /* 上一帧估计值 */
    float p;         /* 估计误差协方差 */
    float q;         /* 过程噪声(越大越信任测量值, 响应越快) */
    float r;         /* 测量噪声(越大越平滑, 响应越慢) */
    float kGain;     /* 卡尔曼增益(自动计算) */
    uint8_t first;   /* 首次采样标志 */
} Kalman_t;

void Kalman_Init(Kalman_t *kf, float q, float r);
float Kalman_Filter(Kalman_t *kf, float in);

#endif