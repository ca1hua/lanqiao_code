#include "reg52.h"

sbit S7 = P3^0;
sbit L1 = P0^0;

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

void InitTimer()
{
	TMOD = 0x01;
	TH0 = (65535 - 100) / 256;
	TL0 = (65535 - 100) % 256;
	
	ET0 = 1;
	EA = 1;

}


unsigned char pwm_duty = 0;
unsigned char count = 0;
void ServiceTimer() interrupt 1
{
	TH0 = (65535 - 100) / 256;
	TL0 = (65535 - 100) % 256;
	count++;
}


void PWM()
{
	if(count == pwm_duty)
	{
		L1 = 1;
	}
	else if(count == 100)
	{
		L1 = 0;
		count = 0;
	}
}

void Delay(unsigned int t)
{
	while(t--);
}

unsigned char stat = 0;
void ScanKey()
{
	if(S7 == 0)
	{
		Delay(100);
			if(S7 == 0)
			{
			switch(stat)
			{
				case 0:
				    TR0 = 1;
					pwm_duty = 10;
				    stat = 1;
				break;
				case 1:
					pwm_duty = 50;
					stat = 2;
				break;
				case 2:
					pwm_duty = 90;
					stat = 3;
				break;
				case 3:
					L1 = 1;
				    TR0 = 0;
				    stat = 0;
				break;
			}
					
			}
			while(S7 == 0);
	}

}

void Init()
{
	SelectHC573(5);
	P0 = 0x00;
	SelectHC573(4);
	P0 = 0xff;
}



void main()
{
	Init();
	InitTimer();
	while(1)
	{
		ScanKey();
		PWM();
	}
}
