#include "reg52.h"

#define TSMG 500
//
unsigned char code SMG_NoDot[18]={0xc0,0xf9,
    0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
    0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f};

unsigned char stat_led = 0xff;
unsigned char num1 = 0,num2 = 0;
void Display_Num();

void Delay(unsigned int t)
{
	while(t--){
	Display_Num();//�ڵȴ�ʱ����ɨ�裬��ֹ��̬��ʾ����ȷ 
	}
}

void DelaySMG(unsigned int t){
	while(t--);
} 

//==================����ѡͨ��������===================
void Set_HC573(unsigned char channel, unsigned char dat)
{
  P2 = (P2 & 0x1f) | 0x00;       //��ֵ֮ǰ���ر���������������ֹ��ͻ
  P0 = dat;                      //���ô���ֵ����
  switch(channel)                //���ݲ�����ѡͨ����������Ŀ�긳ֵ
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
  P2 = (P2 & 0x1f) | 0x00;      //��ֵ��ɺ󣬹ر�����������
}

void DisplaySMG_Bit(unsigned char pos,unsigned  char value){
	Set_HC573(6,0x01 << pos);
	Set_HC573(7,value);
	
	DelaySMG(TSMG);
	
	Set_HC573(6,0x01 << pos);
	Set_HC573(7,0xff); // ���� 
}

void DisplaySMG_All(unsigned char value) {
	Set_HC573(6, 0xff);    //ʹ�����������
    Set_HC573(7, value);   //�������ʾ���� 
}

void Display_Num()
{
  //����������0λ
  DisplaySMG_Bit(7,SMG_NoDot[num1  % 10]);
  //����������1λ
  DisplaySMG_Bit(6,SMG_NoDot[num1 / 10]);    
  //����������7λ
  DisplaySMG_Bit(0,SMG_NoDot[num2]);        
}

void LED_Control(){
	 stat_led &= ~0x80;         
  Set_HC573(4, stat_led);    //L8�Ƶ���
  Delay(200);                //��ʱ
  stat_led |= 0x80;         
  Set_HC573(4, stat_led);    //L8��Ϩ��
  Delay(200);                //��ʱ
  
  num1++;                    //L8��˸�ۼ�
  if(num1 == 100)            //�ﵽ���ֵʱ�ָ�0
  {
    num1 = 0;
  }
  if((stat_led & 0x03) == 0x03)
  {
    stat_led &= ~0x03;
  }
  else
  {
    stat_led |= 0x03;
  }
  Set_HC573(4, stat_led);
  
  num2++;                    //L1��L2��ת�����ۼ�
  if(num2 == 10)             //�ﵽ���ֵʱ�ָ�0
  {
    num2 = 0;
  }
}

void Init_sys()
{
  Set_HC573(0, 0x00);        //�ر�����������
  Set_HC573(5, 0x00);        //�رշ������ͼ̵���
  Set_HC573(4, 0xff);        //�ر�ȫ��LED��
  DisplaySMG_All(0xff);      //�ر�ȫ�������
}

void main(){
	Init_sys();  
	while(1){
	LED_Control();          //�ƹ���˸
    Display_Num();          //����ܼ���
	}
}
