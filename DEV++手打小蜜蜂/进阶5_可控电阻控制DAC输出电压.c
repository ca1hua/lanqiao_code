#include "reg52.h"
#include "iic.h"

#define TSMG 500

sbit S4 = P3^3;

unsigned char code SMG_NoDot[18]= {0xc0,0xf9,
    0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
    0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f};

unsigned char code SMG_Dot[10]= {0x40,0x79,
    0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};

unsigned char adc_value = 0;    //AIN3�Ĳ�������
float adc_volt = 0;              //AIN3�Ļ����ѹ
unsigned int smg_volt = 0;      //AIN3����ʾ��ѹ
unsigned char stat_smg = 1;      //��ǰ��ʾ�����־
unsigned char mode = 1;          //ģʽ

void DelaySMG(unsigned int t) {
	while(t--);
}

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

void DisplaySMG_Bit(unsigned char pos, unsigned char value) {
	Set_HC573(6, 0x01 << pos);    //����ܵĶ�λ
	Set_HC573(7, value);          //�������ʾ����
	DelaySMG(TSMG);                //����ܵ���ʱ��

	Set_HC573(6, 0x01 << pos);
	Set_HC573(7, 0xff);            //����
}

void DisplaySMG_All(unsigned char value) {
	Set_HC573(6, 0xff);    //ʹ�����������
	Set_HC573(7, value);  //�������ʾ����
}

void DisplaySMG_ADC() {
	DisplaySMG_Bit(7,SMG_NoDot[smg_volt  % 10]);
	DisplaySMG_Bit(6,SMG_NoDot[(smg_volt / 10) % 10]);
	DisplaySMG_Bit(5,SMG_Dot[smg_volt / 100]);
	//����ģʽ
	DisplaySMG_Bit(2,SMG_NoDot[16]);
	DisplaySMG_Bit(1,SMG_NoDot[mode]);
	DisplaySMG_Bit(0,SMG_NoDot[16]);
}

//==================PCF8591��ѹ����������================
//void IIC_Start(void);
//void IIC_Stop(void);
//bit IIC_WaitAck(void);
//void IIC_SendAck(bit ackbit);
//void IIC_SendByte(unsigned char byt);
//unsigned char IIC_RecByte(void);

void Read_PCF8591_AIN3() {
	IIC_Start();
	IIC_SendByte(0x90);    //PCF8591��д�豸��ַ
	IIC_WaitAck();
	IIC_SendByte(0x43);   //���DAC��ת��AIN3
	IIC_WaitAck();
	IIC_Stop();

	DisplaySMG_ADC();      //�ȴ���ѹת�����

	IIC_Start();
	IIC_SendByte(0x91);   //PCF8591�Ķ��豸��ַ
	IIC_WaitAck();
	adc_value = IIC_RecByte();  //����AD��������
	IIC_SendAck(1);         //������Ӧ���ź�
	IIC_Stop();
	//��ADC�����������ݻ���ɶ�Ӧ�ĵ�ѹֵ
	adc_volt = adc_value * (5.0 / 255);
	smg_volt = adc_volt * 100;
}

void Set_PCF8591_DAC(unsigned char dat) {
	IIC_Start();
	IIC_SendAck(0x90);  //PCF8591��д�豸��ַ
	IIC_WaitAck();
	IIC_SendByte(0x43);   // д������ֽڣ�DA ת����ͨ�� 3��

	IIC_WaitAck();
	IIC_SendByte(dat);     //����DAC��ѹ�������
	IIC_WaitAck();          //������Ӧ���ź�
	IIC_Stop();

}

void Scan_Keys() {
	if(S4 == 0) {
		DelaySMG(500);
		if(S4 == 0) {
			if(mode == 1) {
				mode = 2;
				Set_PCF8591_DAC(204);
				smg_volt = 400;
			} else if(mode == 2) {
				mode = 3;
				Read_PCF8591_AIN3();        //����AIN3��ѹ
				Set_PCF8591_DAC(adc_value);  //����DAC�������
			} else if(mode == 3) {
				mode = 1;
				Set_PCF8591_DAC(102);
				smg_volt = 200;
			}
			while(S4 == 0) {
				DisplaySMG_ADC();
			}
		}
	}
}

void Init_sys() {
	Set_HC573(0, 0x00);        //�ر�����������
	Set_HC573(5, 0x00);        //�رշ������ͼ̵���
	Set_HC573(4, 0xff);        //�ر�ȫ��LED��
	DisplaySMG_All(0xff);      //�ر�ȫ�������
	Set_PCF8591_DAC(102);      //����̶���ѹ2V
	smg_volt = 200;            //�������ʾ2.00V
}

void main() {
	Init_sys();
	while(1) {
		Scan_Keys();          //ѭ��ɨ�谴��
		DisplaySMG_ADC();      //��̬ˢ�������
	}
}
