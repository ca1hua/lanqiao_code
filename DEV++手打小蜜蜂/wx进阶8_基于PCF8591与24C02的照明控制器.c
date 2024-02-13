#include "reg52.h"
#include "iic.h"

#define TSMG  500

sbit S4 = P3^3;
sbit S5 = P3^2;

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
unsigned char stat_led = 0xff;
unsigned char level = 0;

void DelaySMG(unsigned int t) {
	while(t--);
}

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

void DisplaySMG_Bit(unsigned char pos, unsigned char value) {
	Set_HC573(6, 0x01 << pos);    //数码管的段位
	Set_HC573(7, value);          //数码管显示内容
	DelaySMG(TSMG);                //数码管点亮时长
	Set_HC573(6, 0x01 << pos);
	Set_HC573(7, 0xff);            //消隐
}

void DisplaySMG_All(unsigned char value) {
	Set_HC573(6, 0xff);    //使能所有数码管
	Set_HC573(7, value);  //数码管显示内容
}

void DisplaySMG_Info() {

	DisplaySMG_Bit(7,SMG_NoDot[smg_volt  % 10]);
	DisplaySMG_Bit(6,SMG_NoDot[(smg_volt / 10) % 10]);
	DisplaySMG_Bit(5,SMG_Dot[smg_volt / 100]);

	DisplaySMG_Bit(2,SMG_NoDot[16]);
	DisplaySMG_Bit(1,SMG_NoDot[level]);
	DisplaySMG_Bit(0,SMG_NoDot[16]);
}

void Write_24C02(unsigned char addr,unsigned char dat) {

	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();

	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();
}

unsigned char Read_24C02(unsigned char addr) {
	unsigned char tmp;
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();

	IIC_Start();          //起始信号
	IIC_SendByte(0xa1);    //EEPROM的读设备地址
	IIC_WaitAck();        //等待从机应答
	tmp = IIC_RecByte();  //读取内存中的数据
	IIC_SendAck(1);        //产生非应答信号
	IIC_Stop();            //停止信号
	return tmp;
}

void Read_PCF8591_AIN1() {
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(0x01);
	IIC_WaitAck();
	IIC_Stop();

	DisplaySMG_Info();    //等待电压转换完成

	IIC_Start();
	IIC_SendByte(0x91);
	IIC_WaitAck();
	adc_value = IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();

	adc_volt = adc_value * (5.0 / 255);
	smg_volt = adc_volt * 100;
	DisplaySMG_Info();
}

void LED_Auto_Control() {
	Read_PCF8591_AIN1();
	if(adc_volt > 4.5) {
		stat_led = 0xff;
		level = 0;
	} else if(adc_volt > 4.0) {
		stat_led = 0xfc;
		level = 1;
	} else if(adc_volt > 3.5) {
		stat_led = 0xfc;
		level = 2;
	}  else if(adc_volt > 3.0) {
		stat_led = 0xf8;        //等级3，L1至L3点亮
		level = 3;
	} else if(adc_volt > 2.5) {
		stat_led = 0xf0;        //等级4，L1至L4点亮
		level = 4;
	} else if(adc_volt > 2.0) {
		stat_led = 0xe0;        //等级5，L1至L5点亮
		level = 5;
	} else if(adc_volt > 1.5) {
		stat_led = 0xc0;        //等级6，L1至L6点亮
		level = 6;
	} else if(adc_volt > 1.0) {
		stat_led = 0x80;        //等级7，L1至L7点亮
		level = 7;
	} else {
		stat_led = 0x00;        //等级8，L1至L8点亮
		level = 8;
	}
	Set_HC573(4, stat_led);    //更新灯光控制数据
}

//================-=========================
void Save_to_24C02() {
	Write_24C02(0x01, level);      //保存照明控制等级
	DisplaySMG_Info();
	Write_24C02(0x02, adc_value);  //保存光照采样数据
	DisplaySMG_Info();
}

void Read_from_24C02() {
	level = Read_24C02(0x01);
	adc_value = Read_24C02(0x02);
	adc_volt = adc_value * (5.0 / 255);
	smg_volt = adc_volt * 100;
}

void Scan_Keys() {
	if(S4 == 0) {
		DelaySMG(100);
		if(S4 == 0) {
			Save_to_24C02();
			while(S4 == 0) {
				LED_Auto_Control();
			}
		}
	}

	if(S5 == 0) {
		DelaySMG(100);          //去抖动
		if(S5 == 0) {
			Set_HC573(4, 0xff);    //关闭全部灯光
			Read_from_24C02();    //读取历史数据
			while(S5 == 0) {      //松手检测
				DisplaySMG_Info();  //显示历史数据
			}
		}
	}
}

void Init_sys() {
	Set_HC573(0, 0x00);
	Set_HC573(5, 0x00);
	Set_HC573(4, 0xff);
	DisplaySMG_All(0xff);
}

void main()
{
  Init_sys();                //系统初始化        
  while(1)
  {
    Scan_Keys();            //扫描按键
    LED_Auto_Control();      //照明自动控制
  }
}

