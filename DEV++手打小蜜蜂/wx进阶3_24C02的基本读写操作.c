#include "reg52.h"
#include "iic.h"

#define TSMG 500

unsigned char code SMG_NoDot[18] = {
	0xc0, 0xf9,
	0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90,
	0x88, 0x83, 0xc6, 0xc0, 0x86, 0x8e, 0xbf, 0x7f};

unsigned char dat1 = 0, dat2 = 0, dat3 = 0;

void DelaySMG(unsigned int t)
{
	while (t--)
		;
}

void Set_HC573(unsigned char channel, unsigned char dat)
{
	P0 = dat;
	switch (channel)
	{
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

void DisplaySMG_Bit(unsigned char pos, unsigned char value)
{
	Set_HC573(6, 0x01 << pos);
	Set_HC573(7, value);
	DelaySMG(TSMG);
	Set_HC573(6, 0x01 << pos);
	Set_HC573(7, 0xff);
}

void Display_All(unsigned char value)
{
	Set_HC573(6, 0xff);
	Set_HC573(7, value);
}

void DisplaySMG_24C02()
{

	DisplaySMG_Bit(7, SMG_NoDot[dat3 % 10]);
	DisplaySMG_Bit(6, SMG_NoDot[dat3 / 10]);

	DisplaySMG_Bit(5, SMG_NoDot[16]);

	DisplaySMG_Bit(4, SMG_NoDot[dat2 % 10]);
	DisplaySMG_Bit(3, SMG_NoDot[dat2 / 10]);

	DisplaySMG_Bit(2, SMG_NoDot[16]);

	DisplaySMG_Bit(1, SMG_NoDot[dat1 % 10]);
	DisplaySMG_Bit(0, SMG_NoDot[dat1 / 10]);
}

void Write_24C02(unsigned char addr, unsigned char dat)
{
	IIC_Start();
	IIC_SendByte(0xa0); // EEPROM��д���豸��ַ
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();
	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();
	// �˶���
}

unsigned char Read_24C02(unsigned char addr)
{
	unsigned char tmp;
	//=====���α����======
	IIC_Start();
	IIC_SendByte(0xa0); // EEPROM��д���豸��ַ
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();
	//====================
	IIC_Start();
	IIC_SendByte(0xa1); // EEPROM�Ķ��豸��ַ
	IIC_WaitAck();
	tmp = IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
	return tmp;
}

void Read_Write_24C02()
{
	dat1 = Read_24C02(0x01); // 从24C02读取dat1
	dat2 = Read_24C02(0x03); // 从24C02读取dat2
	dat3 = Read_24C02(0x05); // 从24C02读取dat3

	dat1 = dat1 + 1; // 第1个计数变量+1
	dat2 = dat2 + 2; // 第2个计数变量+2
	dat3 = dat3 + 3; // 第3个计数变量+3
	if (dat1 > 10)
		dat1 = 0;
	if (dat2 > 20)
		dat2 = 0;
	if (dat3 > 30)
		dat3 = 0;

	Write_24C02(0x01, dat1);
	DelaySMG(5000); // 等待字节写入完成
	Write_24C02(0x03, dat2);
	DelaySMG(5000); // 等待时间不够会影响dat3写入
	Write_24C02(0x05, dat3);
	DelaySMG(5000);
}

void Init_sys()
{
	Set_HC573(0, 0x00);
	Set_HC573(5, 0x00);
	Set_HC573(4, 0xff);
	Display_All(0xff);
}

void main()
{
	Init_sys();
	Read_Write_24C02();
	while (1)
	{
		DisplaySMG_24C02();
	}
}