/*==================蓝桥杯单片机特训==================
【进阶09】：DS18B20温度传感器数据采样与显示
**平  台：CT107D单片机综合实训平台
**模  式：IO模式
**底层驱动文件：2022年竞赛资源数据包提供的文件
**设  计：欧浩源（小蜜蜂老师，ohy3686@qq.com）
**时  间：2022-04-09
**更多详见：www.xmf393.com
====================================================*/

#include "reg52.h"
#include "onewire.h"

sbit S4 = P3 ^ 3;

// 定义动态显示中单个数码管点亮时长
#define TSMG 500
//-------共阳数码管的段码编码表（无小数点）--------
unsigned char code SMG_NoDot[18] = {0xc0, 0xf9,
									0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90,
									0x88, 0x80, 0xc6, 0xc0, 0x86, 0x8e, 0xbf, 0x7f};
//-------共阳数码管的段码编码表（带小数点）--------
// 0.～9.
unsigned char code SMG_Dot[10] = {0x40, 0x79,
								  0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10};
//----------------------------------------------

float temp_ds18b20 = 0;		  // 实际温度值
unsigned int smg_ds18b20 = 0; // 数码管显示温度值
unsigned char mode = 1;		  // 显示模式

//==============数码管动态显示专用延时函数=============
void DelaySMG(unsigned int t)
{
	while (t--)
		;
}
//==================设置选通的锁存器===================
void Set_HC573(unsigned char channel, unsigned char dat)
{
	P0 = dat;		 // 待设置数据
	switch (channel) // 选通锁存器
	{
	case 4:
		P2 = (P2 & 0x1f) | 0x80; // Y4输出0，LED控制
		break;
	case 5:
		P2 = (P2 & 0x1f) | 0xa0; // Y5输出0，蜂鸣器和继电器控制
		break;
	case 6:
		P2 = (P2 & 0x1f) | 0xc0; // Y6输出0，数码管位选
		break;
	case 7:
		P2 = (P2 & 0x1f) | 0xe0; // Y7输出0，数码管段码
		break;
	case 0:
		P2 = (P2 & 0x1f) | 0x00; // 所有锁存器不选择
		break;
	}
	P2 = (P2 & 0x1f) | 0x00; // 设置完成，关闭全部锁存器
}
//=================单个数码管动态显示=================
void DisplaySMG_Bit(unsigned char pos, unsigned char value)
{
	Set_HC573(6, 0x01 << pos); // 数码管的段位
	Set_HC573(7, value);	   // 数码管显示内容
	DelaySMG(TSMG);			   // 数码管点亮时长
	Set_HC573(6, 0x01 << pos);
	Set_HC573(7, 0xff); // 消隐
}
//=================全部数码管静态显示=================
void DisplaySMG_All(unsigned char value)
{
	Set_HC573(6, 0xff);	 // 使能所有数码管
	Set_HC573(7, value); // 数码管显示内容
}
//=================温度值显示函数=================
void DisplaySMG_Temp()
{
	switch (mode)
	{
	// 2位小数显示模式
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
	// 1位小数显示模式
	case 2:
		smg_ds18b20 = temp_ds18b20 * 10;
		DisplaySMG_Bit(7, SMG_NoDot[smg_ds18b20 % 10]);
		DisplaySMG_Bit(6, SMG_Dot[(smg_ds18b20 / 10) % 10]);
		if (smg_ds18b20 / 100 != 0)
		{
			DisplaySMG_Bit(5, SMG_NoDot[smg_ds18b20 / 100]);
		}
		break;
	// 整数显示模式
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

void Read_Dsiplay_DS18B20()
{
	unsigned char LSB, MSB;
	unsigned int temp = 0;

	init_ds18b20();		  // 初始化DS18B20
	DisplaySMG_Temp();	  // 动态刷新数码管
	Write_DS18B20(0xcc);  // 忽略ROM操作
	Write_DS18B20(0x44);  // 启动温度转换
	DisplaySMG_Temp();	  // 动态刷新数码管
	init_ds18b20();		  // 初始化DS18B20
	DisplaySMG_Temp();	  // 动态刷新数码管
	Write_DS18B20(0xcc);  // 忽略ROM操作
	Write_DS18B20(0xbe);  // 读出内部存储器
	LSB = Read_DS18B20(); // 第0字节：温度低8位
	MSB = Read_DS18B20(); // 第1字节：温度高8位
	DisplaySMG_Temp();	  // 动态刷新数码管
	// 上述程序中插入多处数码管刷新，可使显示亮度充足
	temp = MSB; // 合成16位温度原始数据
	temp = (temp << 8) | LSB;
	if ((temp & 0xf800) == 0x0000) // 处理正温度
	{
		temp_ds18b20 = temp * 0.0625; // 计算实际温度值
	}
	DisplaySMG_Temp(); // 动态刷新数码管
}
//=================按键扫描与处理==================
void Scan_Keys()
{
	if (S4 == 0)
	{
		DelaySMG(500);
		if (S4 == 0)
		{
			while (S4 == 0) // 等待按键松开
			{
				DisplaySMG_Temp();
			}
			if (mode == 1)
			{
				mode = 2; // 切换至：1位小数模式
			}
			else if (mode == 2)
			{
				mode = 3; // 切换至：整数模式
			}
			else if (mode == 3)
			{
				mode = 1; // 切换至：2位小数模式
			}
		}
	}
}
//===================系统初始化===================
void Init_sys()
{
	Set_HC573(0, 0x00);	  // 关闭所有锁存器
	Set_HC573(5, 0x00);	  // 关闭蜂鸣器和继电器
	Set_HC573(4, 0xff);	  // 关闭全部LED灯
	DisplaySMG_All(0xff); // 关闭全部数码管
}
//=====================主函数=====================
void main()
{
	Init_sys(); // 系统初始化
	while (1)
	{
		Scan_Keys();			// 扫描按键
		Read_Dsiplay_DS18B20(); // 循环采样温度并显示
	}
}