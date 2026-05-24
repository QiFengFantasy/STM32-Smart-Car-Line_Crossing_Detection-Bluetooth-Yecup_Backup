#ifndef __MENU_H
#define __MENU_H

typedef enum{
	Data_signed = 0,
	Data_unsigned,
}Data_Type;

typedef struct{
	char Name[32];					//展示名称
	int16_t * DataPoint;			//数据指针
	uint8_t Length;					//数据长度
	Data_Type Type;
}Menu_Manager;

void Menu_Init(Menu_Manager* Menu_Setting,uint8_t DisplayLength);

void Menu_Display(void);

void Menu_SetCurrentLine(int8_t Value);

void Menu_SetRange(uint8_t Head,uint8_t End);
void Menu_Roll(void);			//需自行搭配延时使用


#endif
