#include "reg52.h"

unsigned char arr[19] ={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,
     0x80,0x90,0x88,0x80,0xc6,0xc0,0x86,0x8e,
     0xbf,0x7f,0x00};//16是 - 

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

void SMG_Display(unsigned char n,unsigned char pos)
{
    P0 = 0xff;// 消隐 
	SelectHC573(6);
	P0 = 0x01 << pos;
	SelectHC573(7);
	P0 = arr[n];
}

void Delay(unsigned int t)
{
	while(t--);
	while(t--);
}

unsigned char i;
void test_one()
{
	SelectHC573(4);
	for(i = 1;i <= 8;i++)
	{
		P0 = 0xff << i;
		Delay(60000);
		Delay(60000);
	}
		for(i = 1;i <= 8;i++)
	{
		P0 = ~(0xff << i);//1111 1111
		Delay(60000);
		Delay(60000);
	}

	for(i = 0;i <= 8;i++)
	{
		SMG_Display(18,i);
		Delay(60000);
		Delay(60000);
	}
}

void Delay_SMG(unsigned int t)
{
	while(t--);
}


unsigned char s = 0;
unsigned char m = 0;
unsigned char h = 0;
unsigned char count = 0;
//==============定时器中断================
void InitTimer()
{
	TMOD = 0x21;
	TH0 = (65535 - 50000) / 256;
	TL0 = (65535 - 50000) % 256;
	
	ET0 = 1;					//使能定时器T0
	EA = 1;						//使能总中断
	TR0 = 1;					//启动定时器T0
} 

void Severcie() interrupt 1
{
	TH0 = (65535 - 50000) / 256;
	TL0 = (65535 - 50000) % 256;
	count++;
if(count == 20)
	{
		count = 0;
		s++;
	}
	if(s == 60)
	{
		s = 0;
		m++;
		if(m == 60)
		{
			m = 0;
			h++;
		}
	}
}
//======================================== 

void SMG_Dy()
{

	SMG_Display(h/10,0);
	Delay_SMG(500);
	SMG_Display(h%10,1);
	Delay_SMG(500);
	SMG_Display(16,2);
	Delay_SMG(500);
	SMG_Display(m/10,3);
	Delay_SMG(500);
	SMG_Display(m%10,4);
	Delay_SMG(500);
	SMG_Display(16,5);
	Delay_SMG(500);
	SMG_Display(s/10,6);
	Delay_SMG(500);
	SMG_Display(s%10,7);
	Delay_SMG(500);
		
}

void main()
{
	Init();
test_one();
	InitTimer();
	while(1)
	{
		SMG_Dy();
	}
} //续写。。。 
