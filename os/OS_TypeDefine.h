#ifndef _OSTYPEDEFIN_H_
#define _OSTYPEDEFIN_H_

//	定义变量类型
#define uint8_t		unsigned char
#define uint16_t	unsigned int
#define uint32_t	unsigned long
#define int8_t		char
#define int16_t		int
#define int32_t		long
//	信号量类型
#define Semaphore int8_t
#define SemaphoreNull (Semaphore*)0
//	定义状态
#define STATUS	uint8_t
#define SUCCESS 1
#define FAIL		0

#endif