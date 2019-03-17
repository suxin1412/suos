#include <STC15F2K60S2.H>
#include "intrins.h"
#include <os.H>
#include <handle.h>

void HandleInit()
{
	P1ASF = 0X07; 		//	打开ADC端口 p1.0 p1.1 p1.2
	ADC_CONTR = 0XE0; //	ADC上电，速度最高，清除FLAG
	CLK_DIV &=0xDF;		//	高8位放ADC_RES
}

unsigned char HandleReadX()  //p1.1
{
	ADC_CONTR = 0xE9;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	while(!(ADC_CONTR&0x10));
	ADC_CONTR &= 0xef;
	return ADC_RES;
}
unsigned char HandleReadY() //p1.0
{
	ADC_CONTR = 0xE8;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	while(!(ADC_CONTR&0x10));
	ADC_CONTR &= 0xef;
	return ADC_RES;
}
//unsigned char Random(uint8_t base) //p1.2
//{
//	uint8_t Rand;
//	ADC_CONTR = 0xEA;
//	_nop_();
//	_nop_();
//	_nop_();
//	_nop_();
//	while(!(ADC_CONTR&0x10));
//	ADC_CONTR &= 0xef;
//	Rand = (ADC_RESL&0x03)+(ADC_RES<<2);
//	Rand %= base;
//	return Rand;
//}