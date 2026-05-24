#ifndef __OS_H
#define __OS_H

typedef struct{
	void (*Task)(void);
	uint32_t Period;
	uint32_t LastRunTick;
}TaskStruct;


void OS_Init(TaskStruct *Schedule,uint8_t TasksNum);

uint8_t OS_GetTasksCNT(void);
uint32_t OS_GetOSTimeTick(void);

void OS_ScheduleRun(TaskStruct *TasksSchedule);


#endif
