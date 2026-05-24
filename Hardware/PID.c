#include "stm32f10x.h"                  // Device header
#include "MPU.h"
#include "OS.h"

uint32_t Time;

typedef struct{
	int16_t Goal;			//目标值
	int16_t Current;		//现有数值
	
	int16_t MaxLimit;		//最大限制
	int16_t MinLimit;		//最小限制
	
	float P_Rate;			//比例占比
	float I_Rate;			//积分占比
	float D_Rate;			//微分占比
	
	float IntegralMax;		//积分限幅
	float IntegralMin;
	
	/*以下为自有变量无需初始化赋值*/
	int16_t Output;			//最终输出
	
	float Integral;			//积分积累
	int16_t LastCurrent;	//上一个数值
	
	float LastOutset;		//上一个误差
	
	uint32_t TimeTick;		//时间戳
	uint32_t LastTimeTick;	//上一个时间戳
	
	float P_Value;			//比例数值
	float I_Value;			//积分数值
	float D_Value;			//微分数值
	
}PIDManager;

/*

以下是定义结构体的样例

PIDManager xxxxx={
	.Goal=x,				//目标值
	
	.MaxLimit=x,			//最大限制
	.MinLimit=x,			//最小限制
	
	.P_Rate=x,				//比例占比
	.I_Rate=x,				//积分占比
	.D_Rate=x,				//微分占比

	.IntegralMax=x,			//积分限幅
	.IntegralMin=x,
};

*/

PIDManager* TargetManager;

void PID_Init(PIDManager* Manager)
{
	TargetManager=Manager;
	
	MPU_Init();
	
	MPU_Start();
}

void PID_Calculate(PIDManager* Manager)
{
	//时间过滤 确保10ms的pid数据更新频率
	Manager->TimeTick = OS_GetOSTimeTick()/10;
	
	float Outset=Manager->Goal - Manager->Current;						//当前误差
	float TimeBuff=(Manager->TimeTick - Manager->LastTimeTick)*0.01f;	//时间间隔
	if(TimeBuff<0.01f)TimeBuff=0.01f;
	
	
	//比例项
	Manager->P_Value = Manager->P_Rate * Outset;
	
	//积分项
	Manager->Integral += Outset * TimeBuff;
	
	if(Manager->Integral > Manager->IntegralMax){
		Manager->Integral=Manager->IntegralMax;
	}
	if(Manager->Integral < Manager->IntegralMin){
		Manager->Integral=Manager->IntegralMin;
	}
	
	Manager->I_Value = Manager->I_Rate * Manager->Integral;
	
	//微分项
	Manager->D_Value = Manager->D_Rate * (Outset - Manager->LastOutset) / TimeBuff;
	
	//
	float output = Manager->P_Value + Manager->I_Value + Manager->D_Value;
	
	if(output > Manager->MaxLimit){
		output = Manager->MaxLimit;
		
		//饱和时停止积分
		Manager->Integral -= Outset * TimeBuff;
	}
	if(output < Manager->MinLimit){
		output = Manager->MinLimit;
		
		//饱和时停止积分
		Manager->Integral -= Outset * TimeBuff;
	}
	
	Manager->Output = output;
	
	Manager->LastTimeTick = Manager->TimeTick;
	Manager->LastOutset = Outset;
	Manager->LastCurrent = Manager->Current;
}
