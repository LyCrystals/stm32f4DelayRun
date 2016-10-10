/*
*延时运行
*定时运行
*
*/


#ifndef __DELAY_RUN_H__
#define __DELAY_RUN_H__
#include  "stm32f4xx_hal.h"

struct DelayRunTickStruct
{
	uint32_t s;
	uint32_t ms;
};

struct DelayRunStruct
{
	uint8_t SameNum;
	struct DelayRunTickStruct RunTick;
	void (*proc)(void);
	
	struct DelayRunStruct *next;
};


int8_t ProcIns_t(void (*proc)(void), struct DelayRunTickStruct Tick,uint8_t InsSig);
int8_t ProcIns_d(void (*proc)(void), struct DelayRunTickStruct Tick,uint8_t InsSig);
int8_t ProgInsms(void (*proc)(void), uint32_t Delayms,uint8_t InsSig);
void DelayRunKernel(void);
void DelayRunTick(void);
void DelayRunInit(void);
int8_t DelayRunReset(void);
int8_t GetDrcLock(void);
int8_t ReleaseDrcLock(void);
inline static int8_t DrTickCompare(struct DelayRunTickStruct *Tick_1,struct DelayRunTickStruct *Tick_2);







#endif



