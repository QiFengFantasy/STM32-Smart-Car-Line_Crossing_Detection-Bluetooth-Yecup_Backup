#include "stm32f10x.h"                  // Device header

/*用于实现定时启动的os功能*/

/*

使用以下方式在main中或其他调用函数中使用定时启动

TaskStruct Schedule[]={
	{TaskName,TaskPeriod}
};

*/

typedef struct{
	void (*Task)(void);			//函数指针接收
	uint32_t Period;			//执行的周期
	
	/*自由函数无需定义*/
	
	uint32_t LastRunTick;
}TaskStruct;

uint8_t TasksCNT = 0;

uint32_t OSTimeTick;

void OS_Init(TaskStruct *Schedule,uint8_t TasksNum)
{
	TasksCNT = TasksNum;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	
	TIM_TimeBaseInitTypeDef TIM_TimebaseInitStructure;
	TIM_TimebaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimebaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimebaseInitStructure.TIM_Period=1000 - 1;
	TIM_TimebaseInitStructure.TIM_Prescaler=72 - 1;
	TIM_TimebaseInitStructure.TIM_RepetitionCounter=1;
	TIM_TimeBaseInit(TIM4,&TIM_TimebaseInitStructure);
	
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM4,ENABLE);
}

void OS_ScheduleRun(TaskStruct *Schedule)
{
	for(uint8_t i=0;i<TasksCNT;i++){
		if(OSTimeTick - Schedule[i].LastRunTick > Schedule[i].Period){
			Schedule[i].LastRunTick = OSTimeTick;
			Schedule[i].Task();
		}
	}
}

uint8_t OS_GetTasksCNT(void)
{
	return TasksCNT;
}

uint32_t OS_GetOSTimeTick(void)
{
	return OSTimeTick;
}

void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4,TIM_IT_Update)){
		OSTimeTick++;
		
		TIM_ClearITPendingBit(TIM4,TIM_IT_Update);
	}
}
