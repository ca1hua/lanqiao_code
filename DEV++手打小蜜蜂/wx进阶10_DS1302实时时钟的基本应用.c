#include "reg52.h"
#include "ds1302.h"
sbit S4 = P3^3;
sbit S5 = P3^2;
sbit S6 = P3^1;
#define TSMG  500

unsigned char code SMG_NoDot[18]= {
	0xc0,0xf9,
	0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
	0x88,0x83,0xc6,0xc0,0x86,0x8e,0xbf,0x7f
};
//-----------------------------------------------
unsigned char code READ_RTC_ADDR[7] = {0x81, 0x83, 0x85, 0x87, 0x89, 0x8b, 0x8d};
unsigned char code WRITE_RTC_ADDR[7] = {0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c};
unsigned char TIME[7] = {0x24, 0x58, 0x23, 0x19, 0x04, 0x06, 0x20};

unsigned char F_pause = 0;    //暂停/启动的标志
unsigned char hour = 0;        //时
unsigned char min = 0;        //分
unsigned char sec = 0;        //秒

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
	Set_HC573(6, 0xff);            //使能所有数码管
	Set_HC573(7, value);          //数码管显示内容
}
//==================实时时间显示函数==================
void DisplaySMG_RTC() {
	//秒
	DisplaySMG_Bit(7,SMG_NoDot[sec & 0x0f]);
	DisplaySMG_Bit(6,SMG_NoDot[(sec >> 4) & 0x07]);
	DisplaySMG_Bit(5,SMG_NoDot[16]);
	//分
	DisplaySMG_Bit(4,SMG_NoDot[min & 0x0f]);
	DisplaySMG_Bit(3,SMG_NoDot[min >> 4]);
	DisplaySMG_Bit(2,SMG_NoDot[16]);
	//时
	DisplaySMG_Bit(1,SMG_NoDot[hour & 0x0f]);
	DisplaySMG_Bit(0,SMG_NoDot[hour >> 4]);
}

void Init_DS1302_Param() {
	unsigned char i;
	Write_Ds1302_Byte(0x8E,0x00);  //解除写保护
	for(i = 0; i < 7; i++) {      //依次写入7个参数
		Write_Ds1302_Byte(WRITE_RTC_ADDR[i],TIME[i]);
	}
	Write_Ds1302_Byte(0x8E,0x80);  //使能写保护
}

void Set_DS1302_Time() {
	Write_Ds1302_Byte(0x8E,0x00);    //解除写保护
	Write_Ds1302_Byte(0x80,sec);    //设置：秒
	Write_Ds1302_Byte(0x82,min);    //设置：分
	Write_Ds1302_Byte(0x84,hour);    //设置：时
	Write_Ds1302_Byte(0x8E,0x80);    //使能写保护
}

void Read_DS1302_Time() {
	if(F_pause == 0) {            //在非暂停模式中
		sec = Read_Ds1302_Byte(0x81);
		min = Read_Ds1302_Byte(0x83);
		hour = Read_Ds1302_Byte(0x85);
	}
	DisplaySMG_RTC();              //显示最新的时分秒
}

void Scan_Keys() {
//S4按键：暂停与启动控制
	if(S4 == 0) {
		DelaySMG(500);                  //去抖动
		if(S4 == 0) {                    //按下S4按键
			if(F_pause == 0) {
				F_pause = 1;                //标志暂停模式
			} else {
				F_pause = 0;                //标志启动模式
				Set_DS1302_Time();          //设置最新时分秒参数
			}

			while(S4 == 0) {              //松手检测
				Read_DS1302_Time();
			}
		}
	}
	//S5按键：分钟递减
	if(S5 == 0) {
		DelaySMG(500);                  //去抖动
		if(S5 == 0) {
			if(F_pause == 1) {            //在暂停计时的情况下
				min = (min/16)*10 + min%16;  //BCD码转十进制
				if(min == 0) {
					min = 59;                  //分钟递减的边界处理
				} else {
					min = min - 1;            //分钟减1
				}
				min = (min/10)*16 + min%10;  //十进制转BCD码
			}
			while(S5 == 0) {              //松手检测
				Read_DS1302_Time();
			}
		}
	}
	//S6按键：分钟递增
	if(S6 == 0) {
		DelaySMG(500);                  //去抖动
		if(S6 == 0) {
			if(F_pause == 1) {            //在暂停计时的情况下
				min = (min/16)*10 + min%16;  //BCD码转十进制
				if(min == 59) {
					min = 0;                  //分钟递增的边界处理
				} else {
					min++;                    //分钟加1
				}
				min = (min/10)*16 + min%10;  //十进制转BCD码
			}
			while(S6 == 0) {              //松手检测
				Read_DS1302_Time();
			}
		}
	}
}

void Init_sys() {
	Set_HC573(0, 0x00);        //关闭所有锁存器
	Set_HC573(5, 0x00);        //关闭蜂鸣器和继电器
	Set_HC573(4, 0xff);        //关闭全部LED灯
	DisplaySMG_All(0xff);      //关闭全部数码管
	Init_DS1302_Param();      //初始化DS1302
}

void main() {
	Init_sys();                //系统初始化
	while(1) {
		Read_DS1302_Time();      //更新显示DS1302时分秒
		Scan_Keys();            //循环扫描按键并处理
	}
}

//代码写的真漂亮啊 
