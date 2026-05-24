#ifndef __CONNECTION_H
#define __CONNECTION_H

typedef struct{
	uint8_t PackLength;			//接收包长度
	uint8_t PackFill;			//填充空字符
	
	uint8_t CMDLength;			//命令组长度
	uint8_t DataLength;			//数据组长度
	
	uint8_t TempLength;			//复合包缓冲区长度
	
	/*以下无需初始化*/
	
	uint8_t * CMDGroup;			//命令组储存
	uint8_t * DataGroup;		//数据组储存
	
	uint8_t * TempGroup;		//缓存组储存
	
	uint8_t CMDGroupStatus;		//命令组状态位
	uint8_t DataGroupStatus;	//数据组状态位
	uint8_t TempGroupState;		//缓存组状态位
	uint8_t TempGroupErrorState;//缓存组错误状态位
	
	uint8_t GDP2T;				//接收函数的私有函数
	uint8_t SCGD;
	
}Connection_Manager;

/*

以下是外部定义的模板

Connection_Manager CON_xxxxxx={
	.PackLength=8,				//接收包长度 推荐值
	.PackFill=0,                //填充空字符 推荐值
	                            
	.CMDLength=2,               //命令组长度 推荐值
	.DataLength=6,              //数据组长度 推荐值
                                
	.TempLength=32,             //复合包缓冲区长度 推荐值
};

*/

//获取相关结构体地址
USART_Manager * CON_GetOMPoint(void);
USART_Manager * CON_GetBTPoint(void);
//结构体初始化
uint8_t CON_ManagerInit(Connection_Manager * Manager);
//初始化
void CON_Init(void);
//读状态位
uint8_t CON_GetCMDGroupStatus(Connection_Manager * Manager);
uint8_t CON_GetDataGroupStatus(Connection_Manager * Manager);
uint8_t CON_GetTempGroupStatus(Connection_Manager * Manager);
uint8_t CON_GetTempGroupErrorStatus(Connection_Manager * Manager);
//读数组
uint8_t * CON_GetCMDGroupPointer(Connection_Manager * Manager);
//获取命令数组指针（不改灯）
uint8_t * CON_GetCMDGroupPointerTemp(Connection_Manager * Manager);

uint8_t * CON_GetDataGroupPointer(Connection_Manager * Manager);
uint8_t * CON_GetDataGroupPointerTemp(Connection_Manager * Manager);

uint8_t * CON_GetTempGroupPointer(Connection_Manager * Manager);
uint8_t * CON_GetTempGroupPointerTemp(Connection_Manager * Manager);

void CON_ClearCMDGroup(Connection_Manager * Manager);
void CON_ClearDataGroup(Connection_Manager * Manager);
void CON_ClearTempGroup(Connection_Manager * Manager);

//包拆分
void CON_SplitPack(Connection_Manager * Manager,uint8_t * Pack);
//指令识别
uint8_t CON_IdentifyCMDGroup(Connection_Manager * Manager,uint8_t *CommandGroup);
//特定指令识别
uint8_t CON_IdentifyCMDSpecial(Connection_Manager * Manager,uint8_t Command,uint8_t Site);
/*
数据包加工函数

对openmv传来的由两个包组成的特征值包进行解析

传入指定数组中
*/

//专用于接收只有两个命令尾的复合包
uint8_t CON_GetDataPack2Tails(Connection_Manager * Manager,USART_Manager * UManager,uint16_t * Container,uint8_t CMDHead,uint8_t CMDTail1,uint8_t CMDTail2);
//重复接收至结束命令尾
uint8_t CON_SpecifyCMDGetData(Connection_Manager * Manager,USART_Manager * UManager,uint8_t CMDHead,uint8_t CMDEnd);

uint8_t CON_PossessTempGroup(Connection_Manager * Manager,uint16_t * Container,uint8_t Length,uint8_t Items);

void CON_BTSpecial(int16_t* DataGroup,uint8_t Length,uint8_t BTCMDHead,uint8_t BTCMDTailEnd);


#endif
