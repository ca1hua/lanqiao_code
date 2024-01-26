#include "reg52.h"


sbit L8 = P0^7;
sbit L1 = P0^0;

void SelectHC573(unsigned char channel)
{
	switch(channel)
	{
		case 4: P2 = (P2 & 0x1f) | 0x80; break;
		case 5: P2 = (P2 & 0x1f) | 0xa0; break;
		case 6: P2 = (P2 & 0x1f) | 0xc0; break;
		case 7: P2 = (P2 & 0x1f) | 0xe0; break;
	}
}

void Init()
{
	SelectHC573(5);
	P0 = 0x00;
	SelectHC573(4);
	P0 = 0xff;

}

void InitTime1()
{
	IT1 = 1;
	EX1 = 1;
	EA = 1;
}

void Delay(unsigned int t)
{
	while(t--);
	while(t--);
}

void Serv_Time1() interrupt 2
{
	SelectHC573(4);
	L8 = 0;
	Delay(60000);
	Delay(60000);
	L8 = 1;
}

void working()
{
	SelectHC573(4);
	L1 = 0;
	Delay(60000);
	L1 = 1;
	Delay(60000);
}

void main()
{
	Init();
	InitTime1();
	while(1)
	{
		working();
	}
} 
