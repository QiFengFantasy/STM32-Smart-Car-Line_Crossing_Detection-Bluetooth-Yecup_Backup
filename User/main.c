#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "string.h"
#include "MyUSART.h"
#include "Connection.h"
#include "MPU.h"
#include "Detector.h"
#include "CMD.h"
#include "PID.h"
#include "OS.h"
#include "Buzzer.h"
#include "RunningMemory.h"
#include "Menu.h"
#include "Ultrasonic.h"

/*pid数值切换*/

uint8_t PID_ThemesNum=0;
typedef struct{
	PIDManager * PIDManagerPoint;
	uint16_t BaseSpeedSet;
	int8_t LeftSpeedAdd;
	int8_t RightSPeedAdd;
}FlexPID_Manager;

/*对所需要的pid结构体进行定义*/

PIDManager PIDManager_V0={ 
.Goal=0, 		.MaxLimit=30, 	.MinLimit=-30,
.P_Rate=0.255,	.I_Rate=0,		.D_Rate=0.014,
.IntegralMax=100,			.IntegralMin=-100,
};//75速

PIDManager PIDManager_V1={ 
.Goal=0, 		.MaxLimit=35, 	.MinLimit=-35,
.P_Rate=0.27,	.I_Rate=0,		.D_Rate=0.015,
.IntegralMax=100,			.IntegralMin=-100,
};//85速

PIDManager PIDManager_V2={ 
.Goal=0, 		.MaxLimit=45, 	.MinLimit=-45,
.P_Rate=0.295,	.I_Rate=0,		.D_Rate=0.018,
.IntegralMax=100,			.IntegralMin=-100,
};//105速

PIDManager PIDManager_V3={ 
.Goal=0, 		.MaxLimit=50, 	.MinLimit=-50,
.P_Rate=0.31,	.I_Rate=0,		.D_Rate=0.02,
.IntegralMax=100,			.IntegralMin=-100,
};//120速

PIDManager PIDManager_V4={ 
.Goal=0, 		.MaxLimit=55, 	.MinLimit=-55,
.P_Rate=0.335,	.I_Rate=0,		.D_Rate=0.024,
.IntegralMax=100,			.IntegralMin=-100,
};//125速

PIDManager PIDManager_V5={ 
.Goal=0, 		.MaxLimit=70, 	.MinLimit=-70,
.P_Rate=0.415,	.I_Rate=0,		.D_Rate=0.045,
.IntegralMax=100,			.IntegralMin=-100,
};//135速

PIDManager PIDManager_V6={ 
.Goal=0, 		.MaxLimit=70, 	.MinLimit=-70,
.P_Rate=0.41,	.I_Rate=0,		.D_Rate=0.045,
.IntegralMax=100,			.IntegralMin=-100,
};//170速

PIDManager PIDManager_V7={ 
.Goal=0, 		.MaxLimit=68, 	.MinLimit=-68,
.P_Rate=0.39,	.I_Rate=0,		.D_Rate=0.039,
.IntegralMax=100,			.IntegralMin=-100,
};//160速

/*多重pid方案复合结构体*/
FlexPID_Manager PID_Themes[]={
	{&PIDManager_V3,120,0,0},
	{&PIDManager_V4,125,0,0},
	{&PIDManager_V5,180,0,5},
	{&PIDManager_V6,170,0,5},
	{&PIDManager_V7,160,0,5},
};

Connection_Manager CON_OM={
	.PackLength=8,				//接收包长度 推荐值
	.PackFill=0,                //填充空字符 推荐值
	                            
	.CMDLength=2,               //命令组长度 推荐值
	.DataLength=6,              //数据组长度 推荐值
                                
	.TempLength=32,             //复合包缓冲区长度 推荐值
};

Connection_Manager CON_BT={
	.PackLength=8,				//接收包长度 推荐值
	.PackFill=0,                //填充空字符 推荐值
	                            
	.CMDLength=2,               //命令组长度 推荐值
	.DataLength=6,              //数据组长度 推荐值
                                
	.TempLength=32,             //复合包缓冲区长度 推荐值
};

/*对所需要定时调用的函数任务轮进行定义*/
int16_t TestValue=0;

void PIDTest(void)
{
	if(TestValue<160){
		TestValue+=20;
	}else{
		TestValue=-160;
	}
	PID_Themes[PID_ThemesNum].PIDManagerPoint->Current=TestValue;
}

int16_t BTDataGroup[24]={0};					//这是蓝牙数据回传的储存数组

void BTSend(void)
{
	/*蓝牙数据回传*/
	CON_BTSpecial(BTDataGroup,11,0xA0,0x05);		//最后的包尾长度需与数组长度对应
}

/*主函数部分*/

int main(void){
	
	/*首要前置*/
	
	/*第一初始化部分*/
	
	OLED_Init();
	Buzzer_Init();
	
	/*第一前置代码*/
	int16_t * PIDMemory;
	
	uint16_t DetectorData[4]={0};	//各种传感器数据的储存数组
	
	uint16_t XY[4]={0};
	uint16_t BaseSpeed=75;			//已废弃的变量

	//已废弃的菜单显示
	Menu_Manager Menu_Main[]={
		{"LeftX",(int16_t *)&XY[0],3,Data_unsigned},
		{"LeftY",(int16_t *)&XY[1],3,Data_unsigned},
		{"RightX",(int16_t *)&XY[2],3,Data_unsigned},
		{"RightY",(int16_t *)&XY[3],3,Data_unsigned},
		{"Speed",(int16_t *)&(PID_Themes[PID_ThemesNum].BaseSpeedSet),3,Data_unsigned},
		{"Max",(int16_t *)&(PID_Themes[PID_ThemesNum].PIDManagerPoint->MaxLimit),3,Data_signed},
		{"Min",(int16_t *)&(PID_Themes[PID_ThemesNum].PIDManagerPoint->MinLimit),3,Data_signed},
		{"Sensor1",(int16_t *)&DetectorData[0],4,Data_unsigned},
		{"Sensor2",(int16_t *)&DetectorData[1],4,Data_unsigned},
		{"Sensor3",(int16_t *)&DetectorData[2],4,Data_unsigned},
		{"Sensor4",(int16_t *)&DetectorData[3],4,Data_unsigned},
	};
	
	/*oled硬件检测部分*/
	
	OLED_ShowString(1,1,"Ready"); 
	
	Delay_s(1);
	
	OLED_Clear();
	OLED_ShowString(1,1,"Done"); 
	
	Delay_ms(500);
	OLED_ShowString(1,1,"     "); 
	
	/*蜂鸣器检测部分*/
	
	Buzzer_Bzz();
	Delay_ms(100);
	Buzzer_NoBzz();
	
	/*第二初始化部分*/
	
	CON_ManagerInit(&CON_OM);
	CON_ManagerInit(&CON_BT);
	
	CON_Init();
	
	Detector_Init((uint32_t)&DetectorData);
	PID_Init();
	
	US_Init();
	
	Menu_SetRange(2,4);
	Menu_Init(Menu_Main,11);
	
	RunningMemory_Init(PIDMemory,3000);
	
	/*任务函数及其初始化部分*/
	
	TaskStruct Schedule[]={
		{BTSend,150},
		{US_SendTrig,250},
		{US_CalculateResult,250},
		{Menu_Display,500},
		{Menu_Roll,2000},
		{PIDTest,500},
	};
	
	OS_Init(Schedule,3);
	
	/*第二前置代码*/
	
	MPU_Start();
	MPU_AllAhead();
	MPU_SetSpeed(BaseSpeed,BaseSpeed);
	
	const uint16_t WindowCX = 160;
	
	int16_t Value=0;
	int16_t LastValue=0;
	
	uint8_t RunningState=1;
	uint16_t StopRunning=250;
	
	uint16_t DT_Threholds[6]={0,750,1500,2250,3000,4096};
	
	uint16_t Distance=0;
	uint8_t DIS_TurnState;		//转向标注位
	
	PIDManager * CurrentPIDThemePoint=&PIDManager_V3;
	
	FlexPID_Manager * CurrentThemesPoint=&PID_Themes[0];
	
	/*OLED前置显示*/
	OLED_ShowString(1,1,"PID Mode");
	
	/*循环代码*/
	
	while(1){
		
		
		/*串口通信&数据加工*/
		
		if(CON_SpecifyCMDGetData(&CON_OM,CON_GetOMPoint(),GetBlobsCMDHead,GetBlobsCMDTail2)){
			CON_PossessTempGroup(&CON_OM,XY,4,2);
		}
		
		/*pid方案选择*/
		
		uint16_t TempDTData=DetectorData[1];
		
		for(uint8_t i=0;i<5;i++){
			if(OS_GetOSTimeTick() > 500){
				if(TempDTData > DT_Threholds[i] && TempDTData < DT_Threholds[i+1] && PID_ThemesNum != i){
					PID_ThemesNum=i;
					
					CurrentThemesPoint=&PID_Themes[PID_ThemesNum];
					
					CurrentPIDThemePoint = CurrentThemesPoint->PIDManagerPoint;
					
					Menu_Main[4].DataPoint = (int16_t *)&(CurrentThemesPoint->BaseSpeedSet);
					Menu_Main[5].DataPoint = (int16_t *)&(CurrentThemesPoint->PIDManagerPoint->MaxLimit);
					Menu_Main[6].DataPoint = (int16_t *)&(CurrentThemesPoint->PIDManagerPoint->MinLimit);
					
				}
			}
			else{
				PID_ThemesNum=0;
				
				CurrentThemesPoint=&PID_Themes[PID_ThemesNum];
					
				CurrentPIDThemePoint = CurrentThemesPoint->PIDManagerPoint;
			}
		}
		
		/*pid数据加工*/
		
		PID_Calculate(CurrentPIDThemePoint);
		
		//一阶低通滤波
		
		LastValue=Value;
		Value=Value * 0.7f + CurrentPIDThemePoint->Output * 0.3f;
		
		//数据更新 //调试时以下代码注释掉
		
		if(XY[0]&&XY[2]==0){
			CurrentPIDThemePoint->Current=0 - XY[0];		//左检测，输出负值
		}else if(XY[2]&&XY[0]==0){
			CurrentPIDThemePoint->Current=320 -XY[2];		//右检测，输出正值
		}else if(XY[2] && XY[0]){
			CurrentPIDThemePoint->Current=(XY[2]+XY[0]-2*WindowCX)*1.5f;	//比较哪边误差大
		}else{
			CurrentPIDThemePoint->Current=0;
		}
		
		
		/*蓝牙回传数据加工*/
		BTDataGroup[0]=XY[0];
		BTDataGroup[1]=XY[1];
		BTDataGroup[2]=XY[2];
		BTDataGroup[3]=XY[3];
		BTDataGroup[4]=Value;
		BTDataGroup[5]=PID_ThemesNum;
		BTDataGroup[6]=DetectorData[0];
		BTDataGroup[7]=DetectorData[1];
		BTDataGroup[8]=DetectorData[2];
		BTDataGroup[9]=DetectorData[3];
		BTDataGroup[10]=(uint16_t)Distance;
		
		/*轮询任务执行*/
		
		OS_ScheduleRun(Schedule);
		
		/*超声波数据更新*/
		
		Distance=US_GetDistance();
		
		/*电机任务*/
		
		//设定电机速度
		MPU_SetSpeedAndTurn(CurrentThemesPoint->BaseSpeedSet+CurrentThemesPoint->LeftSpeedAdd-Value,CurrentThemesPoint->BaseSpeedSet+CurrentThemesPoint->LeftSpeedAdd+Value);	//对pid的输出进行调整
		
		//紧急停机操作
		uint16_t RunningBar=DetectorData[0]/10;
		if(RunningBar < StopRunning){
				MPU_Start();
				RunningState=0;
				Buzzer_TickBzz(250);
		}else if(RunningState==0 && RunningBar >=StopRunning){
				MPU_AllAhead();
				RunningState=1;
		}
		
		//电机避障
//		if(Distance < 650 && DIS_TurnState ==0){
//			DIS_TurnState=1;
//			MPU_LeftSideARightSideB();
//			Buzzer_TickBzz(250);
//		}else if(Distance > 1000 && DIS_TurnState == 1){
//			DIS_TurnState=0;
//			MPU_AllAhead();
//		}x
		
		
		/*蜂鸣器操作*/
		
		Buzzer_AutoCheck(50);
		
		if(LastValue >= 0 && Value < 0){
			Buzzer_TickBzz(100);
		}else if(LastValue <= 0 && Value > 0){
			Buzzer_TickBzz(100);
		}
		
		
		/*oled屏显示*/
		
		OLED_ShowNum(1,10,PID_ThemesNum,1);
//		OLED_ShowNum(2,1,Distance,12);
		
	}
}
