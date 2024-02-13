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

	IIC_Start();          //��ʼ�ź�
	IIC_SendByte(0xa1);    //EEPROM�Ķ��豸��ַ
	IIC_WaitAck();        //�ȴ��ӻ�Ӧ��
	tmp = IIC_RecByte();  //��ȡ�ڴ��е�����
	IIC_SendAck(1);        //������Ӧ���ź�
	IIC_Stop();            //ֹͣ�ź�
	return tmp;
}

void Read_PCF8591_AIN1() {
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(0x01);
	IIC_WaitAck();
	IIC_Stop();

	DisplaySMG_Info();    //�ȴ���ѹת�����

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
		stat_led = 0xf8;        //�ȼ�3��L1��L3����
		level = 3;
	} else if(adc_volt > 2.5) {
		stat_led = 0xf0;        //�ȼ�4��L1��L4����
		level = 4;
	} else if(adc_volt > 2.0) {
		stat_led = 0xe0;        //�ȼ�5��L1��L5����
		level = 5;
	} else if(adc_volt > 1.5) {
		stat_led = 0xc0;        //�ȼ�6��L1��L6����
		level = 6;
	} else if(adc_volt > 1.0) {
		stat_led = 0x80;        //�ȼ�7��L1��L7����
		level = 7;
	} else {
		stat_led = 0x00;        //�ȼ�8��L1��L8����
		level = 8;
	}
	Set_HC573(4, stat_led);    //���µƹ��������
}

//================-=========================
void Save_to_24C02() {
	Write_24C02(0x01, level);      //�����������Ƶȼ�
	DisplaySMG_Info();
	Write_24C02(0x02, adc_value);  //������ղ�������
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
		DelaySMG(100);          //ȥ����
		if(S5 == 0) {
			Set_HC573(4, 0xff);    //�ر�ȫ���ƹ�
			Read_from_24C02();    //��ȡ��ʷ����
			while(S5 == 0) {      //���ּ��
				DisplaySMG_Info();  //��ʾ��ʷ����
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
  Init_sys();                //ϵͳ��ʼ��        
  while(1)
  {
    Scan_Keys();            //ɨ�谴��
    LED_Auto_Control();      //�����Զ�����
  }
}

