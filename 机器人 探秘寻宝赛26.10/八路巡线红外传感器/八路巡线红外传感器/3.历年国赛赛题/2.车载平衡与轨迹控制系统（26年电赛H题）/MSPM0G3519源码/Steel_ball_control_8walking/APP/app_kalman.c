/* app_kalman.c - 卡尔曼滤波器实现 */
#include "app_kalman.h"
#include <string.h>

/* 初始化卡尔曼滤波器 */
/* q: 过程噪声(推荐0.001~0.1) */
/* r: 测量噪声(推荐0.001~0.1) */
void Kalman_Init(Kalman_t *kf, float q, float r)
{
    memset(kf, 0, sizeof(*kf));
    kf->q = q;
    kf->r = r;
    kf->p = 0.01f;     /* 初始协方差(不为0即可) */
    kf->first = 1;
}

/* 单步卡尔曼滤波迭代 */
/* in: 当前测量值, 返回: 滤波后的估计值 */
float Kalman_Filter(Kalman_t *kf, float in)
{
    if (kf->first) {                   /* 首次采样: 直接返回测量值 */
        kf->prev = in;
        kf->first = 0;
        return in;
    }
    kf->p = kf->p + kf->q;             /* 预测: 协方差增加过程噪声 */
    kf->kGain = kf->p / (kf->p + kf->r); /* 计算卡尔曼增益 */
    kf->prev = kf->prev + kf->kGain * (in - kf->prev); /* 更新估计 */
    kf->p = (1.0f - kf->kGain) * kf->p; /* 更新协方差 */
    return kf->prev;
}