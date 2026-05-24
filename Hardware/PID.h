#ifndef __SPEEDCONTROL_H
#define __SPEEDCONTROL_H

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


void PID_Init(void);
void PID_Calculate(PIDManager* Manager);

#endif
