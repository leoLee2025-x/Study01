# MSPM0-I2C 方式

[toc]

## 实验准备
1. MSPM0G3507主板
2. 8路巡线模块
3. 杜邦线若干

MSPM0G3507板子需要下载资料提供的I2C通信源码

## 实验目的
此实验的内容主要是利用MSPM0G3507主控通过I2C的方式接收8路巡线模块的数据。


## 实验接线
**MSPM0G3507接串口助手**
如果msp的type-c口没有下载程序的功能，需要用usb转ttl模块接到电脑，接线下表说述

|MSPM0G|usb转ttl|
|:------:|:--------:|
|PA10|TX|
|PA11|RX|
|vcc|vcc|
|GND|GND|
|**如果MSPM0G3507单片机type有下载功能，直接用type-c接入电脑的串口助手即可**||

|MSPM0G|8路巡线模块|
|:------:|:--------:|
|PA0|SDA|
|PA1|SCL|
|5v|5v|
|GND|GND|



## 实验步骤和现象
1. 把线连接好后，打开串口助手，就能看到红外模块的数值型数据了。波特率设置成115200。
如下图所示
![image-2024110100002](2024110100002.png)


**需要MSPM0的开发把环境搭建好,才能编译运行工程**
环境搭建教程:
https://wiki.lckfb.com/zh-hans/dmx/beginner/install.html

## 实验部分源码
```C
//主函数
int main(void)
{
    SYSCFG_DL_init();
	
	
		//等待红外模块正常 
		delay_ms(1000);
		delay_ms(1000);

	
	 //清除串口中断标志
    NVIC_ClearPendingIRQ(MYUART_INST_INT_IRQN);
    //使能串口中断
    NVIC_EnableIRQ(MYUART_INST_INT_IRQN);

		printf("start\r\n");
	
		
		IRI2C_WriteByte(0x01,1);//控制进入校准
		delay_ms(200);	
		IRI2C_WriteByte(0x01,0);//控制退出校准
		delay_ms(200);	
	
		printf("okok\r\n");
	
    while (1) 
    {    
			
		printf_i2c_data();
		delay_ms(500);	
				      
    }
} 

```
**上电前，先按住mspM0的复位脚，等待红外模块正常工作后，再松开MSP的复位按键，不然容易造成i2c死锁。**
源码的主函数就很简单，读取8路巡线的探头引脚，并打印出来。



