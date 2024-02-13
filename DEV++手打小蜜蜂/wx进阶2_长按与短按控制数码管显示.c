#include "reg52.h"

sbit S4 = P3^3;

//���Ƿ��� ��ʵ���� �� �̰� ��ʵ���� ���ж������е�

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
	Set_HC573(6, 0x01 << pos);    //����ܵĶ�λ
	Set_HC573(7, value);          //�������ʾ����
	DelaySMG(500);                //����ܵ���ʱ��
	Set_HC573(6, 0x01 << pos);
	Set_HC573(7, 0xff);            //����
}

void DisplaySMG_All(unsigned char value) {
	Set_HC573(6,0xff);
	Set_HC573(7,value);
}

void Display_Num() {
	//0λ
	DisplaySMG_Bit(7,SMG_NoDot[num  % 10]);
	//1λ
	DisplaySMG_Bit(6,SMG_NoDot[num / 10]);
}

//========================================

void Init_Timer0() {
	TMOD = 0x01;
	TH0 = (65536 - 10000) / 256;
	TL0 = (65536 - 10000) % 256;
	ET0 = 1;
	EA = 1;
	TR0 = 1; //���˾���
}

void Service_Timer0() interrupt 1 {
	TH0 = (65536 - 10000) / 256;
	TL0 = (65536 - 10000) % 256;
	if(F_key == 1) {      //�ڰ��������ڼ���м���
		count_t++;          //�������µĳ���ʱ��
	}
}

void Scan_Keys() {
	if(S4 == 0) {
		count_t = 0;      //ʱ�����������0
		F_key = 1;        //��־��������״̬
		while(S4 == 0) {  //�ȴ������ɿ�
			Display_Num();  //�ڰ����ڼ䱣�������������ʾ
		}
		F_key = 0;        //��־�����ɿ�״̬

		if(count_t > 100) { //����ʱ�����1�룬����
			num = 0;        //����ܼ������Ϊ00
		} else {          //����ʱ��С��1�룬�̰�
			num++;          //����ܼ�����1
			if(num == 100) {
				num = 0;      //�����������ֵ99��ָ�00
			}
		}
	}
}


void Init_sys() {
	Set_HC573(0, 0x00);        //�ر�����������
	Set_HC573(5, 0x00);        //�رշ������ͼ̵���
	Set_HC573(4, 0xff);        //�ر�ȫ��LED��
	DisplaySMG_All(0xff);      //�ر�ȫ�������
	Init_Timer0();            //��ʼ����ʱ��0������
}

void main() {
	Init_sys();
	while(1) {
		Scan_Keys();            //ѭ��ɨ�谴��
		Display_Num();          //�����ˢ��
	}
}
