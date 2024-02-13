#include "reg52.h"
#include "iic.h"
sbit S4 = P3^3;
sbit S5 = P3^2;
sbit S6 = P3^1;
//���嶯̬��ʾ�е�������ܵ���ʱ��
#define TSMG  500
//-------��������ܵĶ���������С���㣩--------
//0 1 2 3 4 5 6 7 8 9 A B C D E F - .
unsigned char code SMG_NoDot[18]= {
	0xc0,0xf9,
	0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
	0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f
};
//-------��������ܵĶ���������С���㣩--------
//0. 1. 2. 3. 4. 5. 6. 7. 8. 9.
unsigned char code SMG_Dot[10]= {
	0x40,0x79,
	0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10
};
//-----------------------------------------------
unsigned int adc_value = 0;      //AIN3�Ĳ�������
float adc_volt = 0;              //AIN3�Ļ����ѹ
unsigned int smg_volt = 0;      //AIN3����ʾ��ѹ
unsigned int param = 300;        //��ѹ������3.00V
unsigned char stat_smg = 1;      //��ǰ��ʾ�����־
unsigned char num = 0;          //������������
unsigned int count_t = 0;        //50ms��λ�ۼƱ���
unsigned char stat_led = 0xff;  //LED�ƿ���״̬
unsigned char F_shan = 0;        //������־
unsigned char F_time = 0;        //��ʱ��־

//==============����ܶ�̬��ʾר����ʱ����=============
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
	Set_HC573(6, 0xff);    //ʹ�����������
	Set_HC573(7, value);  //�������ʾ����
}
//=================ϵͳ��Ϣ�������ʾ=================
void DisplaySMG_Info() {
	switch(stat_smg) {
			//��ѹ���ݽ���
		case 1:
			DisplaySMG_Bit(7,SMG_NoDot[smg_volt  % 10]);
			DisplaySMG_Bit(6,SMG_NoDot[(smg_volt / 10) % 10]);
			DisplaySMG_Bit(5,SMG_Dot[smg_volt / 100]);
			//��ʾ����U---->0xC1
			DisplaySMG_Bit(0,0xc1);
			break;
			//�������ý���
		case 2:
			DisplaySMG_Bit(7,SMG_NoDot[param  % 10]);
			DisplaySMG_Bit(6,SMG_NoDot[(param / 10) % 10]);
			DisplaySMG_Bit(5,SMG_Dot[param / 100]);
			//��ʾ����P---->0x8C
			DisplaySMG_Bit(0,0x8c);
			break;
			//������������
		case 3:
			DisplaySMG_Bit(7,SMG_NoDot[num  % 10]);
			if(num / 10 != 0) {
				DisplaySMG_Bit(6,SMG_NoDot[num / 10]);
			}
			//��ʾ����S---->0x92
			DisplaySMG_Bit(0,0x92);
			break;

	}
}
//===================��ʱ��T0��ʼ��===================
void Init_Timer0() {
	//��ʱ��������ֵ���ã�50ms
	TH0 = (0 - 50000) / 256;
	TL0 = (0 - 50000) % 256;
	TMOD = 0x01;      //T0��ģʽ1��16λ������װģʽ
	ET0 = 1;          //ʹ�ܶ�ʱ��0�ж�
	EA = 1;           //�����ж�
	TR0 = 1;          //������ʱ��0
}
//===============��ʱ��T0���жϷ�����================
void Service_Timer0() interrupt 1 {
	TH0 = (0 - 50000) / 256;
	TL0 = (0 - 50000) % 256;

	if(F_time == 1) {            //����������ʱ
		count_t++;                //50ms��λ��ʱ�ۼ�

		if(count_t % 20 == 0) {    //1��ʱ��
			if(num != 99) {          //û�е������ֵ99ǰ
				num++;                //���������ۼ�
			}
			if(F_shan == 1) {        //������������
				if((stat_led & 0x80) == 0x80) {
					stat_led &= ~0x80;  //����L8��
				} else {
					stat_led |= 0x80;    //Ϩ��L8��
				}
			}
		}

		if(count_t > 120) {        //����6��
			stat_led &= ~0x07;      //����L1 L2 L3
			F_shan = 1;
		} else if(count_t > 80) { //����4��
			stat_led &= ~0x03;      //����L1 L2
		} else if(count_t > 40) {  //����2��
			stat_led &= ~0x01;      //����L1
		} else {
			stat_led = 0xff;
		}
	}
}
//===============PCF8591��ѹ����������===============
unsigned char Read_PCF8591_AIN3() {
	unsigned char tmp;
	IIC_Start();
	IIC_SendByte(0x90);      //PCF8591��д�豸��ַ
	IIC_WaitAck();
	//ѡ��ADת��ͨ��
	IIC_SendByte(0x03);     //AIN3ͨ�����ɵ������ѹ
	IIC_WaitAck();
	IIC_Stop();
	DisplaySMG_Info();      //�ȴ���ѹת�����
	IIC_Start();
	IIC_SendByte(0x91);     //PCF8591�Ķ��豸��ַ
	IIC_WaitAck();
	tmp = IIC_RecByte();
	IIC_SendAck(1);           //������Ӧ���ź�
	IIC_Stop();
	return tmp;
}
//=================��ѹ����Ӧ�÷�������=================
void App_Volt() {
	unsigned char i;
	//����ͨ��3�ĵ�ѹ�������򵥵ľ�ֵ�˲�����
	adc_value = 0;
	for(i = 0; i < 3; i++) {
		adc_value += Read_PCF8591_AIN3();
	}
	adc_value = adc_value / 3;
	//��ADC�����������ݻ���ɶ�Ӧ�ĵ�ѹֵ
	adc_volt = adc_value * (5.0 / 255);
	smg_volt = adc_volt * 100;
	//��ǰ��ѹ���ѹ�����ıȽϴ���
	if(smg_volt < param) {        //��ǰ��ѹ < ��ѹ����
		if(F_time == 0) {
			F_time = 1;                //��־�������ʱ��
		}
	} else {                      //��ǰ��ѹ >= ��ѹ����
		F_time = 0;                  //�ָ�����ʱ״̬
		F_shan = 0;
		count_t = 0;                //��ʱ������0
		num = 0;
		stat_led |= 0xff;            //ָʾ��L1Ϩ��
	}
	DisplaySMG_Info();            //��̬ˢ�������
	Set_HC573(4, stat_led);        //����LED�ƿ���״̬
}
//===================����ɨ�账����==================
void Scan_Keys() {
	if(S4 == 0) {                  //�л�����
		DelaySMG(500);
		if(S4 == 0) {
			if(stat_smg == 1) {
				stat_smg = 2;            //�л�������������
			} else if(stat_smg == 2) {
				stat_smg = 3;            //�л�������������
			} else if(stat_smg == 3) {
				stat_smg = 1;            //�л�������ѹ����
			}
			while(S4 == 0) {          //���ּ��
				App_Volt();              //���ֵ�ѹ��������ʾ
			}
		}
	}

	if(S5 == 0) {                  //��ѹ������С
		DelaySMG(500);
		if(S5 == 0) {
			if(stat_smg == 2) {        //���ڲ�����������Ч
				if(param != 0) {        //δ��С����Сֵ
					param = param - 50;    //ÿ��1�£���С0.5V
				} else {
					param = 500;          //�߽紦���ָ�5.00V
				}
			}
			while(S5 == 0) {
				App_Volt();
			}
		}
	}

	if(S6 == 0) {                  //��ѹ��������
		DelaySMG(500);
		if(S6 == 0) {
			if(stat_smg == 2) {        //���ڲ�����������Ч
				if(param != 500) {      //δ���ӵ����ֵ
					param = param + 50;    //ÿ��1�£�����0.5V
				} else {
					param = 0;            //�߽紦���ָ�0.00V
				}
			}
			while(S6 == 0) {
				App_Volt();
			}
		}
	}
}
//=====================ϵͳ��ʼ��=====================
void Init_sys() {
	Set_HC573(0, 0x00);        //�ر�����������
	Set_HC573(5, 0x00);        //�رշ������ͼ̵���
	Set_HC573(4, 0xff);        //�ر�ȫ��LED��
	DisplaySMG_All(0xff);      //�ر�ȫ�������
	Init_Timer0();             //��ʼ����������ʱ��0
}
//=======================������=======================
void main() {
	Init_sys();                //ϵͳ��ʼ��
	while(1) {
		App_Volt();              //������ѹ������
		Scan_Keys();             //ѭ��ɨ�谴��
	}
}
