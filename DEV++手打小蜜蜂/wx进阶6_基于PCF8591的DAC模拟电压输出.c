#include "reg52.h"
#include "iic.h"
sbit S4 = P3^3;

#define TSMG 500
unsigned char code SMG_NoDot[18]= {
	0xc0,0xf9,
	0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
	0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f
};

unsigned char code SMG_Dot[10]= {
	0x40,0x79,
	0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10
};

unsigned char adc_value = 0;
float adc_volt = 0;
unsigned int smg_volt = 0;
unsigned char stat_smg = 1;
unsigned char mode = 1;

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
	//输出电压
	DisplaySMG_Bit(7,SMG_NoDot[smg_volt  % 10]);
	DisplaySMG_Bit(6,SMG_NoDot[(smg_volt / 10) % 10]);
	DisplaySMG_Bit(5,SMG_Dot[smg_volt / 100]);
	//工作模式
	DisplaySMG_Bit(2,SMG_NoDot[16]);
	DisplaySMG_Bit(1,SMG_NoDot[mode]);
	DisplaySMG_Bit(0,SMG_NoDot[16]);
}


//===============PCF8591电压采样处理函数===============
void Read_PCF8591_AIN3() {
	IIC_Start();
	IIC_SendByte(0x90);    //PCF8591的写设备地址
	IIC_WaitAck();
	IIC_SendByte(0x43);   //输出DAC，转换AIN3
	IIC_WaitAck();
	IIC_Stop();

	DisplaySMG_ADC();      //等待电压转换完成

	IIC_Start();
	IIC_SendByte(0x91);   //PCF8591的读设备地址
	IIC_WaitAck();
	adc_value = IIC_RecByte();  //读出AD采样数据
	IIC_SendAck(1);         //产生非应答信号
	IIC_Stop();
	//将ADC采样到的数据换算成对应的电压值
	adc_volt = adc_value * (5.0 / 255);
	smg_volt = adc_volt * 100;
}

void Set_PCF8591_DAC(unsigned char dat) {
	IIC_Start();
	IIC_SendByte(0x90);    //PCF8591的写设备地址
	IIC_WaitAck();
	IIC_SendByte(0x43);   //输出DAC，转换AIN3
	IIC_WaitAck();
	IIC_SendByte(dat);     //设置DAC电压输出参数
	IIC_WaitAck();          //产生非应答信号
	IIC_Stop();
}

void Scan_Keys() {
	if(S4 == 0) {
		DelaySMG(500);
		if(S4 == 0) {
			if(mode == 1) {
				mode = 2;              //切换为模式2
				Set_PCF8591_DAC(204);  //输出固定电压4V
				smg_volt = 400;        //显示电压：4.00
			} else if(mode == 2) {
				mode = 3;
				Read_PCF8591_AIN3();        //采样AIN3电压
				Set_PCF8591_DAC(adc_value);            //切换为模式3
			} else if(mode == 3) {
				mode = 1;              //切换为模式1
				Set_PCF8591_DAC(102);  //输出固定电压2V
				smg_volt = 200;        //显示电压：2.00
			}
			while(S4 == 0) {        //松手检测
				DisplaySMG_ADC();      //保持数码管动态显示
			}
		}
	}
}

void Init_sys() {
	Set_HC573(0, 0x00);        //关闭所有锁存器
	Set_HC573(5, 0x00);        //关闭蜂鸣器和继电器
	Set_HC573(4, 0xff);        //关闭全部LED灯
	DisplaySMG_All(0xff);      //关闭全部数码管
	Set_PCF8591_DAC(102);      //输出固定电压2V
	smg_volt = 200;            //数码管显示2.00V
}

void main() {
	Init_sys();                //系统初始化
	while(1) {
		Scan_Keys();          //循环扫描按键
		DisplaySMG_ADC();      //动态刷新数码管
	}
}
