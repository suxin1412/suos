#include <STC15F2K60S2.H>
#include <OS_Core.h>
#include <OS.h>
#include <stdlib.h>

//	开辟栈空间
uint8_t idata SystemStack[StackSize]; //系统栈，系统进程使用
uint8_t idata UserStack[StackSize];		//用户进程栈，用户进程使用,中转用

//	开辟堆空间
uint8_t xdata HeapMemory[HeapSize];

//	开辟CPU现场保存到进程PCB的中转站
uint8_t  PSW_Backups;
uint8_t  R0_Backups;
uint8_t  R1_Backups;
uint8_t  R2_Backups;
uint8_t  R3_Backups;
uint8_t  R4_Backups;
uint8_t  R5_Backups;
uint8_t  R6_Backups;
uint8_t  R7_Backups;
uint8_t  ACC_Backups;
uint8_t  B_Backups;
uint8_t  SP_Backups;
uint8_t  DPH_Backups;
uint8_t  DPL_Backups;

//系统PCB索引表
struct PCB_Struct *PCB_IndexTable[PCB_MaxNumber];
uint8_t PCB_Number  = 0;	//PCB数量 = PCB_Number + 1
uint8_t	PCB_Current = 0;	//当前进程PID

uint8_t xdata temp = 0;

void main()
{
	Core_InitSystem();
	Create_Process(SystemProcess);
	PCB_IndexTable[0]->SP = (uint8_t)UserStack-1;//系统进程直接开始运行
	PCB_IndexTable[0]->ProcessState = Running;
	PCB_IndexTable[0]->Function();
	while (1); //多余
}

/**	
*		功能：初始化系统
*
*/
void Core_InitSystem(void)
{
	//	初始化堆空间 位于内部扩展RAM 从0x00地址开始，400Byte
	init_mempool(HeapMemory,HeapSize);
	//	初始化定时器 2ms中断一次，即时间片为2ms
	AUXR |= 0x80;			//1T 模式
	TMOD &= 0xF0;			//16位自动重载
	TL0 = 0x80;				//初值装载
	TH0 = 0x44;				
	TF0 = 0;					//标志清除
	TR0 = 1;					//开启定时
	ET0 = 1;
	//	初始化SPI中断，SPI接口不被使用，该中断被用于用户态切换核心态
	//SPCTL = SPCTL & 0XBF; //关闭spi
	//IE2 = IE2 | 0X02;	// OPEN SPI INTERRUPT
	
	EA  = 1;					//开中断
}

/**
*		功能：创建新进程
*		参数：进程入口地址
*		返回值：进程的PID，-1表示创建失败
*/
int8_t Create_Process(void (*Function)(void))
{
	if (PCB_Number == PCB_MaxNumber-1) return -1; //进程数量已经满了
	PCB_IndexTable[PCB_Number] = (struct PCB_Struct*)malloc(sizeof(struct PCB_Struct)); //为进程申请PCB 挂在PCB索引表上
	PCB_IndexTable[PCB_Number]->Function = Function;		//保存进程入口地址
	PCB_IndexTable[PCB_Number]->ProcessState = Ready;		//设置进程为就绪态
	PCB_IndexTable[PCB_Number]->BlockEvent = 0;					//无阻塞事件
	PCB_IndexTable[PCB_Number]->s = SemaphoreNull;			//没有信号量阻塞
	PCB_IndexTable[PCB_Number]->PID = PCB_Number;				//进程标识符设置
	PCB_IndexTable[PCB_Number]->SP = (uint8_t)UserStack+1;//进程栈指针，初始时栈中存放函数入口地址
	PCB_IndexTable[PCB_Number]->ProcessStack[0] = (uint16_t)Function & 0x00ff;	//函数入口地址，低8位
	PCB_IndexTable[PCB_Number]->ProcessStack[1] = ((uint16_t)Function>>8) & 0x00ff;	//高8位
	PCB_IndexTable[PCB_Number]->PSW = 0;
	PCB_IndexTable[PCB_Number]->ACC = 0;
	PCB_IndexTable[PCB_Number]->B   = 0;
	PCB_IndexTable[PCB_Number]->R0  = 0;
	PCB_IndexTable[PCB_Number]->R1  = 0;
	PCB_IndexTable[PCB_Number]->R2  = 0;
	PCB_IndexTable[PCB_Number]->R3  = 0;
	PCB_IndexTable[PCB_Number]->R4  = 0;
	PCB_IndexTable[PCB_Number]->R5  = 0;
	PCB_IndexTable[PCB_Number]->R6  = 0;
	PCB_IndexTable[PCB_Number]->R7  = 0;
	PCB_IndexTable[PCB_Number]->DPL = 0;
	PCB_IndexTable[PCB_Number]->DPH = 0;
	PCB_Number++;
	return (int8_t)PCB_IndexTable[PCB_Number]->PID;
}

/**
*		功能：进程控制函数
					扫描进程队列，释放已终止进程的PCB空间，唤醒符合条件的阻塞进程
*					调用进程切换功能
*		参数：无
*		返回值：无
*/
void Control_Process(void)
{
	EA = 0;
	Refresh_Process();
	Switch_Process();
	EA = 1;
}

/**
*		功能：进程切换函数 由定时器0中断服务函数调用 实现时间片轮转
*		参数：无
*		返回值：无
*/
void Switch_Process(void)
{
	uint8_t i;
	//	0.判断是否需要切换进程：若没有处于就绪态的进程则无需切换
	bit Unwanted = 1;			//	不需要切换进程标志 为1时不切换
	for (i = 0;i<=PCB_Number;i++) 
		if (PCB_IndexTable[i]->ProcessState == Ready)
			Unwanted = 0;
	if (Unwanted) return;
	//	1.将用户栈空间复制到自己的PCB,腾出来给下一个进程
	for (i=0;i<StackSize;i++)
		PCB_IndexTable[PCB_Current]->ProcessStack[i] = UserStack[i];
	// 	2.将CPU环境复制到PCB,腾出来给下一个进程
	PCB_IndexTable[PCB_Current]->PSW = PSW_Backups;
	PCB_IndexTable[PCB_Current]->ACC = ACC_Backups;
	PCB_IndexTable[PCB_Current]->B   = B_Backups;
	PCB_IndexTable[PCB_Current]->R0  = R0_Backups;
	PCB_IndexTable[PCB_Current]->R1  = R1_Backups;
	PCB_IndexTable[PCB_Current]->R2  = R2_Backups;
	PCB_IndexTable[PCB_Current]->R3  = R3_Backups;
	PCB_IndexTable[PCB_Current]->R4  = R4_Backups;
	PCB_IndexTable[PCB_Current]->R5  = R5_Backups;
	PCB_IndexTable[PCB_Current]->R6  = R6_Backups;
	PCB_IndexTable[PCB_Current]->R7  = R7_Backups;
	PCB_IndexTable[PCB_Current]->DPL = DPL_Backups;
	PCB_IndexTable[PCB_Current]->DPH = DPH_Backups;
	PCB_IndexTable[PCB_Current]->SP  = SP_Backups;
	//  3.选择下一个就绪的进程并切换
	//
	i = PCB_Current;
	while(1)   /*所有进程都阻塞了这么办？*/
	{
		if (i==PCB_Number) 
			i=0;																				//	循环查找
		if (PCB_IndexTable[i]->ProcessState == Ready)	
			break; 																			//	找到下一个就绪的进程了，退出寻找
		i++;																		
	} 
	if (PCB_IndexTable[PCB_Current]->ProcessState == Running) //如果当前进程为运行态
		PCB_IndexTable[PCB_Current]->ProcessState = Ready;	//当前进程设置为就绪
	PCB_Current = i;
	PCB_IndexTable[PCB_Current]->ProcessState = Running;//找到的进程设置为运行状态
	//	4.将找到的进程CPU的现场信息复制到中转站
	PSW_Backups = PCB_IndexTable[PCB_Current]->PSW;
	ACC_Backups = PCB_IndexTable[PCB_Current]->ACC;
	B_Backups   = PCB_IndexTable[PCB_Current]->B;
	R0_Backups  = PCB_IndexTable[PCB_Current]->R0;
	R1_Backups  = PCB_IndexTable[PCB_Current]->R1;
	R2_Backups  = PCB_IndexTable[PCB_Current]->R2;
	R3_Backups  = PCB_IndexTable[PCB_Current]->R3;
	R4_Backups  = PCB_IndexTable[PCB_Current]->R4;
	R5_Backups  = PCB_IndexTable[PCB_Current]->R5;
	R6_Backups  = PCB_IndexTable[PCB_Current]->R6;
	R7_Backups  = PCB_IndexTable[PCB_Current]->R7;
	DPL_Backups = PCB_IndexTable[PCB_Current]->DPL;
	DPH_Backups = PCB_IndexTable[PCB_Current]->DPH;
	SP_Backups  = PCB_IndexTable[PCB_Current]->SP;
	//	5.将栈空间从PCB复制到用户栈
	for (i=0;i<StackSize;i++)
		UserStack[i] = PCB_IndexTable[PCB_Current]->ProcessStack[i];
}

/**
*		功能：阻塞进程切换函数 由用户进程将自己阻塞时切换进程
*		参数：无
*		返回值：无
*/
void BlockSwitch_Process(void)
{
	uint8_t i;
	//	0.判断是否需要切换进程：若没有处于就绪态的进程则无需切换
	bit Unwanted = 1;			//	不需要切换进程标志 为1时不切换
	for (i = 0;i<=PCB_Number;i++) 
		if (PCB_IndexTable[i]->ProcessState == Ready)
			Unwanted = 0;
	if (Unwanted) return;
	//	1.将用户栈空间复制到自己的PCB,腾出来给下一个进程
	for (i=0;i<StackSize;i++)
		PCB_IndexTable[PCB_Current]->ProcessStack[i] = UserStack[i];
	// 	2.将CPU环境复制到PCB,腾出来给下一个进程
	PCB_IndexTable[PCB_Current]->PSW = PSW_Backups;
	PCB_IndexTable[PCB_Current]->ACC = ACC_Backups;
	PCB_IndexTable[PCB_Current]->B   = B_Backups;
	PCB_IndexTable[PCB_Current]->R0  = R0_Backups;
	PCB_IndexTable[PCB_Current]->R1  = R1_Backups;
	PCB_IndexTable[PCB_Current]->R2  = R2_Backups;
	PCB_IndexTable[PCB_Current]->R3  = R3_Backups;
	PCB_IndexTable[PCB_Current]->R4  = R4_Backups;
	PCB_IndexTable[PCB_Current]->R5  = R5_Backups;
	PCB_IndexTable[PCB_Current]->R6  = R6_Backups;
	PCB_IndexTable[PCB_Current]->R7  = R7_Backups;
	PCB_IndexTable[PCB_Current]->DPL = DPL_Backups;
	PCB_IndexTable[PCB_Current]->DPH = DPH_Backups;
	PCB_IndexTable[PCB_Current]->SP  = SP_Backups;
	//  3.选择下一个就绪的进程并切换
	//
	i = PCB_Current;
	while(1)   /*所有进程都阻塞了这么办？*/
	{
		if (i==PCB_Number) 
			i=0;																				//	循环查找
		if (PCB_IndexTable[i]->ProcessState == Ready)	
			break; 																			//	找到下一个就绪的进程了，退出寻找
		i++;																		
	} 
	if (PCB_IndexTable[PCB_Current]->ProcessState == Running) //如果当前进程为运行态
		PCB_IndexTable[PCB_Current]->ProcessState = Ready;	//当前进程设置为就绪
	PCB_Current = i;
	PCB_IndexTable[PCB_Current]->ProcessState = Running;//找到的进程设置为运行状态
	//	4.将找到的进程CPU的现场信息复制到中转站
	PSW_Backups = PCB_IndexTable[PCB_Current]->PSW;
	ACC_Backups = PCB_IndexTable[PCB_Current]->ACC;
	B_Backups   = PCB_IndexTable[PCB_Current]->B;
	R0_Backups  = PCB_IndexTable[PCB_Current]->R0;
	R1_Backups  = PCB_IndexTable[PCB_Current]->R1;
	R2_Backups  = PCB_IndexTable[PCB_Current]->R2;
	R3_Backups  = PCB_IndexTable[PCB_Current]->R3;
	R4_Backups  = PCB_IndexTable[PCB_Current]->R4;
	R5_Backups  = PCB_IndexTable[PCB_Current]->R5;
	R6_Backups  = PCB_IndexTable[PCB_Current]->R6;
	R7_Backups  = PCB_IndexTable[PCB_Current]->R7;
	DPL_Backups = PCB_IndexTable[PCB_Current]->DPL;
	DPH_Backups = PCB_IndexTable[PCB_Current]->DPH;
	SP_Backups  = PCB_IndexTable[PCB_Current]->SP;
	//	5.将栈空间从PCB复制到用户栈
	for (i=0;i<StackSize;i++)
		UserStack[i] = PCB_IndexTable[PCB_Current]->ProcessStack[i];
}
/**
*		功能：运行进程
*		参数：无
*		返回值：无
*/
//void Run_Process(uint8_t PCB_ID)
//{
//	
//}

/**
*		功能：更新进程状态 
*					主要是检测阻塞进程所等待的事件是否发生了
*		参数：无
*		返回值：无
*/
void Refresh_Process(void)
{
	uint8_t i;
	for (i = 0;i<=PCB_Number;i++)
	{
		if (PCB_IndexTable[i]!=(void*)0 && PCB_IndexTable[i]->ProcessState == Block) ////PCB非空 进程被阻塞
			{
				///*信号量的阻塞唤醒设计有问题*/
				if (PCB_IndexTable[i]->BlockEvent == 0 && PCB_IndexTable[i]->s == SemaphoreNull)
					PCB_IndexTable[i]->ProcessState = Ready;
				else if (PCB_IndexTable[i]->BlockEvent>0)
				{
					--PCB_IndexTable[i]->BlockEvent;
					if (PCB_IndexTable[i]->BlockEvent == 0) PCB_IndexTable[i]->ProcessState = Ready;
				}
					/*
				if (PCB_IndexTable[i]->BlockEvent==0&&*(PCB_IndexTable[i]->s)>0)//计时器为0且信号量>0 唤醒
					{
						PCB_IndexTable[i]->ProcessState = Ready;
						continue;
					}
				if (PCB_IndexTable[i]->BlockEvent<=60000) 
					--PCB_IndexTable[i]->BlockEvent;
				else ;//待添加的阻塞检查事件
				if (PCB_IndexTable[i]->BlockEvent==0&&*(PCB_IndexTable[i]->s)>0)//计时器为0且信号量>0 唤醒
					{PCB_IndexTable[i]->ProcessState = Ready;continue;}
				*/
			}//if end
	}//for end
}

/**
*		功能：阻塞进程，延时time毫秒后唤醒
*		参数：time 需要延时的时长，ms为单位，取值范围：2-120000 
*		返回值：调用成功SUCCESS或失败FAIL
*/
STATUS DelayBlock_Proccess(uint16_t time)
{
	if (time>60000 || time < 2) return FAIL;
	PCB_IndexTable[PCB_Current]->BlockEvent = time / 2;
	PCB_IndexTable[PCB_Current]->ProcessState = Block;
	//切换进程	暂时没实现
	//目前先盲等	等时间片耗完自动切换
	while (PCB_IndexTable[PCB_Current]->ProcessState==Block);
	return SUCCESS;
}

/**
*		功能：信号量P操作
*		参数：信号量（Semaphore）类型
*		返回：无
*/
void SemaphoreWait(Semaphore *s)
{
	EA = 0;//关中断，实现原子操作
	*s = (*s) - 1;
	if ((*s)<0) 
	{
		PCB_IndexTable[PCB_Current]->s = s;
		PCB_IndexTable[PCB_Current]->ProcessState = Block;
	}
	EA = 1;
	//目前先盲等	等时间片耗完自动切换
	while (PCB_IndexTable[PCB_Current]->ProcessState==Block);
}
/**
*		功能：信号量V操作
*		参数：信号量（Semaphore）类型
*		返回：无
*/
void SemaphoreSignal(Semaphore *s)
{
	EA = 0;
	++(*s); 	/*唤醒操作怎么搞？信号量的阻塞和唤醒有问题*/
	if ((*s)<=0) 
	{
		uint8_t i = 0;
		for (;i<=PCB_Number;i++)
			if (PCB_IndexTable[i]->ProcessState==Block&&PCB_IndexTable[i]->s==s)
			{
				PCB_IndexTable[i]->ProcessState=Ready;
				PCB_IndexTable[i]->s = SemaphoreNull;
				break;
			}
	}
	EA = 1;	
}

/**
*		功能：系统进程，系统初始化后的第一个进程，
*					所有用户进程为该进程的子进程。
*		参数：无
*		返回值：无
*/
void SystemProcess(void)
{
	main_t();
	while(1) temp += 1;
}