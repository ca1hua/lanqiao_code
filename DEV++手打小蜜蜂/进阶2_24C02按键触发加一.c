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
  Set_HC573(6, 0xff);    //ʹ�����������
  Set_HC573(7, value);  //�������ʾ����  
}

void DelaySMG(unsigned int t) {
	while(t--);
}

void DisplaySMG_Bit(unsigned char pos,unsigned char value) {
	Set_HC573(6, 0x01 << pos);    //����ܵĶ�λ
	Set_HC573(7, value);          //�������ʾ����
	DelaySMG(500);
	Set_HC573(6, 0x01 << pos);
	Set_HC573(7, 0xff);  //xiaoyin
}

void DisplaySMG_24C02() {
	//����������0λ
	DisplaySMG_Bit(7,SMG_NoDot[dat3  % 10]);
	//����������1λ
	DisplaySMG_Bit(6,SMG_NoDot[dat3 / 10]);
	//����������2λ
	DisplaySMG_Bit(5,SMG_NoDot[16]);

	//����������3λ
	DisplaySMG_Bit(4,SMG_NoDot[dat2  % 10]);
	//����������4λ
	DisplaySMG_Bit(3,SMG_NoDot[dat2 / 10]);
	//����������5λ
	DisplaySMG_Bit(2,SMG_NoDot[16]);

	//����������6λ
	DisplaySMG_Bit(1,SMG_NoDot[dat1 % 10]);
	//����������7λ
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
} //д �� ��

unsigned char Read_24C02(unsigned char addr) {

//AT24C02��һ��2Kλ����CMOS E2PROM
	unsigned char tmp;
	//���ȣ�����һ��αд����
	IIC_Start();          //��ʼ�ź�
	IIC_SendByte(0xa0);    //EEPROM��д�豸��ַ
	IIC_WaitAck();        //�ȴ��ӻ�Ӧ��
	IIC_SendByte(addr);    //�ڴ浥Ԫ��ַ
	IIC_WaitAck();        //�ȴ��ӻ�Ӧ��
	//Ȼ�󣬿�ʼ�ֽڶ�����
	IIC_Start();          //��ʼ�ź�
	IIC_SendByte(0xa1);    //EEPROM�Ķ��豸��ַ
	IIC_WaitAck();        //�ȴ��ӻ�Ӧ��
	tmp = IIC_RecByte();  //��ȡ�ڴ��е�����
	IIC_SendAck(1);        //������Ӧ���ź�
	IIC_Stop();            //ֹͣ�ź�
	return tmp;
}

//ȥ����
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
	//����S5��ɨ�账��
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

	//����S6��ɨ�账��
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
			} //���°�����ʱ�� ��ҲҪһֱɨ����������
		}
	}

}

void Init_sys() {

	Set_HC573(0, 0x00);        //�ر�����������
	Set_HC573(5, 0x00);        //�رշ������ͼ̵���
	Set_HC573(4, 0xff);        //�ر�ȫ��LED��
	DisplaySMG_All(0xff);      //�ر�ȫ�������

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
