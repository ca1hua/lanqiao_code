#include "reg52.h"
#include "absacc.h"
#include "intrins.h"

sbit TX = P1^0;
sbit RX = P1^1;

unsigned int distance = 0;

unsigned char code SMG_duanma[18]= {
	0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
	0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f
};

void DelaySMG(unsigned int t) {
	while(t--);
}

void DisplaySMG_Bit(unsigned char pos, unsigned char value) {
	XBYTE[0xE000] = 0xFF;
	XBYTE[0xC000] = 0x01 << pos;
	XBYTE[0xE000] = value;
}


void Display_Distance() {
	if(distance == 999) {
		DisplaySMG_Bit(0, SMG_duanma[15]);			//??????????¦¶?????F
		DelaySMG(500);
	} else {
		DisplaySMG_Bit(5, SMG_duanma[distance / 100]);
		DelaySMG(500);
		DisplaySMG_Bit(6, SMG_duanma[(distance % 100) / 10]);
		DelaySMG(500);
		DisplaySMG_Bit(7, SMG_duanma[distance % 10]);
		DelaySMG(500);
	}
}

void Delay12us() {
	unsigned char i;

	_nop_();
	_nop_();
	i = 33;
	while (--i);
}

void Send_Wave() {               //????8??40KHx?????????
	unsigned char i;
	for(i = 0; i < 8; i++) {
		TX = 1;
		Delay12us();
		TX = 0;
		Delay12us();
	}
}

void Measure_Distance() {
	unsigned int time = 0;

	TMOD &= 0x0f;
	TL1= 0x00;
	TH1 = 0x00;

	Send_Wave();

	TR1 = 1;            //?????????
	while((RX == 1) && (TF1 == 0)){
	if(distance==999)
	Display_Distance();
	}  //??????????????????????????????¦¶
	TR1 = 0;            //???????

	if(TF1 == 0) {            //??????????¦¶
		time = TH1;
		time = (time << 8) | TL1;
		distance = ((time / 10) * 17) / 100 + 3;
	} else {                    //??????????¦¶
		TF1 = 0;
		distance = 999;
	}
}

void Delay(unsigned char n) {      //???????????
	while(n--) {
		Display_Distance();
	}
}

void main() {
	while(1) {
		Measure_Distance();
		Delay(10);
	}
}
