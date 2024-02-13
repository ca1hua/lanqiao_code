#include "reg52.h"

sbit S4 = P3^3;

//我们发现 其实长按 和 短按 其实就是 用中断来进行的

unsigned char code SMG_NoDot[18]= {
	0xc0,0xf9,
	0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
	0x88,0x83,0xc6,0xc0,0x86,0x8e,0xbf,0x7f
};

unsigned char num = 28;
bit F_key = 0;
unsigned int count_t = 0;


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


void DelaySMG(unsigned int t) {
	while(t--);
}

void DisplaySMG_Bit(unsigned char pos, unsigned char value) {
	Set_HC573(6, 0x01 << pos);    //数码管的段位
	Set_HC573(7, value);          //数码管显示内容
	DelaySMG(500);                //数码管点亮时长
	Set_HC573(6, 0x01 << pos);
	Set_HC573(7, 0xff);            //消隐
}

void DisplaySMG_All(unsigned char value) {
	Set_HC573(6,0xff);
	Set_HC573(7,value);
}

void Display_Num() {
	//0位
	DisplaySMG_Bit(7,SMG_NoDot[num  % 10]);
	//1位
	DisplaySMG_Bit(6,SMG_NoDot[num / 10]);
}

//========================================

void Init_Timer0() {
	TMOD = 0x01;
	TH0 = (65536 - 10000) / 256;
	TL0 = (65536 - 10000) % 256;
	ET0 = 1;
	EA = 1;
	TR0 = 1; //打开了就是
}

void Service_Timer0() interrupt 1 {
	TH0 = (65536 - 10000) / 256;
	TL0 = (65536 - 10000) % 256;
	if(F_key == 1) {      //在按键按下期间进行计数
		count_t++;          //按键按下的持续时间
	}
}

void Scan_Keys() {
	if(S4 == 0) {
		count_t = 0;      //时间计数变量清0
		F_key = 1;        //标志按键按下状态
		while(S4 == 0) {  //等待按键松开
			Display_Num();  //在按下期间保持数码管正常显示
		}
		F_key = 0;        //标志按键松开状态

		if(count_t > 100) { //按下时间大于1秒，长按
			num = 0;        //数码管计数清除为00
		} else {          //按下时间小于1秒，短按
			num++;          //数码管计数加1
			if(num == 100) {
				num = 0;      //计数超出最大值99后恢复00
			}
		}
	}
}


void Init_sys() {
	Set_HC573(0, 0x00);        //关闭所有锁存器
	Set_HC573(5, 0x00);        //关闭蜂鸣器和继电器
	Set_HC573(4, 0xff);        //关闭全部LED灯
	DisplaySMG_All(0xff);      //关闭全部数码管
	Init_Timer0();            //初始化定时器0并启动
}

void main() {
	Init_sys();
	while(1) {
		Scan_Keys();            //循环扫描按键
		Display_Num();          //数码管刷新
	}
}
