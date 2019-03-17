#ifndef _SUOS_H_
#define _SUOS_H_
#include <OS_TypeDefine.h>
#include <STC15F2K60S2.H>

extern uint8_t xdata temp;
void main_t(void);															//在该函数中创建用户进程
int8_t Create_Process(void (*Function)(void)); 	//使用此函数创建用户进程
STATUS DelayBlock_Proccess(uint16_t time);			//将当前进程阻塞time毫秒，误差较大
void SemaphoreWait(Semaphore *s);
void SemaphoreSignal(Semaphore *s);
#endif