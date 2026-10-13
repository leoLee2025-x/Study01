#include "app_mpu6050.h"
#include "Filter.h"
#define Pi 3.14159265
volatile short angle[3];
volatile short accel[3];
volatile float pitch=0,roll=0,yaw=0;   //?????Euler Angles
uint8_t Way_Angle=2; 
Bias_t Angle;

const int CALIB_SAMPLES = 30; // ??????  Number of samples
float Angle_Balance,Gyro_Balance,Gyro_Turn; //?????? ???????? ????????
float Acceleration_Z;
// ????????  Number of samples
volatile float yawBias = 0, pitchBias = 0, rollBias = 0;

volatile float calibratedYaw = 0, calibratedPitch = 0, calibratedRoll = 0;

float AngleOffsetCalc(void)
{
    for (int i = 0; i < CALIB_SAMPLES; i++)
    {
        Get_EulerAngles();
//        printf("yaw:%.2f, pitch:%.2f, roll:%.2f\r\n", yaw, pitch, roll);
        calibratedYaw += calibratedYaw;

       
//        delay_ms(20);
    }
//    printf("yawsum:%d\r\n", (int)yawSum);
    // ????????  Calculate offset
    calibratedYaw = calibratedYaw / CALIB_SAMPLES;

    
//    printf("yawBias:%.2f, pitchBias:%.2f, rollBias:%.2f", yawBias, pitchBias, rollBias);
}
//????????????  Get the calibrated angle
void Get_CalibratedAngles(void)
{
    if(yaw < -3)
    {
        calibratedYaw = -yaw;
    }
    else if(yaw >= 3)
    {
        calibratedYaw = 360 - yaw; //??????yaw?????????  ????????? yaw clockwise is negative
    }
    else
    {
        calibratedYaw = 0; 	//???????60???????? Filter out data near 0 and 360
			
    }
//    calibratedYaw = local_yaw - local_yawBias;
//    calibratedPitch = local_pitch - local_pitchBias;
//    calibratedRoll = local_roll - local_rollBias;
//    printf("hanshu: %d,%d,%d\r\n", (int)local_yawBias, (int)local_pitchBias, (int)local_rollBias);
	   	//printf("%3.1f \\r\\n", calibratedYaw);
}

void Get_EulerAngles(void)
{
    //????????Get Euler angles
    mpu_dmp_get_data(&pitch,&roll,&yaw);

    //delay_ms(20);//??????????????????????????According to the set sampling rate, the delay cannot be set too large
}

//?????ID??? Angle ring PID control
float dir_kp = 4,dir_ki=0.01,dir_kd = 0.5;
int Integral_Max = 300; //300
int pid_max = 3000; //3000
float Dir_PID(float error)
{ //5*20/1000*188
    float result = 0;
    static int16_t err_last = 0; //????????????0  Last error
    static float Integral = 0; // ????????? Initialize integral term

//   if(error == 0)          
//   {
//       Integral = 0;          //??????   Integral cleared
//   }
    Integral += error;           // ?????????????????Update the integral term and limit it
    if (Integral > Integral_Max) Integral = Integral_Max;               //?????? Integral limiting
    if (Integral < -Integral_Max) Integral = -Integral_Max;             //?????? Integral limiting

    // ?????PID
    result = dir_kp * error +dir_ki*Integral+ dir_kd * (error - err_last);
  
	 err_last = error;       // ?????????????????Update the integral term and limit it

    // ???????????utput limiting value
    if (result > Integral_Max) result = pid_max;  
    if (result < -Integral_Max) result = -pid_max;

    return -result;
}
//???????????0-360 ??????????????????????????? 0-360 ???
//Limit the heading_angle to 0-360 degrees(to prevent the range of heading_angle over 0-360 degrees beacuse of Addition or subtraction operations )
float navigetion_0_360_limit(float angle)
{
		float temp = 0;
		 if(angle > 360)
		{
			temp = angle - 360;
		}
		else
		{
			temp = angle;
		}
		return temp;
}

//??? 0-360 ???????????????????????????????????????????????????????
//Calculate the minor arc deviation in the 0-360 Navigation Coordinate System (Counterclockwise negative arc Angle is Positive, Clockwise is Negative)  
//??? 0-360 ???????????????????????????????????????????????????????
//Calculate the minor arc deviation in the 0-360 Navigation Coordinate System (Counterclockwise negative arc Angle is Positive, Clockwise is Negative)  
float get_minor_arc(float azimuth,float headingAngle)  
{
    float angle_err = 0.0; 
    if(azimuth >= 180 + headingAngle)  
    {
        angle_err = azimuth - headingAngle - 360;  
    }
    else if(headingAngle > 180 + azimuth)
    {
        angle_err = azimuth - headingAngle + 360;
    }
    else
    {
        angle_err =  azimuth - headingAngle;
    }
    return -angle_err;
}


 /**************************************************************************
Function: Get angle
Input   : way??he algorithm of getting angle 1??MP  2??alman  3??omplementary filtering
Output  : none
??????????????
????????ay???????????? 1??MP  2?????? 3????????
???  ?????
**************************************************************************/	
void Get_Angle(uint8_t way)
{ 
	float gyro_x,gyro_y,gyro_z,accel_z;
	float Accel_Angle_X,Accel_Angle_Y;
	if(way==1)                           //DMP??????	??????????????????????????
	{	
		if( mpu_dmp_get_data(&pitch,&roll,&yaw) == 0 )
        { 
          //  printf("\r\npitch =%d\r\n", (int)pitch);
           // printf("\r\nroll =%d\r\n", (int)roll);
            //printf("\\r\\nyaw =%3.2f\\r\\n", yaw);
        }      
        delay_ms(10);//??????????????????????????
	}			
	else
	{
		
		MPU6050ReadGyro(angle);
		MPU6050ReadAcc(accel);
		Accel_Angle_X=atan2(accel[0],accel[2])*180/Pi;     //?????????????????
		Accel_Angle_Y=atan2(accel[1],accel[2])*180/Pi;     //?????????????????
		accel_z=accel[2]*1.962/32768;
		gyro_z = angle[2] * 2000 / 32768; // ???????????
		delay_ms(10);
		if(Way_Angle==2)		  	
		{
			pitch= -Kalman_Filter_x(Accel_Angle_X,gyro_x);//????????
			roll = -Kalman_Filter_y(Accel_Angle_Y,gyro_y);
			 //printf("\\r\\nyaw =%3.2f\\r\\n", yaw);
		}
		else if(Way_Angle==3) 
		{  
			 pitch = -Complementary_Filter_x(Accel_Angle_X,gyro_x);//??????
			 roll = -Complementary_Filter_y(Accel_Angle_Y,gyro_y);
		}
		Angle_Balance=pitch;                              //?????????
		Gyro_Turn=angle[2];                                 //???????????
		Acceleration_Z=accel[2];                           //???Z????????
		
	}

} 


