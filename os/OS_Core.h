/* ！用户程序禁止包含该头文件 */
#ifndef _CORE_H_
#define _CORE_H_

#include <OS_TypeDefine.h>

//	定义最大进程数量
#define PCB_MaxNumber 10

//	定义栈空间大小
#define StackSize 10

//	堆空间大小
#define HeapSize 2000

/*	PCB结构定义
*		阻塞事件的选项：
*						BlockEvent <= 60000 时
*									进程为延时阻塞，每次刷新进程表时执行 --BlockEvent
*									BlockEvent == 0 时 进程自动唤醒
***/
struct PCB_Struct
{
	uint8_t ProcessStack[StackSize];											//进程栈，保存进程断点
	enum {Ready=1,Block=2,Running=3} ProcessState;				//进程状态
	uint8_t PID;																					//进程标识符
	uint8_t PSW,R0,R1,R2,R3,R4,R5,R6,R7,ACC,B,DPH,DPL,SP; //进程现场
	void (*Function)(void);																//进程函数入口
	uint16_t BlockEvent;																	//阻塞事件
	Semaphore *s;
};


//	声明栈空间
extern uint8_t idata SystemStack[StackSize]; 	//系统栈，系统进程使用
extern uint8_t idata UserStack[StackSize];		//用户进程栈，用户进程使用,中转用

//	申明开辟CPU现场保存到进程PCB的中转站
extern uint8_t  PSW_Backups;
extern uint8_t  R0_Backups;
extern uint8_t  R1_Backups;
extern uint8_t  R2_Backups;
extern uint8_t  R3_Backups;
extern uint8_t  R4_Backups;
extern uint8_t  R5_Backups;
extern uint8_t  R6_Backups;
extern uint8_t  R7_Backups;
extern uint8_t  ACC_Backups;
extern uint8_t  B_Backups;
extern uint8_t  SP_Backups;
extern uint8_t  DPH_Backups;
extern uint8_t  DPL_Backups;

//函数申明 系统函数，用户不可调用
void Core_InitSystem(void);
void SystemProcess(void);
void Switch_Process(void);
void Control_Process(void);
void Refresh_Process(void);
void BlockSwitch_Process(void);//目前无用


#endif
