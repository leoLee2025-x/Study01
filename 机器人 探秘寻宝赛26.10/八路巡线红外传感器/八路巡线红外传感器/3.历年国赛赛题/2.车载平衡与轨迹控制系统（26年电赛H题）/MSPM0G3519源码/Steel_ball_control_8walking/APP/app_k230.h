/* app_k230.h - K230 binary protocol parser (matches ai_mjpeg_web_server.py)
 *
 * Frame format:
 *   Test:    0x55 0xAA 0x01 xh  0    0    0xFA   (7 bytes, cx=xh*10)
 *   Coord:   0x55 0xAA 0x02 xh  xl   yh   yl   conf 0xFA (9 bytes)
 *
 * Usage in main.c:
 *   K230_Init();               // call once after USART_Init()
 *   K230_Receive_Process();    // call in main loop (non-blocking)
 *   K230_Servo_Control();      // call in main loop to control servo via K230
 */
#ifndef __APP_K230_H
#define __APP_K230_H
#include <stdint.h>

/* Initialize K230 UART3 interrupt */
void K230_Init(void);

/* Process received K230 binary frames (non-blocking, call in loop) */
void K230_Receive_Process(void);

/* Control servo based on latest K230 X coordinate */
void K230_Servo_Control(float target_cm);


/* Accessors */
float   K230_GetX(void);
uint8_t K230_IsValid(void);
void    K230_Clear(void);

void K230_SetTarget(float x);   /* Set PID target X position */
#endif
/* Backward compat */
void K230_UART_Init(void);
void K230_Parse(uint8_t byte);

/* Debug: UART3->UART1 echo for testing */
void K230_Echo_Test(void);
