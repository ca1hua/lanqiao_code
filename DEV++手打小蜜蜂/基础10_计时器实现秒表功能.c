#include "reg52.h"

unsigned char arr[19] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,
     0x80,0x90,0x88,0x80,0xc6,0xc0,0x86,0x8e,
     0xbf,0x7f,0xbf};  //       hgfe dcba 1011 1111

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

void InitTimer()
{
	TMOD = 0x01;
	TH0 = (65535 - 50000) / 256;
	TL0 = (65535 - 50000) % 256;
	
	ET0 = 1;
	EA = 1;
	TR0 = 1;
}

void Display_SMG(unsigned char n,unsigned char pos)
{
	SelectHC573(6);
	P0 = 0x01 << pos;
	SelectHC573(7);
	P0 = arr[n];
}

void DelayK(unsigned int t)
{
	while(t--);
}

unsigned char cms = 0;
unsigned char m = 0;
unsigned char f = 0;
void ServiceTimer() interrupt 1
{
	TH0 = (65535 - 50000) / 256;
	TL0 = (65535 - 50000) % 256;
	cms++;
	if(cms == 20)
	{
		m++;
		cms = 0;
	}
	if(m == 60)
	{
		f++;
		m = 0;
	}
	if(f == 100)
	{
		f = 0;
	}
	
}

void True_Display()
{
		Display_SMG(f/10, 0);
	DelayK(500);
	Display_SMG(f%10, 1);
	DelayK(500);
	Display_SMG(18, 2);
	DelayK(500);
	Display_SMG(m/10, 3);
	DelayK(500);
	Display_SMG(m%10, 4);
	DelayK(500);
	Display_SMG(18, 5);
	DelayK(500);
	Display_SMG(cms/10, 6);
	DelayK(500);
	Display_SMG(cms%10, 7);
	DelayK(500);
}

void main()
{
	Init();
	InitTimer();
	while(1)
	{
		True_Display();
	}
} 
