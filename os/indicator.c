#include "indicator.h"
#include <STC15F2K60S2.H>
bit UartBusy = 0;
void IndicatorInit(void)		//115200bps@24MHz
{
	SCON = 0x50;		
	AUXR |= 0x01;		
	AUXR |= 0x04;		
	T2L = 0xCC;		
	T2H = 0xFF;		
	AUXR |= 0x10;		
	ES = 1;
}
void UartTI()
{
	if (TI)
	{
		TI = 0;
		UartBusy = 0;
	}
}
void UartSendBYTE(unsigned char Data) 
{
	while (UartBusy);
	SBUF = Data;
	UartBusy = 1;
}
void UartSendString(unsigned char *Str) 
{
	while (*Str)
	{
		UartSendBYTE(*Str);
		Str++;
	}
}

void UartSendBYTE_(unsigned char Data)
{
	while (UartBusy);
	SBUF = Data;
	UartBusy = 1;
}
void UartSendString_(unsigned char *Str)
{
	while (*Str)
	{
		UartSendBYTE_(*Str);
		Str++;
	}
}
void DisplaySendCommand(uint8_t *Str)
{
	UartSendString(Str);
	UartSendBYTE(0xff);
	UartSendBYTE(0xff);
	UartSendBYTE(0xff);
}
void DisplaySendEnd()
{
	UartSendBYTE(0xff);
	UartSendBYTE(0xff);
	UartSendBYTE(0xff);
}
void DisplaySendEnd_()
{
	UartSendBYTE_(0xff);
	UartSendBYTE_(0xff);
	UartSendBYTE_(0xff);
}

