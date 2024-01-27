#include "reg52.h"
#include "absacc.h"

void Init()
{
	XBYTE[0x8000] = 0xff;
	XBYTE[0xa000] = 0x00; 
}

void Delay(unsigned int t)
{
	while(t--);
	while(t--);
}

void LEDRunning()
{
	XBYTE[0x8000] = 0xf0;
	Delay(60000);
	XBYTE[0x8000] = 0x0f;
	Delay(60000);
	XBYTE[0x8000] = 0xff;
	Delay(60000);
}

unsigned char i;
void SMGRunning()
{
	for(i = 0; i <= 8; i++)
	{
		XBYTE[0xc000] = 0x01 << i;//0000 000 1000 0000
		XBYTE[0xe000] = 0x00; 
		Delay(60000);
	}
	
}

void main()
{
	Init();
	while(1)
	{
		LEDRunning();
		SMGRunning();
	}
}
