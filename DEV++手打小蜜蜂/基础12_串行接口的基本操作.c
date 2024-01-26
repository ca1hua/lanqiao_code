#include "reg52.h"

sfr AUXR = 0x8e;

void SelectHC573(unsigned char n)
{
	switch(n)
	{
		case 4: P2 = ( P2 & 0x1f ) | 0x80; break;
		case 5: P2 = ( P2 & 0x1f ) | 0xa0; break;
		case 6: P2 = ( P2 & 0x1f ) | 0xc0; break;
		case 7: P2 = ( P2 & 0x1f ) | 0xe0; break;
	}
}

void Init()
{
	SelectHC573(5);
	P0 = 0x00;
	SelectHC573(4);
	P0 = 0xff;
}

void Init_Uart()
{
	TMOD = 0x20;
	TH1 = TL1 =0xfd;
	SCON = 0x50;
	AUXR = 0x00;
	TR1= 1;
	ES = 1;
	EA = 1;
}

unsigned char dat = 0;
void Service_Uart() interrupt 4
{
	if(RI == 1)
	{
		RI = 0;
		dat = SBUF;
		dat++;
		SBUF = dat;
		while(TI == 0);//这个发送底层好像还没搞好 
		TI = 0;
	}
}

void SendByte(unsigned char dat)
{
	SBUF = dat;
	while(TI == 0);
	TI = 0;
}

void Delay(unsigned int t)
{
	while(t--);
}
void main()
{
	Init();
	Delay(200);
	SendByte(0x5a);
	SendByte(0xa5);
	Init_Uart(); 
	while(1)
	{
		
	}
} 
