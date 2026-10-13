/**			                                                    
		   ____                    _____ _______ _____       XTARK@���˴���
		  / __ \                  / ____|__   __|  __ \ 
		 | |  | |_ __   ___ _ __ | |       | |  | |__) |
		 | |  | | '_ \ / _ \ '_ \| |       | |  |  _  / 
		 | |__| | |_) |  __/ | | | |____   | |  | | \ \ 
		  \____/| .__/ \___|_| |_|\_____|  |_|  |_|  \_\
				| |                                     
				|_|                OpenCTR   �����˿�����
									 
  ****************************************************************************** 
  *           
  * ��Ȩ���У� XTARK@���˴���  ��Ȩ���У�����ؾ�
  * ��˾��վ�� www.xtark.cn   www.tarkbot.com
  * �Ա����̣� https://xtark.taobao.com  
  * ����΢�ţ� ���˴��£���ע���ںţ���ȡ���¸�����Ѷ��
  *      
  ******************************************************************************
  * @��  ��  ���˴����Ŷ�
  * @��  ��  �����˿���������
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AX_ROBOT_H
#define __AX_ROBOT_H

/* Includes ------------------------------------------------------------------*/	 
#include "stm32f4xx.h"

//C�⺯�������ͷ�ļ�
#include <stdio.h> 
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//�������ͷ�ļ�
#include "ax_sys.h"      //ϵͳ����
#include "ax_delay.h"    //������ʱ
#include "ax_led.h"      //LED�ƿ���
#include "ax_beep.h"     //����������
#include "ax_uart1.h"    //���Դ���

#include "ax_motor.h"    //ֱ��������ٿ���
#include "ax_encoder.h"  //����������
#include "ax_servo.h"    //�������

#include "ax_ps2.h"      //PS2�ֱ�


//�����������ٶ����ݽṹ��
typedef struct  
{
	double  RT;       //����ʵʱ�ٶȣ���λm/s
	float  TG;       //����Ŀ���ٶȣ���λm/s
	short  PWM;      //����PWM�����ٶ�
	
}ROBOT_Wheel;

//�������ٶȽṹ��
typedef struct  
{
	short  RT_IX;     //ʵʱX���ٶȣ�16λ������
	short  RT_IY;     //ʵʱY���ٶȣ�16λ������
	short  RT_IW;     //ʵʱYaw��ת���ٶȣ�16λ������
	
	short  TG_IX;     //Ŀ��X���ٶȣ�16λ������
	short  TG_IY;     //Ŀ��Y���ٶȣ�16λ������
	short  TG_IW;     //Ŀ��Yaw��ת���ٶȣ�16λ������
	
	float  RT_FX;     //ʵʱX���ٶȣ����㣩
	float  RT_FY;     //ʵʱY���ٶȣ����㣩
	float  RT_FW;     //ʵʱYaw��ת���ٶȣ����㣩
	
	float  TG_FX;     //Ŀ��X���ٶȣ����㣩
	float  TG_FY;     //Ŀ��Y���ٶȣ����㣩
	float  TG_FW;     //Ŀ��Yaw��ת���ٶȣ����㣩
	
}ROBOT_Velocity;

//����
#define  PI           3.1416     //Բ����PI
#define  SQRT3        1.732      //3��ƽ��
#define  PID_RATE     50         //PIDƵ��

/******����������***********************************/

//����������
#define ROBOT_MEC   0x01   //�����ķ�ֵ���
#define ROBOT_FWD   0x02   //���ֲ��ٵ���
#define ROBOT_TWD   0x03   //���ֲ���ת�����
#define ROBOT_AKM   0x04   //������ת�����
#define ROBOT_OMT   0x05   //����ȫ�����

//�����˶�������
#define ROBOT_TYPE   ROBOT_FWD

/******�����˲���*************************************/

//���ֻ����˲���
#define  MEC_WHEEL_BASE           0.182	      //�־࣬�����ֵľ���
#define  MEC_ACLE_BASE            0.124       //��࣬ǰ���ֵľ���
#define  MEC_WHEEL_DIAMETER	      0.080		  //����ֱ��
#define  MEC_WHEEL_RESOLUTION     1560.0      //�������ֱ���(13��),���ٱ�30,13x30x4=1560
#define  MEC_WHEEL_SCALE          (PI*MEC_WHEEL_DIAMETER*PID_RATE/MEC_WHEEL_RESOLUTION) //�����ٶ�m/s�������ת��ϵ��

//���ֲ��ٻ����˲���
#define  FWD_WHEEL_BASE           0.162	     //�־࣬�����ֵľ���
#define  FWD_WB_SCALE             1.75       //�־�ϵ�����־�ϵ��������˵��ܸ��ء���̥���������Ħ��ϵ����ת��뾶������λ�ö����й�ϵ��һ���ǳ����ӵĲ��������Գ��õķ���������ʵ��
#define  FWD_WHEEL_DIAMETER	      0.065		 //����ֱ��
#define  FWD_WHEEL_RESOLUTION     1560.0     //�������ֱ���(13��),���ٱ�30,13x30x4=1560
#define  FWD_WHEEL_SCALE          (PI*FWD_WHEEL_DIAMETER*PID_RATE/FWD_WHEEL_RESOLUTION) //�����ٶ�m/s�������ת��ϵ��

//���ֲ��ٻ����˲��� 
#define  TWD_WHEEL_DIAMETER	      0.0724	 //����ֱ��
#define  TWD_WHEEL_BASE           0.206	     //�־࣬�����ֵľ���
#define  TWD_WHEEL_RESOLUTION     1560.0      //�������ֱ���(13��),���ٱ�30,13x30x4=1560
#define  TWD_WHEEL_SCALE          (PI*TWD_WHEEL_DIAMETER*PID_RATE/TWD_WHEEL_RESOLUTION)  //�����ٶ�m/s�������ת��ϵ��

//�����������˲���
#define  AKM_WHEEL_BASE           0.165	     //�־࣬�����ֵľ���
#define  AKM_ACLE_BASE            0.175f     //��࣬ǰ���ֵľ���
#define  AKM_WHEEL_DIAMETER	      0.075		 //����ֱ��
#define  AKM_WHEEL_RESOLUTION     1560.0      //�������ֱ���(13��),���ٱ�30,13x30x4=1560
#define  AKM_TURN_R_MINI          0.35f      //��Сת��뾶( L*cot30-W/2)
#define  AKM_WHEEL_SCALE          (PI*AKM_WHEEL_DIAMETER*PID_RATE/AKM_WHEEL_RESOLUTION)  //�����ٶ�m/s�������ת��ϵ��

//����ȫ������˲��� 
#define  OMT_WHEEL_DIAMETER	      0.058	     //����ֱ��
#define  OMT_WHEEL_RADIUS         0.206	     //�����˰뾶����������������ľ���
#define  OMT_WHEEL_RESOLUTION     1560.0     //�������ֱ���(13��),���ٱ�30,13x30x4=1560
#define  OMT_WHEEL_SCALE          (PI*TWD_WHEEL_DIAMETER*PID_RATE/OMT_WHEEL_RESOLUTION)  //�����ٶ�m/s�������ת��ϵ��

/******������ͨ��Э��*************************************/

//����ͨ��֡ͷ����
#define  ID_UTX_DATA     0x10    //���͵��ۺ�����
#define  ID_URX_VEL      0x50    //���յ��ٶ�����

//�������ٶ�����
#define R_VX_LIMIT  1500   //X���ٶ���ֵ m/s*1000
#define R_VY_LIMIT  1200   //Y���ٶ���ֵ m/s*1000
#define R_VW_LIMIT  6280   //W��ת���ٶ���ֵ rad/s*1000


//�����˹ؼ�ȫ�ֱ���
extern  ROBOT_Velocity  R_Vel; //�������ٶ�����
extern  ROBOT_Wheel  R_Wheel_A,R_Wheel_B,R_Wheel_C,R_Wheel_D; //��������������

extern uint8_t ax_robot_type;
extern uint8_t ax_robot_move_enable;

extern int16_t ax_motor_kp;    
extern int16_t ax_motor_kd; 

//������������
extern int16_t ax_akm_offset;
extern int16_t ax_akm_angle;

//PS2�ֱ���ֵ�ṹ��
extern JOYSTICK_TypeDef my_joystick;  

#endif

/******************* (C) ��Ȩ 2023 XTARK **************************************/
