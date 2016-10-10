/*
*延时运行
*定时运行
*
*/

#include  "stdlib.h"
#include  "string.h"
#include "tim.h"
#include 	"DelayRun.h"
struct DelayRunStruct *DrHeader;		//延时运行链的头部
struct DelayRunTickStruct DrTick;			//延时运行的核心时间
uint8_t DrcLock;											//运行链锁；1:上锁；0：未上锁；

#define TICKCTRL_IT

//未能实现相同时间点的插入顺序；

int8_t ProcIns_t(void (*proc)(void), struct DelayRunTickStruct Tick,uint8_t InsSig)
{
	struct DelayRunStruct *ptr=DrHeader;
	struct DelayRunStruct *ins;
	if(GetDrcLock() == -1) goto  ProgInstFail;
	if(DrTickCompare(&Tick,&DrTick) == -1) goto ProgInstFail;		//执行块插到ptr->next上

	
	while(ptr->next!=NULL){		
		if(DrTickCompare(&Tick,&(ptr->next->RunTick)) == 1)	break;		//此时ptr->next 指向执行结构运行时间大于当前；ptr->RunTick<=当前的运行点,插入到ptr->next上
		ptr=ptr->next;	
	}

	ins=(struct DelayRunStruct *)malloc(sizeof(struct DelayRunStruct));
	if(ins == NULL) goto ProgInstFail;
	ins->next = ptr->next;
	ins->proc = proc;
	ins->RunTick.ms =Tick.ms;
	ins->RunTick.s =Tick.s;
	ptr->next = ins;
	return 0;
	
	ProgInstFail:
		return -1;
}

int8_t ProcIns_d(void (*proc)(void), struct DelayRunTickStruct Tick,uint8_t InsSig)
{
#ifdef TICKCTRL_IT
	__HAL_TIM_DISABLE_IT(&htim6, TIM_IT_UPDATE);
	struct DelayRunTickStruct temp=DrTick;
	__HAL_TIM_ENABLE_IT(&htim6, TIM_IT_UPDATE);
#else
	struct DelayRunTickStruct temp=DrTick;
#endif
	temp.ms += Tick.ms;
	if(temp.ms>=1000){
		temp.ms -=1000;
		temp.s += (Tick.ms+1);
	}
	return ProcIns_t(proc,temp,InsSig);
}

int8_t ProgInsms(void (*proc)(void), uint32_t Delayms,uint8_t InsSig)
{
#ifdef TICKCTRL_IT
	__HAL_TIM_DISABLE_IT(&htim6, TIM_IT_UPDATE);
	struct DelayRunTickStruct  Tick = DrTick;
	__HAL_TIM_ENABLE_IT(&htim6, TIM_IT_UPDATE);
#else
	struct DelayRunTickStruct  Tick = DrTick;
#endif
	Tick.s +=Delayms/1000;
	Tick.ms %= 1000;
	return ProcIns_t(proc,Tick,InsSig);
}

void DelayRunKernel(void)
{
	void (*proc)(void);
	if(DrHeader->next == NULL) return ;
	while(DrTickCompare(&DrHeader->RunTick,&DrTick)!= -1){
		proc = DrHeader->proc;
		DrHeader = DrHeader->next;
		proc();
	}
}

#ifdef TICKCTRL_IT
void DelayRunTick(void)
{
	if(++DrTick.ms>=1000){
		DrTick.s ++;
		DrTick.ms=0;
	}
}
#else
void DelayRunTick(void)
{
	static uint32_t LastTick=0;
	uint32_t CurTick;
	uint32_t DetTick;
	CurTick = HAL_GetTick();
	if(LastTick>CurTick){
		DetTick = 0xFFFFFFFF - LastTick + CurTick;
	}else{
		DetTick = LastTick - CurTick;
	}
	LastTick = CurTick;
	DrTick.ms += DetTick;
	if(DrTick.ms >= 1000){
		DrTick.ms %=1000;
		DrTick.s ++;
	}
	
}	


#endif



void DelayRunInit(void)
{
		DrcLock 	=0;
		DrHeader 	=NULL;
		DrTick.s 	=0;
		DrTick.ms =0;
}

int8_t DelayRunReset(void)
{
	struct DelayRunStruct *ptr;
	if(GetDrcLock()==-1){
	//获取资源锁失败，返回-1；
		return -1;
	}
	//释放整个运行链
	while(DrHeader->next != NULL){
		ptr =DrHeader->next;
		free(DrHeader);
		DrHeader =ptr;
	}
	free(DrHeader);
	DrHeader=NULL;
	return 0;
}

__weak int8_t GetDrcLock(void)
{
	/*关中断*/
		if(DrcLock){
			return -1;
		}else{
			DrcLock =1;
			return 0;
		}
	/*开中断*/	
}


__weak int8_t ReleaseDrcLock(void)
{
	/*关中断*/
	DrcLock =0;
	return 0;
	/*开中断*/

}


/*
*		Tick_1 > Tick_2 返回1；Tick_1 ==Tick_2 返回0； Tick_1 < Tick_2 返回 -1；
*/

inline static int8_t DrTickCompare(struct DelayRunTickStruct *Tick_1,struct DelayRunTickStruct *Tick_2)
{
	if(Tick_1->s >Tick_2->s){
		return 1;
	}else if(Tick_1->s == Tick_2->s){
		if(Tick_1->ms > Tick_2->ms){
			return 1;
		}else if(Tick_1->ms == Tick_2->ms){
			return 0;
		}else{
			return -1;
		}
	}else{
		return -1;
	}
}



