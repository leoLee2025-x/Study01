/* app_ball.h - Ball tracking state machine */
#ifndef __APP_BALL_H
#define __APP_BALL_H
#include "app_kalman.h"
#include "app_pid.h"
void BallTrack_Init(void);
void BallTrack_Run(void);
#endif
