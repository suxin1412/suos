;
;	版权：苏鑫(suxin1412@qq.com)，2019
;	
;	文件：Core.asm
;	版本：VER 1.0
;	时间：2019/03/04
;	描述：系统内核，实现进程调度功能
;

	PUBLIC	?C_STARTUP        	;向外部申明，ROM运行起始位置
	
;	申明外部符号
	EXTRN	IDATA(SystemStack)	;系统栈地址
	EXTRN	IDATA(UserStack)	;用户进程栈地址
	EXTRN	CODE(?C_START)		;main函数入口地址
	EXTRN	CODE(Control_Process,BlockSwitch_Process,UartTI)	;系统进程
	EXTRN	DATA(SP_Backups,PSW_Backups,R0_Backups,R1_Backups,R2_Backups,R3_Backups)	
	EXTRN	DATA(ACC_Backups,B_Backups,R4_Backups,R5_Backups,R6_Backups,R7_Backups)
	EXTRN	DATA(DPH_Backups,DPL_Backups)
	
	;PUBLIC FXC
	;EXTRN CODE(xdxd)
	
;	PC指针起点
?C_STARTUP :
	ORG 0X00
	JMP System_Reset

	;外部0中断向量
	;ORG		0x03
	;JMP		Handle_External_0_Interrupt
	
	;定时器0中断向量
	ORG		0x0B
	JMP		Timer0_ServiceFunction
	
	;SPI 中断向量
	ORG 	0X4B
	JMP		UserBlock_ServiceFunction
	
	;UART 中断向量
	ORG 	0X23
	JMP		Uart_ServiceFunction

; 系统复位
System_Reset:
	MOV 	SP, #(UserStack - 1)	;初始化系统栈
	JMP		?C_START
RET

;保存CPU现场信息，保存上文
Save_CPU_Context:
	MOV 	PSW_Backups, PSW
	MOV 	ACC_Backups, A
	MOV 	B_Backups, B
	MOV		R0_Backups, R0
	MOV		R1_Backups, R1
	MOV		R2_Backups, R2
	MOV		R3_Backups, R3
	MOV		R4_Backups, R4
	MOV		R5_Backups, R5
	MOV		R6_Backups, R6
	MOV		R7_Backups, R7
	MOV		DPL_Backups, DPL
	MOV		DPH_Backups, DPH
RET

;恢复CPU现场信息，切换下文
Recovery_CPU_Context:
	MOV 	PSW, PSW_Backups
	MOV 	A, ACC_Backups
	MOV 	B, B_Backups
	MOV		R0, R0_Backups
	MOV		R1, R1_Backups
	MOV		R2, R2_Backups
	MOV		R3, R3_Backups
	MOV		R4, R4_Backups
	MOV		R5, R5_Backups
	MOV		R6, R6_Backups
	MOV		R7, R7_Backups
	MOV		DPL, DPL_Backups
	MOV		DPH, DPH_Backups
RET

;定时器0中断服务函数 进程调度在此实现 
Timer0_ServiceFunction:
	MOV 	SP_Backups, SP			;将当前用户的SP指针保存
	MOV 	SP, #(SystemStack - 1)	;将系统栈设置为当前栈
	LCALL 	Save_CPU_Context		;保存现场环境
	LCALL 	Control_Process			;调用进程调度函数 调用后系统将切换现场信息和断点
	LCALL 	Recovery_CPU_Context	;恢复现场信息
	MOV 	SP, SP_Backups			;恢复栈指针
RETI

UserBlock_ServiceFunction:
	MOV 	SP_Backups, SP			;将当前用户的SP指针保存
	MOV 	SP, #(SystemStack - 1)	;将系统栈设置为当前栈
	LCALL 	Save_CPU_Context		;保存现场环境
	LCALL 	BlockSwitch_Process		;调用进程调度函数 调用后系统将切换现场信息和断点
	LCALL 	Recovery_CPU_Context	;恢复现场信息
	MOV 	SP, SP_Backups			;恢复栈指针
RETI

Uart_ServiceFunction:
	LCALL UartTI
RETI

END