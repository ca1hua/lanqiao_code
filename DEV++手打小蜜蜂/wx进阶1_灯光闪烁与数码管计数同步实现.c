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
	Display_Num();//在等待时继续扫描，防止动态显示不正确 
	}
}

void DelaySMG(unsigned int t){
	while(t--);
} 

//==================设置选通的锁存器===================
void Set_HC573(unsigned char channel, unsigned char dat)
{
  P2 = (P2 & 0x1f) | 0x00;       //赋值之前，关闭所有锁存器，防止冲突
  P0 = dat;                      //设置待赋值数据
  switch(channel)                //根据参数，选通锁存器，向目标赋值
  {
    case 4:
      P2 = (P2 & 0x1f) | 0x80;  //Y4输出0，LED控制
    break;
    case 5:
      P2 = (P2 & 0x1f) | 0xa0;  //Y5输出0，蜂鸣器和继电器控制
    break;
    case 6:
      P2 = (P2 & 0x1f) | 0xc0;  //Y6输出0，数码管位选
    break;
    case 7:
      P2 = (P2 & 0x1f) | 0xe0;  //Y7输出0，数码管段码
    break;
    case 0:
      P2 = (P2 & 0x1f) | 0x00;  //所有锁存器不选择
    break;
  }
  P2 = (P2 & 0x1f) | 0x00;      //赋值完成后，关闭所有锁存器
}

void DisplaySMG_Bit(unsigned char pos,unsigned  char value){
	Set_HC573(6,0x01 << pos);
	Set_HC573(7,value);
	
	DelaySMG(TSMG);
	
	Set_HC573(6,0x01 << pos);
	Set_HC573(7,0xff); // 消隐 
}

void DisplaySMG_All(unsigned char value) {
	Set_HC573(6, 0xff);    //使能所有数码管
    Set_HC573(7, value);   //数码管显示内容 
}

void Display_Num()
{
  //数码管右起第0位
  DisplaySMG_Bit(7,SMG_NoDot[num1  % 10]);
  //数码管右起第1位
  DisplaySMG_Bit(6,SMG_NoDot[num1 / 10]);    
  //数码管由起第7位
  DisplaySMG_Bit(0,SMG_NoDot[num2]);        
}

void LED_Control(){
	 stat_led &= ~0x80;         
  Set_HC573(4, stat_led);    //L8灯点亮
  Delay(200);                //延时
  stat_led |= 0x80;         
  Set_HC573(4, stat_led);    //L8灯熄灭
  Delay(200);                //延时
  
  num1++;                    //L8闪烁累加
  if(num1 == 100)            //达到最大值时恢复0
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
  
  num2++;                    //L1和L2翻转次数累加
  if(num2 == 10)             //达到最大值时恢复0
  {
    num2 = 0;
  }
}

void Init_sys()
{
  Set_HC573(0, 0x00);        //关闭所有锁存器
  Set_HC573(5, 0x00);        //关闭蜂鸣器和继电器
  Set_HC573(4, 0xff);        //关闭全部LED灯
  DisplaySMG_All(0xff);      //关闭全部数码管
}

void main(){
	Init_sys();  
	while(1){
	LED_Control();          //灯光闪烁
    Display_Num();          //数码管计数
	}
}
