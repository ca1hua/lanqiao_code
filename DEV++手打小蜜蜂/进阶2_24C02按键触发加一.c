#include "reg52.h"
#include "iic.h"

sbit S4 = P3^3;
sbit S5 = P3^2;
sbit S6 = P3^1;

unsigned char code SMG_NoDot[18]={0xc0,0xf9,
    0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
    0x88,0x83,0xc6,0xc0,0x86,0x8e,0xbf,0x7f};

unsigned char dat1 = 0 , dat2 = 0 , dat3 = 0;

void Set_HC573(unsigned char channel,unsigned char dat) {
	P0 = dat;
	switch(channel) {
		case 4:
			P2 = (P2 & 0x1f) | 0x80;
			break;
		case 5:
			P2 = (P2 & 0x1f) | 0xa0;
			break;
		case 6:
			P2 = (P2 & 0x1f) | 0xc0;
			break;
		case 7:
			P2 = (P2 & 0x1f) | 0xe0;
			break;
		case 0:
			P2 = (P2 & 0x1f) | 0x00;
			break;
	}
	P2 = (P2 & 0x1f) | 0x00;
}

void DisplaySMG_All(unsigned char value)
{
  Set_HC573(6, 0xff);    //使能所有数码管
  Set_HC573(7, value);  //数码管显示内容  
}

void DelaySMG(unsigned int t) {
	while(t--);
}

void DisplaySMG_Bit(unsigned char pos,unsigned char value) {
	Set_HC573(6, 0x01 << pos);    //数码管的段位
	Set_HC573(7, value);          //数码管显示内容
	DelaySMG(500);
	Set_HC573(6, 0x01 << pos);
	Set_HC573(7, 0xff);  //xiaoyin
}

void DisplaySMG_24C02() {
	//数码管右起第0位
	DisplaySMG_Bit(7,SMG_NoDot[dat3  % 10]);
	//数码管右起第1位
	DisplaySMG_Bit(6,SMG_NoDot[dat3 / 10]);
	//数码管右起第2位
	DisplaySMG_Bit(5,SMG_NoDot[16]);

	//数码管右起第3位
	DisplaySMG_Bit(4,SMG_NoDot[dat2  % 10]);
	//数码管右起第4位
	DisplaySMG_Bit(3,SMG_NoDot[dat2 / 10]);
	//数码管右起第5位
	DisplaySMG_Bit(2,SMG_NoDot[16]);

	//数码管右起第6位
	DisplaySMG_Bit(1,SMG_NoDot[dat1 % 10]);
	//数码管右起第7位
	DisplaySMG_Bit(0,SMG_NoDot[dat1 / 10]);
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
} //写 八 步

unsigned char Read_24C02(unsigned char addr) {

//AT24C02是一个2K位串行CMOS E2PROM
	unsigned char tmp;
	//首先，进行一个伪写操作
	IIC_Start();          //起始信号
	IIC_SendByte(0xa0);    //EEPROM的写设备地址
	IIC_WaitAck();        //等待从机应答
	IIC_SendByte(addr);    //内存单元地址
	IIC_WaitAck();        //等待从机应答
	//然后，开始字节读操作
	IIC_Start();          //起始信号
	IIC_SendByte(0xa1);    //EEPROM的读设备地址
	IIC_WaitAck();        //等待从机应答
	tmp = IIC_RecByte();  //读取内存中的数据
	IIC_SendAck(1);        //产生非应答信号
	IIC_Stop();            //停止信号
	return tmp;
}

//去抖动
void Scan_Keys() {
	if(S4 == 0) {
		DelaySMG(100);
		if(S4 == 0) {
			dat1++;
			if(dat1 > 13) {
				dat1 = 0;
			}
			Write_24C02(0x00,dat1);
			while(S4 == 0) {
				DisplaySMG_24C02();
			}
		}
	}
	//按键S5的扫描处理
	if(S5 == 0) {
		DelaySMG(100);
		if(S5 == 0) {
			dat2++;
			if(dat2 > 13) {
				dat2 = 0;
			}
			Write_24C02(0x01, dat2);
			while(S5 == 0) {
				DisplaySMG_24C02();
			}
		}
	}

	//按键S6的扫描处理
	if(S6 == 0) {
		DelaySMG(100);
		if(S6 == 0) {
			dat3++;
			if(dat3 > 13) {
				dat3 = 0;
			}
			Write_24C02(0x02, dat3);
			while(S6 == 0) {
				DisplaySMG_24C02();
			} //按下按键的时候 我也要一直扫描这个数码管
		}
	}

}

void Init_sys() {

	Set_HC573(0, 0x00);        //关闭所有锁存器
	Set_HC573(5, 0x00);        //关闭蜂鸣器和继电器
	Set_HC573(4, 0xff);        //关闭全部LED灯
	DisplaySMG_All(0xff);      //关闭全部数码管

	dat1 = Read_24C02(0x00);
	dat2 = Read_24C02(0x01);
	dat3 = Read_24C02(0x02);
}

void main() {
	Init_sys();
	while(1) {
		Scan_Keys();
		DisplaySMG_24C02();
	}
}
