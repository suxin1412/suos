#include "OS.h"
#include "Snake.h"
#include "handle.h"
#include "indicator.h"
#include "Pellet.h"
 //定义信号量 实现屏幕的互斥共享 数量1
Semaphore screen = 1;
uint8_t signal = 0;
//此函数内仅可进行设备初始化和创建进程，禁止添加其他语句
void main_t()
{
	HandleInit();			//初始化手柄
	IndicatorInit();	//初始化屏幕
	Create_Process(UserProcess_Snake);	//创建贪吃蛇进程
	Create_Process(UserProcess_Pellet); //创建弹球进程
}
