# MSPM0-UASRT 方式

[toc]

## 实验准备
1. MSPM0G3507主板
2. 8路巡线模块
3. 杜邦线若干

MSPM0G3507板子需要下载资料提供的UASRT通信源码

## 实验目的
此实验的内容主要是利用MSPM0G3507主控通过UASRT的方式接收8路巡线模块的数据。


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
|PA9|TX|
|PA8|RX|
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

int main(void)
{
    //开发板初始化
    board_init();
	
		delay_ms(500);
		delay_ms(500);
		uart1_send_string("$0,0,1#");
        
    IR_usart_config();   // 串口初始化
    printf("start\r\n");
    while(1) 
    {
			//解析发送过来的数据
			IRDataAnalysis();
			delay_ms(500);
			

                
    }
}
```
**uart1_send_string("$0,0,1#");**：第一个数是校准模式(0:退出校准模式 1:进入校准模式) 第二个数是是否接收模拟型数据 第三个数是是否接收数值型数据。
此例程只提供解析数值型的数据，需要解析模拟型数据，可以根据协议自行解析，本工程的串口解析文件也有解析模拟值的函数，可以自行参考**Deal_Usart_AData**函数。



