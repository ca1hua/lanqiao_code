#include "reg52.h"
#include "iic.h"

#define TSMG  500
//-------共阳数码管的段码编码表（无小数点）--------
//0 1 2 3 4 5 6 7 8 9 A B C D E F - .
unsigned char code SMG_NoDot[18]= {
	0xc0,0xf9,
	0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
	0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f
};
//-------共阳数码管的段码编码表（带小数点）--------
//0. 1. 2. 3. 4. 5. 6. 7. 8. 9.
unsigned char code SMG_Dot[10]= {
	0x40,0x79,
	0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10
};
//-----------------------------------------------

unsigned char adc1_value = 0;
float adc1_volt = 0;
unsigned int smg1_volt = 0;
unsigned char adc3_value = 0;
float adc3_volt = 0;
unsigned int smg3_volt = 0;

//==============数码管动态显示专用延时函数=============

void DelaySMG(unsigned int t) {
	while(t--);
}

//==================设置选通的锁存器===================
void Set_HC573(unsigned char channel, unsigned char dat) {
	P0 = dat;                      //待设置数据
	switch(channel) {              //选通锁存器
		case 4:
			P2 = (P2 & 0x1f) | 0x80;  //Y4输出0，LED控制
			break;
		case 5:
			P2 = (P2 & 0x1f) | 0xa0;  //Y5输出0，蜂鸣器和继电器控制
			break;
		case 6:
			P2 = (P2 & 0x1f) | 0xc0;  //Y6输出0，数码管位选
			break;
		case 7:
			P2 = (P2 & 0x1f) | 0xe0;  //Y7输出0，数码管段码
			break;
		case 0:
			P2 = (P2 & 0x1f) | 0x00;  //所有锁存器不选择
			break;
	}
	P2 = (P2 & 0x1f) | 0x00;      //设置完成，关闭全部锁存器
}
//=================单个数码管动态显示=================
void DisplaySMG_Bit(unsigned char pos, unsigned char value) {
	Set_HC573(6, 0x01 << pos);    //数码管的段位
	Set_HC573(7, value);          //数码管显示内容
	DelaySMG(TSMG);                //数码管点亮时长
	Set_HC573(6, 0x01 << pos);
	Set_HC573(7, 0xff);            //消隐
}
//=================全部数码管静态显示=================
void DisplaySMG_All(unsigned char value) {
	Set_HC573(6, 0xff);    //使能所有数码管
	Set_HC573(7, value);  //数码管显示内容
}
//===================电压值显示函数===================
void DisplaySMG_ADC() {
	//数码管右起第0位
	DisplaySMG_Bit(7,SMG_NoDot[smg1_volt  % 10]);
	//数码管右起第1位
	DisplaySMG_Bit(6,SMG_NoDot[(smg1_volt / 10) % 10]);
	//数码管右起第2位
	DisplaySMG_Bit(5,SMG_Dot[smg1_volt / 100]);

	//数码管右起第5位
	DisplaySMG_Bit(2,SMG_NoDot[smg3_volt  % 10]);
	//数码管右起第6位
	DisplaySMG_Bit(1,SMG_NoDot[(smg3_volt / 10) % 10]);
	//数码管右起第7位
	DisplaySMG_Bit(0,SMG_Dot[smg3_volt / 100]);
}

unsigned char Read_PCF8591_ADC(unsigned char ain) {
	unsigned char tmp;
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	if(ain == 1) {
		IIC_SendByte(0x01);
	} else if(ain == 3) {
		IIC_SendByte(0x03);
	}
	IIC_WaitAck();
	IIC_Stop();

	DisplaySMG_ADC(); //等待电压转换完成

	IIC_Start();
	IIC_SendByte(0x91);   //PCF8591的读设备地址
	IIC_WaitAck();
	tmp = IIC_RecByte();   //读出AD采样数据
	IIC_SendAck(1);         //产生非应答信号
	IIC_Stop();
	return tmp;
}

void Read_AIN1_AIN3() {
	//AD转换 
	adc1_value = Read_PCF8591_ADC(1);
	adc1_volt = adc1_value * (5.0/255);
	smg1_volt = adc1_volt * 100;

	adc3_value = Read_PCF8591_ADC(3);
	adc3_volt = adc3_value * (5.0/255);
	smg3_volt = adc3_volt * 100;
}

//=====================系统初始化=====================
void Init_sys() {
	Set_HC573(0, 0x00);        //关闭所有锁存器
	Set_HC573(5, 0x00);        //关闭蜂鸣器和继电器
	Set_HC573(4, 0xff);        //关闭全部LED灯
	DisplaySMG_All(0xff);      //关闭全部数码管
}
//=======================主函数=======================
void main() {
	Init_sys();               //系统初始化
	while(1) {
		Read_AIN1_AIN3();      //循环采样电压
		DisplaySMG_ADC();      //动态刷新数码管
	}
}
