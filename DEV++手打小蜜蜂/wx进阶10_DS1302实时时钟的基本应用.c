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

unsigned char F_pause = 0;    //��ͣ/�����ı�־
unsigned char hour = 0;        //ʱ
unsigned char min = 0;        //��
unsigned char sec = 0;        //��

void DelaySMG(unsigned int t) {
	while(t--);
}
//==================����ѡͨ��������===================
void Set_HC573(unsigned char channel, unsigned char dat) {
	P0 = dat;                      //����������
	switch(channel) {              //ѡͨ������
		case 4:
			P2 = (P2 & 0x1f) | 0x80;  //Y4���0��LED����
			break;
		case 5:
			P2 = (P2 & 0x1f) | 0xa0;  //Y5���0���������ͼ̵�������
			break;
		case 6:
			P2 = (P2 & 0x1f) | 0xc0;  //Y6���0�������λѡ
			break;
		case 7:
			P2 = (P2 & 0x1f) | 0xe0;  //Y7���0������ܶ���
			break;
		case 0:
			P2 = (P2 & 0x1f) | 0x00;  //������������ѡ��
			break;
	}
	P2 = (P2 & 0x1f) | 0x00;      //������ɣ��ر�ȫ��������
}
//=================��������ܶ�̬��ʾ=================
void DisplaySMG_Bit(unsigned char pos, unsigned char value) {
	Set_HC573(6, 0x01 << pos);    //����ܵĶ�λ
	Set_HC573(7, value);          //�������ʾ����
	DelaySMG(TSMG);                //����ܵ���ʱ��
	Set_HC573(6, 0x01 << pos);
	Set_HC573(7, 0xff);            //����
}
//=================ȫ������ܾ�̬��ʾ=================
void DisplaySMG_All(unsigned char value) {
	Set_HC573(6, 0xff);            //ʹ�����������
	Set_HC573(7, value);          //�������ʾ����
}
//==================ʵʱʱ����ʾ����==================
void DisplaySMG_RTC() {
	//��
	DisplaySMG_Bit(7,SMG_NoDot[sec & 0x0f]);
	DisplaySMG_Bit(6,SMG_NoDot[(sec >> 4) & 0x07]);
	DisplaySMG_Bit(5,SMG_NoDot[16]);
	//��
	DisplaySMG_Bit(4,SMG_NoDot[min & 0x0f]);
	DisplaySMG_Bit(3,SMG_NoDot[min >> 4]);
	DisplaySMG_Bit(2,SMG_NoDot[16]);
	//ʱ
	DisplaySMG_Bit(1,SMG_NoDot[hour & 0x0f]);
	DisplaySMG_Bit(0,SMG_NoDot[hour >> 4]);
}

void Init_DS1302_Param() {
	unsigned char i;
	Write_Ds1302_Byte(0x8E,0x00);  //���д����
	for(i = 0; i < 7; i++) {      //����д��7������
		Write_Ds1302_Byte(WRITE_RTC_ADDR[i],TIME[i]);
	}
	Write_Ds1302_Byte(0x8E,0x80);  //ʹ��д����
}

void Set_DS1302_Time() {
	Write_Ds1302_Byte(0x8E,0x00);    //���д����
	Write_Ds1302_Byte(0x80,sec);    //���ã���
	Write_Ds1302_Byte(0x82,min);    //���ã���
	Write_Ds1302_Byte(0x84,hour);    //���ã�ʱ
	Write_Ds1302_Byte(0x8E,0x80);    //ʹ��д����
}

void Read_DS1302_Time() {
	if(F_pause == 0) {            //�ڷ���ͣģʽ��
		sec = Read_Ds1302_Byte(0x81);
		min = Read_Ds1302_Byte(0x83);
		hour = Read_Ds1302_Byte(0x85);
	}
	DisplaySMG_RTC();              //��ʾ���µ�ʱ����
}

void Scan_Keys() {
//S4��������ͣ����������
	if(S4 == 0) {
		DelaySMG(500);                  //ȥ����
		if(S4 == 0) {                    //����S4����
			if(F_pause == 0) {
				F_pause = 1;                //��־��ͣģʽ
			} else {
				F_pause = 0;                //��־����ģʽ
				Set_DS1302_Time();          //��������ʱ�������
			}

			while(S4 == 0) {              //���ּ��
				Read_DS1302_Time();
			}
		}
	}
	//S5���������ӵݼ�
	if(S5 == 0) {
		DelaySMG(500);                  //ȥ����
		if(S5 == 0) {
			if(F_pause == 1) {            //����ͣ��ʱ�������
				min = (min/16)*10 + min%16;  //BCD��תʮ����
				if(min == 0) {
					min = 59;                  //���ӵݼ��ı߽紦��
				} else {
					min = min - 1;            //���Ӽ�1
				}
				min = (min/10)*16 + min%10;  //ʮ����תBCD��
			}
			while(S5 == 0) {              //���ּ��
				Read_DS1302_Time();
			}
		}
	}
	//S6���������ӵ���
	if(S6 == 0) {
		DelaySMG(500);                  //ȥ����
		if(S6 == 0) {
			if(F_pause == 1) {            //����ͣ��ʱ�������
				min = (min/16)*10 + min%16;  //BCD��תʮ����
				if(min == 59) {
					min = 0;                  //���ӵ����ı߽紦��
				} else {
					min++;                    //���Ӽ�1
				}
				min = (min/10)*16 + min%10;  //ʮ����תBCD��
			}
			while(S6 == 0) {              //���ּ��
				Read_DS1302_Time();
			}
		}
	}
}

void Init_sys() {
	Set_HC573(0, 0x00);        //�ر�����������
	Set_HC573(5, 0x00);        //�رշ������ͼ̵���
	Set_HC573(4, 0xff);        //�ر�ȫ��LED��
	DisplaySMG_All(0xff);      //�ر�ȫ�������
	Init_DS1302_Param();      //��ʼ��DS1302
}

void main() {
	Init_sys();                //ϵͳ��ʼ��
	while(1) {
		Read_DS1302_Time();      //������ʾDS1302ʱ����
		Scan_Keys();            //ѭ��ɨ�谴��������
	}
}

//����д����Ư���� 
