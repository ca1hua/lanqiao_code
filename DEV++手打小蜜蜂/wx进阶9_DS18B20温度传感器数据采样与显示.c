#include "reg52.h"
#include "onewire.h"

#define TSMG 500

sbit S4 = P3 ^ 3;

unsigned char code SMG_NoDot[18] = {
    0xc0, 0xf9,
    0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90,
    0x88, 0x80, 0xc6, 0xc0, 0x86, 0x8e, 0xbf, 0x7f};

unsigned char code SMG_Dot[10] = {
    0x40, 0x79,
    0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10};

float temp_ds18b20 = 0;
unsigned int smg_ds18b20 = 0;
unsigned char mode = 1;

void DelaySMG(unsigned int t)
{
    while(t--);
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

void DisplaySMG_All(unsigned char value)
{
    Set_HC573(6, 0xff);
    Set_HC573(7, value);
}

void DisplaySMG_Temp()
{
    switch (mode)
    {
    case 1:
        smg_ds18b20 = temp_ds18b20 * 100;
        DisplaySMG_Bit(7, SMG_NoDot[smg_ds18b20 % 10]);
        DisplaySMG_Bit(6, SMG_NoDot[(smg_ds18b20 / 10) % 10]);
        DisplaySMG_Bit(5, SMG_Dot[(smg_ds18b20 / 100) % 10]);
        if (smg_ds18b20 / 1000 != 0)
        {
            DisplaySMG_Bit(4, SMG_NoDot[smg_ds18b20 / 1000]);
        }
        break;
    case 2:
        smg_ds18b20 = temp_ds18b20 * 10;
        DisplaySMG_Bit(7, SMG_NoDot[smg_ds18b20 % 10]);
        DisplaySMG_Bit(6, SMG_Dot[(smg_ds18b20 / 10) % 10]);
        if (smg_ds18b20 / 100 != 0)
        {
            DisplaySMG_Bit(5, SMG_NoDot[smg_ds18b20 / 100]);
        }
        break;
    case 3:
        smg_ds18b20 = temp_ds18b20;
        DisplaySMG_Bit(7, SMG_NoDot[smg_ds18b20 % 10]);
        if (smg_ds18b20 / 10 != 0)
        {
            DisplaySMG_Bit(6, SMG_NoDot[smg_ds18b20 / 10]);
        }
        break;
    }
}

void Read_Display_DS18B20()
{
    unsigned char LSB, MSB;
    unsigned int temp = 0;

    init_ds18b20();
    DisplaySMG_Temp();
    Write_DS18B20(0xcc); // 忽略ROM操作
    Write_DS18B20(0x44); // 启动温度转化
    DisplaySMG_Temp();

    init_ds18b20();
    DisplaySMG_Temp();
    Write_DS18B20(0xcc); // 忽略ROM操作
    Write_DS18B20(0xbe); // 读出内部存储器
    LSB = Read_DS18B20();
    MSB = Read_DS18B20();
    DisplaySMG_Temp();

    temp = MSB;
    temp = (temp << 8) | LSB;
    if ((temp & 0xf800) == 0x0000)
    {
        // 处理正温度
        temp_ds18b20 = temp * 0.0625;
    }
    DisplaySMG_Temp();
}

void Scan_Keys()
{
    if (S4 == 0)
    {
        DelaySMG(100);
        if (S4 == 0)
        {
            while (S4 == 0)
            {
                DisplaySMG_Temp();
            }
            if (mode == 1)
            {
                mode = 2;
            }
            else if (mode == 2)
            {
                mode = 3;
            }
            else if (mode == 3){
                mode = 1;
            }
        }
    }
}

void Init_sys(){
    Set_HC573(0,0x00);
    Set_HC573(5,0x00);
    Set_HC573(4,0xff);
    DisplaySMG_All(0xff);
}

void main(){
    Init_sys();
    while(1){
        Scan_Keys();
        Read_Display_DS18B20();
    }
}