#include "reg52.h"
#include "iic.h"
sbit S4 = P3^3;
sbit S5 = P3^2;
sbit S6 = P3^1;
//定义动态显示中单个数码管点亮时长
#define TSMG  500
//-------共阳数码管的段码编码表（无小数点）--------
//0 1 2 3 4 5 6 7 8 9 A B C D E F - .
unsigned char code SMG_NoDot[18]= {
	0xc0,0xf9,
	0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
	0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f
};
//-------共阳数码管的段码编码表（带小数点）--------
//0. 1. 2. 3. 4. 5. 6. 7. 8. 9.
unsigned char code SMG_Dot[10]= {
	0x40,0x79,
	0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10
};
//-----------------------------------------------
unsigned int adc_value = 0;      //AIN3的采样数据
float adc_volt = 0;              //AIN3的换算电压
unsigned int smg_volt = 0;      //AIN3的显示电压
unsigned int param = 300;        //电压参数，3.00V
unsigned char stat_smg = 1;      //当前显示界面标志
unsigned char num = 0;          //报警计数变量
unsigned int count_t = 0;        //50ms单位累计变量
unsigned char stat_led = 0xff;  //LED灯控制状态
unsigned char F_shan = 0;        //秒闪标志
unsigned char F_time = 0;        //计时标志

//==============数码管动态显示专用延时函数=============
void DelaySMG(unsigned int t) {
	while(t--);
}
//==================设置选通的锁存器===================
void Set_HC573(unsigned char channel, unsigned char dat) {
	P0 = dat;                      //待设置数据
	switch(channel) {              //选通锁存器
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
	P2 = (P2 & 0x1f) | 0x00;      //设置完成，关闭全部锁存器
}
//=================单个数码管动态显示=================
void DisplaySMG_Bit(unsigned char pos, unsigned char value) {
	Set_HC573(6, 0x01 << pos);    //数码管的段位
	Set_HC573(7, value);          //数码管显示内容
	DelaySMG(TSMG);                //数码管点亮时长
	Set_HC573(6, 0x01 << pos);
	Set_HC573(7, 0xff);            //消隐
}
//=================全部数码管静态显示=================
void DisplaySMG_All(unsigned char value) {
	Set_HC573(6, 0xff);    //使能所有数码管
	Set_HC573(7, value);  //数码管显示内容
}
//=================系统信息数码管显示=================
void DisplaySMG_Info() {
	switch(stat_smg) {
			//电压数据界面
		case 1:
			DisplaySMG_Bit(7,SMG_NoDot[smg_volt  % 10]);
			DisplaySMG_Bit(6,SMG_NoDot[(smg_volt / 10) % 10]);
			DisplaySMG_Bit(5,SMG_Dot[smg_volt / 100]);
			//提示符：U---->0xC1
			DisplaySMG_Bit(0,0xc1);
			break;
			//参数设置界面
		case 2:
			DisplaySMG_Bit(7,SMG_NoDot[param  % 10]);
			DisplaySMG_Bit(6,SMG_NoDot[(param / 10) % 10]);
			DisplaySMG_Bit(5,SMG_Dot[param / 100]);
			//提示符：P---->0x8C
			DisplaySMG_Bit(0,0x8c);
			break;
			//报警计数界面
		case 3:
			DisplaySMG_Bit(7,SMG_NoDot[num  % 10]);
			if(num / 10 != 0) {
				DisplaySMG_Bit(6,SMG_NoDot[num / 10]);
			}
			//提示符：S---->0x92
			DisplaySMG_Bit(0,0x92);
			break;

	}
}
//===================定时器T0初始化===================
void Init_Timer0() {
	//定时器计数初值设置：50ms
	TH0 = (0 - 50000) / 256;
	TL0 = (0 - 50000) % 256;
	TMOD = 0x01;      //T0的模式1：16位不可重装模式
	ET0 = 1;          //使能定时器0中断
	EA = 1;           //打开总中断
	TR0 = 1;          //启动定时器0
}
//===============定时器T0的中断服务函数================
void Service_Timer0() interrupt 1 {
	TH0 = (0 - 50000) / 256;
	TL0 = (0 - 50000) % 256;

	if(F_time == 1) {            //启动报警计时
		count_t++;                //50ms单位定时累计

		if(count_t % 20 == 0) {    //1秒时长
			if(num != 99) {          //没有到达最大值99前
				num++;                //报警计数累计
			}
			if(F_shan == 1) {        //启动秒闪功能
				if((stat_led & 0x80) == 0x80) {
					stat_led &= ~0x80;  //点亮L8灯
				} else {
					stat_led |= 0x80;    //熄灭L8灯
				}
			}
		}

		if(count_t > 120) {        //超过6秒
			stat_led &= ~0x07;      //点亮L1 L2 L3
			F_shan = 1;
		} else if(count_t > 80) { //超过4秒
			stat_led &= ~0x03;      //点亮L1 L2
		} else if(count_t > 40) {  //超过2秒
			stat_led &= ~0x01;      //点亮L1
		} else {
			stat_led = 0xff;
		}
	}
}
//===============PCF8591电压采样处理函数===============
unsigned char Read_PCF8591_AIN3() {
	unsigned char tmp;
	IIC_Start();
	IIC_SendByte(0x90);      //PCF8591的写设备地址
	IIC_WaitAck();
	//选择AD转换通道
	IIC_SendByte(0x03);     //AIN3通道，可调电阻电压
	IIC_WaitAck();
	IIC_Stop();
	DisplaySMG_Info();      //等待电压转换完成
	IIC_Start();
	IIC_SendByte(0x91);     //PCF8591的读设备地址
	IIC_WaitAck();
	tmp = IIC_RecByte();
	IIC_SendAck(1);           //产生非应答信号
	IIC_Stop();
	return tmp;
}
//=================电压数据应用分析处理=================
void App_Volt() {
	unsigned char i;
	//采样通道3的电压，并做简单的均值滤波处理
	adc_value = 0;
	for(i = 0; i < 3; i++) {
		adc_value += Read_PCF8591_AIN3();
	}
	adc_value = adc_value / 3;
	//将ADC采样到的数据换算成对应的电压值
	adc_volt = adc_value * (5.0 / 255);
	smg_volt = adc_volt * 100;
	//当前电压与电压参数的比较处理
	if(smg_volt < param) {        //当前电压 < 电压参数
		if(F_time == 0) {
			F_time = 1;                //标志计算持续时间
		}
	} else {                      //当前电压 >= 电压参数
		F_time = 0;                  //恢复待计时状态
		F_shan = 0;
		count_t = 0;                //计时变量清0
		num = 0;
		stat_led |= 0xff;            //指示灯L1熄灭
	}
	DisplaySMG_Info();            //动态刷新数码管
	Set_HC573(4, stat_led);        //更新LED灯控制状态
}
//===================按键扫描处理函数==================
void Scan_Keys() {
	if(S4 == 0) {                  //切换界面
		DelaySMG(500);
		if(S4 == 0) {
			if(stat_smg == 1) {
				stat_smg = 2;            //切换至：参数设置
			} else if(stat_smg == 2) {
				stat_smg = 3;            //切换至：报警计数
			} else if(stat_smg == 3) {
				stat_smg = 1;            //切换至：电压数据
			}
			while(S4 == 0) {          //松手检测
				App_Volt();              //保持电压采样与显示
			}
		}
	}

	if(S5 == 0) {                  //电压参数减小
		DelaySMG(500);
		if(S5 == 0) {
			if(stat_smg == 2) {        //仅在参数设置中有效
				if(param != 0) {        //未减小到最小值
					param = param - 50;    //每按1下，减小0.5V
				} else {
					param = 500;          //边界处理，恢复5.00V
				}
			}
			while(S5 == 0) {
				App_Volt();
			}
		}
	}

	if(S6 == 0) {                  //电压参数增加
		DelaySMG(500);
		if(S6 == 0) {
			if(stat_smg == 2) {        //仅在参数设置中有效
				if(param != 500) {      //未增加到最大值
					param = param + 50;    //每按1下，增加0.5V
				} else {
					param = 0;            //边界处理，恢复0.00V
				}
			}
			while(S6 == 0) {
				App_Volt();
			}
		}
	}
}
//=====================系统初始化=====================
void Init_sys() {
	Set_HC573(0, 0x00);        //关闭所有锁存器
	Set_HC573(5, 0x00);        //关闭蜂鸣器和继电器
	Set_HC573(4, 0xff);        //关闭全部LED灯
	DisplaySMG_All(0xff);      //关闭全部数码管
	Init_Timer0();             //初始化并启动定时器0
}
//=======================主函数=======================
void main() {
	Init_sys();                //系统初始化
	while(1) {
		App_Volt();              //采样电压并处理
		Scan_Keys();             //循环扫描按键
	}
}
