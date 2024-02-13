#include "reg52.h"
#include "absacc.h"

unsigned char code SMG_DM_NoDot[18] =
    {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90,
     0x88, 0x80, 0xc6, 0xc0, 0x86, 0x8e, 0xbf, 0x7f};

unsigned int count_f = 0;
unsigned char count_t = 0;
unsigned int dat_f = 0;

void DelaySMG(unsigned int t)
{
    while (t--)
        ;
}

void DisplaySMG_Bit(unsigned char pos, unsigned char value)
{
    XBYTE[0xE000] = 0xff;
    XBYTE[0xC000] = 0x01 << pos;
    XBYTE[0xE000] = value;
}

void DisplaySMG_F()
{
    DisplaySMG_Bit(0, SMG_DM_NoDot[15]);
    DelaySMG(500);

    if (dat_f > 9999)
        DisplaySMG_Bit(3, SMG_DM_NoDot[dat_f / 10000]);
    DelaySMG(500);

    if (dat_f > 999)
        DisplaySMG_Bit(4, SMG_DM_NoDot[dat_f / 1000 % 10]);
    DelaySMG(500);

    if (dat_f > 99)
        DisplaySMG_Bit(5, SMG_DM_NoDot[dat_f / 100 % 10]);
    DelaySMG(500);

    if (dat_f > 9)
        DisplaySMG_Bit(6, SMG_DM_NoDot[dat_f / 10 % 10]);
    DelaySMG(500);

    DisplaySMG_Bit(7, SMG_DM_NoDot[dat_f % 10]);
    DelaySMG(500);
}

void Init_Timer()
{
    TH0 = 0xff;
    TL0 = 0xff;

    TH1 = (65536 - 50000) / 256;
    TL1 = (65536 - 50000) % 256;

    TMOD = 0x16; // 定时器1用方式1，定时；定时器0用方式2，计数

    ET0 = 1;
    ET1 = 1;
    EA = 1;
    TR0 = 1;
    TR1 = 1;
}

void Service_T0() interrupt 1
{
    count_f++;
}

void Service_T1() interrupt 3
{
    TH1 = (65536 - 50000) / 256;
    TL1 = (65536 - 50000) % 256;
    count_t++;
    if (count_t == 20)
    {
        dat_f = count_f;
        count_f = 0;
        count_t = 0;
    }
}

void main()
{
    Init_Timer();
    XBYTE[0x8000] = 0xff;
    XBYTE[0xA000] = 0x00;
    while (1)
    {
        DisplaySMG_F();
    }
}