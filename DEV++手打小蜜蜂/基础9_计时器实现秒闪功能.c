#include "reg52.h"

sbit L8 = P0^7;
sbit L1 = P0^0;

void SelectHC573(unsigned char n)
{
	switch(n)
	{
		case 4:P2 = (0x1f & P2) | 0x80; break;
		case 5:P2 = (0x1f & P2) | 0xa0; break;
		case 6:P2 = (0x1f & P2) | 0xc0; break;
		case 7:P2 = (0x1f & P2) | 0xe0; break;
	}
}

void Init()
{
	SelectHC573(5);
	P0 = 0x00;
	SelectHC573(4);
	P0 = 0xff;
}

void InitTimer()
{
	TMOD = 0x01;
	TH0 = (65535 - 50000) / 256;
	TL0 = (65535 - 50000) % 256;
	ET0 = 1;
	EA = 1;
	TR0 = 1;
}


unsigned char count = 0;
void ServiceTimer() interrupt 1
{
	TH0 = (65535 - 50000) / 256;
	TL0 = (65535 - 50000) % 256;
	count++;
	if(count % 10 == 0)
	{
		L1 = ~L1;
	}
	if(count == 100 )
	{
		L8 = ~L8;
		count = 0;
	}
}

void main()
{
	Init();
	InitTimer();
	while(1)
	{
		
	}
} 
