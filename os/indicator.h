#ifndef _INDICATOR_H_
#define _INDICATOR_H_
#include <OS.h>
void IndicatorInit(void);		//115200bps@24MHz
void DisplaySendCommand(uint8_t *Str);
#define DisplaySendString UartSendString
#define DisplaySendString_ UartSendString_
void DisplaySendEnd();
void DisplaySendEnd_();

void UartSendString_(unsigned char *Str);

void UartSendString(unsigned char *Str);
#endif