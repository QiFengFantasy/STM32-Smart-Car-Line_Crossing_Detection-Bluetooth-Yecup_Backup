#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "string.h"

typedef enum{
	Data_signed = 0,
	Data_unsigned,
}Data_Type;

typedef struct{
	/*需要初始化*/
	char Name[32];					//展示名称
	int16_t * DataPoint;			//数据指针
	uint8_t Length;					//数据长度
	Data_Type Type;					//数据类型
	
	/*无需初始化*/
	int8_t CurrentDisplayLine;
}Menu_Manager;

Menu_Manager * Menu_SettingP=0;		//菜单设定指针
uint8_t Menu_Length=0;				//总数据大小

uint8_t MaxDisplayLine=1;			//OLED显示最大行数

uint8_t CurrentLine=0;				//目前正在展示的第一行的序号

uint8_t RangeHead=1;
uint8_t RangeEnd=4;

void Menu_Init(Menu_Manager* Menu_Setting,uint8_t DisplayLength)
{
	Menu_SettingP=Menu_Setting;
	Menu_Length=DisplayLength;
	
	if(Menu_Length > RangeEnd - RangeHead + 1){
		MaxDisplayLine=RangeEnd - RangeHead + 1;
	}else{
		MaxDisplayLine=Menu_Length;
	}
	
	for(uint8_t i=0;i<Menu_Length;i++){
		Menu_SettingP[i].CurrentDisplayLine=-1;
	}
	
	OLED_Init();
	OLED_Clear();
}

void Menu_Clear(void)
{
	for(uint8_t i=RangeHead;i<=RangeEnd;i++){
		OLED_ShowString(i,1,"                ");
	}
}

void Menu_Display(void)
{
	for(uint8_t i=0;i<Menu_Length;i++){
		
		Menu_SettingP[i].CurrentDisplayLine=-1;
		
		if(i>=CurrentLine && i<CurrentLine+RangeEnd-RangeHead+1){
			Menu_SettingP[i].CurrentDisplayLine=i-CurrentLine+RangeHead-1;
		}
	}
	
	for(uint8_t i=0;i<Menu_Length;i++){
		if(Menu_SettingP[i].CurrentDisplayLine >=0){
			
			uint8_t j=0;
			for(;Menu_SettingP[i].Name[j]!=0;j++);
			
			OLED_ShowString(Menu_SettingP[i].CurrentDisplayLine+1,1,Menu_SettingP[i].Name);
			
			if(Menu_SettingP[i].Type == Data_signed){
				OLED_ShowSignedNum(Menu_SettingP[i].CurrentDisplayLine+1,j+2,*Menu_SettingP[i].DataPoint,Menu_SettingP[i].Length);
			}else{
				OLED_ShowNum(Menu_SettingP[i].CurrentDisplayLine+1,j+2,*Menu_SettingP[i].DataPoint,Menu_SettingP[i].Length);
			}
		}
		
	}
	
}

void Menu_SetCurrentLine(int8_t Value)
{
	if(Value>0){
		if(CurrentLine<Menu_Length-1){
			CurrentLine++;
			Menu_Clear();
		}
	}else if(Value<0){
		if(CurrentLine>0){
			CurrentLine--;
			Menu_Clear();
		}
	}
}

void Menu_SetRange(uint8_t Head,uint8_t End)
{
	RangeHead=Head;
	RangeEnd=End;
}

void Menu_Roll(void)			//需自行搭配延时使用
{
	if(CurrentLine<Menu_Length-1){
		CurrentLine++;
	}else{
		CurrentLine=0;
	}
	Menu_Clear();
}
