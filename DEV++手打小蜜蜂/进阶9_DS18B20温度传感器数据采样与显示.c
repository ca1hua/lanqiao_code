/*==================���ű���Ƭ����ѵ==================
������09����DS18B20�¶ȴ��������ݲ�������ʾ
**ƽ  ̨��CT107D��Ƭ���ۺ�ʵѵƽ̨
**ģ  ʽ��IOģʽ
**�ײ������ļ���2022�꾺����Դ���ݰ��ṩ���ļ�
**��  �ƣ�ŷ��Դ��С�۷���ʦ��ohy3686@qq.com��
**ʱ  �䣺2022-04-09
**���������www.xmf393.com
====================================================*/

#include "reg52.h"
#include "onewire.h"

sbit S4 = P3^3;

//���嶯̬��ʾ�е�������ܵ���ʱ��
#define TSMG  500
//-------��������ܵĶ���������С���㣩--------
unsigned char code SMG_NoDot[18]={0xc0,0xf9,
    0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
    0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f};
//-------��������ܵĶ���������С���㣩--------
//0.��9.
unsigned char code SMG_Dot[10]={0x40,0x79,
    0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};
//----------------------------------------------

float temp_ds18b20 = 0;          //ʵ���¶�ֵ
unsigned int smg_ds18b20 = 0;    //�������ʾ�¶�ֵ
unsigned char mode = 1;          //��ʾģʽ

//==============����ܶ�̬��ʾר����ʱ����=============
void DelaySMG(unsigned int t)
{
  while(t--);
}
//==================����ѡͨ��������===================
void Set_HC573(unsigned char channel, unsigned char dat)
{
  P0 = dat;                      //����������
  switch(channel)                //ѡͨ������
  {
    case 4:
      P2 = (P2 & 0x1f) | 0x80;  //Y4���0��LED����
    break;
    case 5:
      P2 = (P2 & 0x1f) | 0xa0;  //Y5���0���������ͼ̵�������
    break;
    case 6:
      P2 = (P2 & 0x1f) | 0xc0;  //Y6���0�������λѡ
    break;
    case 7:
      P2 = (P2 & 0x1f) | 0xe0;  //Y7���0������ܶ���
    break;
    case 0:
      P2 = (P2 & 0x1f) | 0x00;  //������������ѡ��
    break;
  }
  P2 = (P2 & 0x1f) | 0x00;      //������ɣ��ر�ȫ��������
}
//=================��������ܶ�̬��ʾ=================
void DisplaySMG_Bit(unsigned char pos, unsigned char value)
{
  Set_HC573(6, 0x01 << pos);    //����ܵĶ�λ
  Set_HC573(7, value);          //�������ʾ����
  DelaySMG(TSMG);                //����ܵ���ʱ��
  Set_HC573(6, 0x01 << pos);        
  Set_HC573(7, 0xff);            //����
}
//=================ȫ������ܾ�̬��ʾ=================
void DisplaySMG_All(unsigned char value)
{
  Set_HC573(6, 0xff);    //ʹ�����������
  Set_HC573(7, value);  //�������ʾ����  
}
//=================�¶�ֵ��ʾ����=================
void DisplaySMG_Temp()
{
  switch(mode)
  {
    //2λС����ʾģʽ
    case 1:
      smg_ds18b20 = temp_ds18b20 * 100;
      DisplaySMG_Bit(7,SMG_NoDot[smg_ds18b20  % 10]);
      DisplaySMG_Bit(6,SMG_NoDot[(smg_ds18b20 / 10) % 10]);    
      DisplaySMG_Bit(5,SMG_Dot[(smg_ds18b20 / 100) % 10]);
      if(smg_ds18b20 / 1000 != 0)
      {
        DisplaySMG_Bit(4,SMG_NoDot[smg_ds18b20 / 1000]);
      }
    break;
    //1λС����ʾģʽ
    case 2:
      smg_ds18b20 = temp_ds18b20 * 10;
      DisplaySMG_Bit(7,SMG_NoDot[smg_ds18b20  % 10]);
      DisplaySMG_Bit(6,SMG_Dot[(smg_ds18b20 / 10) % 10]);    
      if(smg_ds18b20 / 100 != 0)
      {
        DisplaySMG_Bit(5,SMG_NoDot[smg_ds18b20 / 100]);
      }
    break;
    //������ʾģʽ
    case 3:
      smg_ds18b20 = temp_ds18b20;
      DisplaySMG_Bit(7,SMG_NoDot[smg_ds18b20  % 10]);
      if(smg_ds18b20 / 10 != 0)
      {
        DisplaySMG_Bit(6,SMG_NoDot[smg_ds18b20 / 10]);
      }
    break;
  }
}
//==============DS18B20�¶ȶ�������ʾ==============
void Read_Dsiplay_DS18B20()
{
  unsigned char LSB,MSB;      
  unsigned int temp = 0;      
  
  init_ds18b20();              //��ʼ��DS18B20
  DisplaySMG_Temp();          //��̬ˢ�������    
  Write_DS18B20(0xcc);        //����ROM����
  Write_DS18B20(0x44);        //�����¶�ת��
  DisplaySMG_Temp();          //��̬ˢ�������
  init_ds18b20();              //��ʼ��DS18B20
  DisplaySMG_Temp();          //��̬ˢ�������
  Write_DS18B20(0xcc);        //����ROM����
  Write_DS18B20(0xbe);        //�����ڲ��洢��
  LSB = Read_DS18B20();        //��0�ֽڣ��¶ȵ�8λ
  MSB = Read_DS18B20();        //��1�ֽڣ��¶ȸ�8λ
  DisplaySMG_Temp();          //��̬ˢ�������
  //���������в���ദ�����ˢ�£���ʹ��ʾ���ȳ���
  temp = MSB;                  //�ϳ�16λ�¶�ԭʼ����
  temp = (temp << 8) | LSB;
  if((temp & 0xf800) == 0x0000)    //�������¶�
  {
    temp_ds18b20 = temp * 0.0625;  //����ʵ���¶�ֵ
  }
  DisplaySMG_Temp();          //��̬ˢ�������
}
//=================����ɨ���봦��==================
void Scan_Keys()
{
  if(S4 == 0)
  {
    DelaySMG(500);
    if(S4 == 0)
    {
      while(S4 == 0)          //�ȴ������ɿ�
      {
        DisplaySMG_Temp();
      }
      if(mode == 1)
      {
        mode = 2;              //�л�����1λС��ģʽ
      }
      else if(mode == 2)
      {
        mode = 3;              //�л���������ģʽ
      }
      else if(mode == 3)
      {
        mode = 1;              //�л�����2λС��ģʽ
      }
    }
  }
}
//===================ϵͳ��ʼ��===================
void Init_sys()
{
  Set_HC573(0, 0x00);        //�ر�����������
  Set_HC573(5, 0x00);        //�رշ������ͼ̵���
  Set_HC573(4, 0xff);        //�ر�ȫ��LED��
  DisplaySMG_All(0xff);      //�ر�ȫ�������
}
//=====================������=====================
void main()
{
  Init_sys();                //ϵͳ��ʼ��
  while(1)
  {
    Scan_Keys();            //ɨ�谴��
    Read_Dsiplay_DS18B20();  //ѭ�������¶Ȳ���ʾ
  }
}
