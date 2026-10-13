#include "app_irtracking.h"

#define IRTrack_Trun_KP (120)
#define IRTrack_Trun_KI (0) 
#define IRTrack_Trun_KD (0.5) 

int pid_output_IRR = 0;
u8 trun_flag = 0;


#define IRR_SPEED 			  250  //巡线速度	Line patrol speed

float APP_ELE_PID_Calc(int8_t actual_value)
{

	float IRTrackTurn = 0;
	int8_t error;
	static int8_t error_last=0;
	static float IRTrack_Integral;//积分	integral
	
	error=actual_value;
	
	IRTrack_Integral +=error;
	
	//位置式pid	Position pid
	IRTrackTurn=error*IRTrack_Trun_KP
							+IRTrack_Trun_KI*IRTrack_Integral
							+(error - error_last)*IRTrack_Trun_KD;
	return IRTrackTurn;
}

 
void IRI2C_WriteByte(uint8_t addr, uint8_t dat) {
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
 
uint8_t IRI2C_ReadByte(uint8_t addr) {
    uint8_t data;
 
    DL_I2C_fillControllerTXFIFO(Sensor_INST, &addr, 1);
	
	DL_I2C_disableInterrupt(Sensor_INST, DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER);
	
    while (!(DL_I2C_getControllerStatus(Sensor_INST) & DL_I2C_CONTROLLER_STATUS_IDLE));
    DL_I2C_startControllerTransfer(Sensor_INST, IR_I2C_ADDR, DL_I2C_CONTROLLER_DIRECTION_TX, 1);
    while (DL_I2C_getControllerStatus(Sensor_INST) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS);
    while (!(DL_I2C_getControllerStatus(Sensor_INST) & DL_I2C_CONTROLLER_STATUS_IDLE));
//    DL_I2C_flushControllerTXFIFO(Sensor_INST);
 
    DL_I2C_startControllerTransfer(Sensor_INST, IR_I2C_ADDR, DL_I2C_CONTROLLER_DIRECTION_RX, 1);
    while (DL_I2C_getControllerStatus(Sensor_INST) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS);
    while (!(DL_I2C_getControllerStatus(Sensor_INST) & DL_I2C_CONTROLLER_STATUS_IDLE));
    data = DL_I2C_receiveControllerData(Sensor_INST);
 
    return data;
}



void deal_IRdata(u8 *x1,u8 *x2,u8 *x3,u8 *x4,u8 *x5,u8 *x6,u8 *x7,u8 *x8)
{
	u8 IRbuf = 0xFF;
	IRbuf = IRI2C_ReadByte(0x30);
	
	*x1 = (IRbuf>>7)&0x01;
	*x2 = (IRbuf>>6)&0x01;
	*x3 = (IRbuf>>5)&0x01;
	*x4 = (IRbuf>>4)&0x01;
	*x5 = (IRbuf>>3)&0x01;
	*x6 = (IRbuf>>2)&0x01;
	*x7 = (IRbuf>>1)&0x01;
	*x8 = (IRbuf>>0)&0x01;
}


void printf_i2c_data(void)
{
    static uint8_t ir_x1,ir_x2,ir_x3,ir_x4,ir_x5,ir_x6,ir_x7,ir_x8;
    deal_IRdata(&ir_x1,&ir_x2,&ir_x3,&ir_x4,&ir_x5,&ir_x6,&ir_x7,&ir_x8);
    printf("x1:%d,x2:%d,x3:%d,x4:%d,x5:%d,x6:%d,x7:%d,x8:%d\r\n",ir_x1,ir_x2,ir_x3,ir_x4,ir_x5,ir_x6,ir_x7,ir_x8);
}

void LineWalking(void)
{
	static int8_t err = 0;
	static u8 x1,x2,x3,x4,x5,x6,x7,x8;
	
	deal_IRdata(&x1,&x2,&x3,&x4,&x5,&x6,&x7,&x8);
	
	//debug
//	static char bufbuf[30]={'\0'};
//	sprintf(bufbuf,"%d\t %d\t %d\t %d\t %d\t %d\t %d\t %d\t \r\n",x1,x2,x3,x4,x5,x6,x7,x8);
//	uart0_send_string((char*)bufbuf);
	
    //优先判断	Priority judgment
	if(x1 == 1 && x2 == 1 &&x3 == 0 &&  x4 == 0  && x5 == 0 && x6  == 0 && x7 == 1 && x8 == 1 ) //过锐角	Over sharp angle
	{
		err = 15; 
	}
	else if(x1 == 1 && x2 == 1 &&x3 == 1 &&  x4 == 1  && x5 == 1 && x6  == 1 && x7 == 1 && x8 == 1 ) //过锐角	Over sharp angle
	{
		if(trun_flag == 0) //出线了	Qualified
		{
			err = 0; 
			trun_flag = 1;
		}
		//其它情况保持上个状态	Stay in the previous state in other situations
	}
	
  else if(x1 == 0 &&  x2 == 0  && x7 == 0 && x8 == 0 ) //俩边都亮，直跑	Both sides are bright, run straight
	{
		err = 0;
		if(trun_flag == 1)
		{
			trun_flag = 0;//走到圈了	Got into the circle
		}
	}
	
	//优先判断	Priority judgment
 else if(x1 == 0 &&  x3 == 0 && x4 == 0 && x5 == 0 && x8 == 0 )
	{
		err = 0;
	}
	//添加直角	Add a right angle
	else if((x1 == 0 || x2 == 0 ) && x8 == 1) 
	{
		err = -15; 
	}
	//添加直角	Add a right angle
	else if((x7 == 0 ||  x8 == 0) && x1 == 1) 
	{
		err = 15 ;
	}
	


	else if(x1 == 1 && x2 == 1  && x3 == 1&& x4 == 0 && x5 == 1 && x6 == 1  && x7 == 1 && x8 == 1) // 1110 1111
	{
		err = -1;
	}
	else if(x1 == 1 && x2 == 1  && x3 == 0&& x4 == 0 && x5 == 1 && x6 == 1  && x7 == 1 && x8 == 1) // 1100 1111
	{
		err = -2;
	}
	else if(x1 == 1 && x2 == 1  && x3 == 0&& x4 == 1 && x5 == 1 && x6 == 1  && x7 == 1 && x8 == 1) // 1101 1111
	{
		err = -2;
	}
	
    
    
    
	else if(x1 == 1 && x2 == 0  && x3 == 0&& x4 == 1 && x5 == 1 && x6 == 1  && x7 == 1 && x8 == 1) // 1001 1111
	{
		err = -3;
	}
	
    

	else if(x1 == 1 && x2 == 1  && x3 == 1&& x4 == 1 && x5 == 0 && x6 == 1  && x7 == 1 && x8 == 1) // 1111 0111
	{
		err = 1;
	} 
	else if(x1 == 1 && x2 == 1  && x3 == 1&& x4 == 1 && x5 == 0 && x6 == 0  && x7 == 1 && x8 == 1) // 1111 0011
	{
		err = 2;
	}
	else if(x1 == 1 && x2 == 1  && x3 == 1&& x4 == 1 && x5 == 1 && x6 == 0  && x7 == 1 && x8 == 1) // 1111 1011
	{
		err = 2;
	}
	else if(x1 == 1 && x2 == 1  && x3 == 1&& x4 == 1 && x5 == 1 && x6 == 0  && x7 == 0 && x8 == 1) // 1111 1001
	{
		err = 3;
	}
	
 
	else if(x1 == 1 &&x2 == 1 &&x3 == 1 && x4 == 0 && x5 == 0 && x6 == 1 && x7 == 1&& x8 == 1) //直走	Go straight
	{
		err = 0;
	}
	
	
	
	//剩下的就保持上一个状态	The rest will remain in the previous state
	pid_output_IRR = (int)(APP_ELE_PID_Calc(err));

	Motion_Car_Control(IRR_SPEED, 0, pid_output_IRR);

}

//检测现在位于黑线还是在白线上	Detection is now on the black line or on the white line
int LineCheck(void)
{
	int if_have = 0;
    static u8 x1,x2,x3,x4,x5,x6,x7,x8;
	deal_IRdata(&x1,&x2,&x3,&x4,&x5,&x6,&x7,&x8);
	//Traversing the grayscale sensor
	if(!x1)
	{
		if_have = 1;
	}
    if(!x2)
	{
		if_have = 1;
	}
    if(!x3)
	{
		if_have = 1;
	}
	if(!x4)
	{
		if_have = 1;
	}
	if(!x5)
	{
		if_have = 1;
	}
	if(!x6)
	{
		if_have = 1;
	}
    if(!x7)
	{
		if_have = 1;
	}
    if(!x8)
	{
		if_have = 1;
	}
		
	//If a black line is detected
	if(if_have >= 1) 
	{
		  return BLACK;
	}
	else
	{
		  return WHITE;
	}
}




