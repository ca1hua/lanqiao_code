#include <reg52.h>

sfr AUXR = 0x8e;


void SelectHC573(unsigned char channel)
{
	switch(channel)
	{
		case 4: P2 = (P2 & 0x1f) | 0x80; break;
		case 5: P2 = (P2 & 0x1f) | 0xa0; break;
		case 6: P2 = (P2 & 0x1f) | 0xc0; break;
		case 7: P2 = (P2 & 0x1f) | 0xe0; break;
		case 0: P2 = (P2 & 0x1f) | 0x00; break;
	}
}

void Init()
{
	SelectHC573(5);
	P0 = 0x00;
	SelectHC573(4);
	P0 = 0xff;
}

//发送字节
void SendByte(unsigned char dat)
{
	SBUF = dat;
	while(TI == 0);
	TI = 0;
}

//发送字符串
void SendString(unsigned char *str)
{
	while(*str != '\0')
	{
		SendByte(*str++);
	}
}

//                 串口 
//=============================================
void Init_Uart()
{
	TMOD = 0x20;
	TH1 = 0xfd;
	TL1 = 0xfd;
	TR1 = 1;
	
	SCON = 0x50;
	AUXR = 0x00;
	
	ES = 1;
	EA = 1;//8行  铁代码 
}

unsigned char command = 0x00;
void Service_Uart() interrupt 4
{
	if(RI == 1)
	{
		RI = 0;
		command = SBUF;
	}
}
//=============================================

unsigned char test1 = 0xf0;
unsigned char test2 = 0x0f;   //高 1111 0000 低 
unsigned char n = 0x00; 
void Working()
{
	switch(test1 & command)
	{
		case 0xa0:
			//1010 0001     0000 1111
			n = ~((command & test2)) ; 
			SelectHC573(4);
			P0 = (P0 | 0x0f) & n;
			command = 0x00;
			break;
		case 0xb0: 
			//1010 0001 
			n = ~((command & test2)<<4) ; 
			SelectHC573(4);
			P0 = (P0 | 0xf0) & n;
			command = 0x00;
			break;
		case 0xc0:
			SendString("The System is Running...\r\n");
			command = 0x00;
			break;
	}
}



void main()
{	
	Init();
	Init_Uart();
	SendString("Welcome to XMF system!\r\n");
	while(1)
	{
		Working();	
	}
}
