#include "stm32f10x.h"                  // Device header
#include "MyUSART.h"
#include <stdlib.h>
#include <string.h>
//#include "CMD.h"

/*
用于解析包指令的库函数组合
*/

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

//定义相关结构体
USART_Manager USART_OM={
	.RCC_APB2Periph_GPIO	=	RCC_APB2Periph_GPIOA,
	.RCC_APBPeriph_USART	=	RCC_APB2Periph_USART1,

	.GPIO	=	GPIOA,
	.TxPin	=	GPIO_Pin_9,
	.RxPin	=	GPIO_Pin_10,
	
	.USART			=	USART1,
	.IRQ_Channel	=	USART1_IRQn,
	.PreemptionPriority=2,
	.SubPriority	=	2,
	.BaudingRate	=	38400,
	
	.RxDataTempMax	=	16,
	
	.PackageTitle	=	0xFF,	//推荐值
	.PackageEnd		=	0xFE,	//推荐值
	.PackageLength	=	8,		//推荐值
	.PackageFill	=	0,		//推荐值
	
	.RvStringMax	=	127		//推荐值
};

USART_Manager USART_BT={
	.RCC_APB2Periph_GPIO	=	RCC_APB2Periph_GPIOA,
	.RCC_APBPeriph_USART	=	RCC_APB1Periph_USART2,

	.GPIO	=	GPIOA,
	.TxPin	=	GPIO_Pin_2,
	.RxPin	=	GPIO_Pin_3,
	
	.USART			=	USART2,
	.IRQ_Channel	=	USART2_IRQn,
	.PreemptionPriority=1,
	.SubPriority	=	2,
	.BaudingRate	=	9600,
	
	.RxDataTempMax	=	16,
	
	.PackageTitle	=	0xFF,
	.PackageEnd		=	0xFE,
	.PackageLength	=	8,
	.PackageFill	=	0,
	
	.RvStringMax	=	127
};

USART_Manager * CON_GetOMPoint(void)
{
	return &USART_OM;
}

USART_Manager * CON_GetBTPoint(void)
{
	return &USART_BT;
}

uint8_t CON_ManagerInit(Connection_Manager * Manager)
{
	// 分配缓冲区内存
    Manager->CMDGroup = (uint8_t*)malloc(Manager->CMDLength * sizeof(uint8_t));
    Manager->DataGroup = (uint8_t*)malloc(Manager->DataLength * sizeof(uint8_t));
    Manager->TempGroup = (uint8_t*)malloc(Manager->TempLength * sizeof(uint8_t));
    
    // 检查内存分配是否成功
    if (!Manager->CMDGroup || !Manager->DataGroup || !Manager->TempGroup) {
        // 内存分配失败，释放已分配的内存
        free(Manager->CMDGroup);
        free(Manager->DataGroup);
        free(Manager->TempGroup);
        return 0; // 初始化失败
    }
    
    // 初始化缓冲区内容
    memset(Manager->CMDGroup, 0, Manager->CMDLength);
    memset(Manager->DataGroup, 0, Manager->DataLength);
    memset(Manager->TempGroup, 0, Manager->TempLength);
    
    // 初始化状态变量
    Manager->CMDGroupStatus = 0;
    Manager->DataGroupStatus = 0;
    Manager->TempGroupErrorState = 0;
    Manager->TempGroupState = 0;
    Manager->GDP2T = 0;
    Manager->SCGD = 0;
    
    return 1; // 初始化成功
}

//初始化
void CON_Init(void)
{
	MyUSART_Init(&USART_OM);
	MyUSART_Init(&USART_BT);
}

//读状态位
uint8_t CON_GetCMDGroupStatus(Connection_Manager * Manager)
{
	return Manager->CMDGroupStatus;
}

uint8_t CON_GetDataGroupStatus(Connection_Manager * Manager)
{
	return Manager->DataGroupStatus;
}

uint8_t CON_GetTempGroupStatus(Connection_Manager * Manager)
{
	return Manager->TempGroupState;
}

uint8_t CON_GetTempGroupErrorStatus(Connection_Manager * Manager)
{
	return Manager->TempGroupErrorState;
}

//读数组
uint8_t * CON_GetCMDGroupPointer(Connection_Manager * Manager)
{
	Manager->CMDGroupStatus=0;
	return Manager->CMDGroup;
}

//获取命令数组指针（不改灯）
uint8_t * CON_GetCMDGroupPointerTemp(Connection_Manager * Manager)
{
	return Manager->CMDGroup;
}


uint8_t * CON_GetDataGroupPointer(Connection_Manager * Manager)
{
	Manager->DataGroupStatus=0;
	return Manager->DataGroup;
}


uint8_t * CON_GetDataGroupPointerTemp(Connection_Manager * Manager)
{
	Manager->DataGroupStatus=0;
	return Manager->DataGroup;
}


uint8_t * CON_GetTempGroupPointer(Connection_Manager * Manager)
{
	Manager->TempGroupState=0;
	return Manager->TempGroup;
}


uint8_t * CON_GetTempGroupPointerTemp(Connection_Manager * Manager)
{
	Manager->TempGroupState=0;
	return Manager->TempGroup;
}

void CON_ClearCMDGroup(Connection_Manager * Manager)
{
	for(uint8_t i=0;i<Manager->CMDLength;i++){
		Manager->CMDGroup[i]=0;
	}
}

void CON_ClearDataGroup(Connection_Manager * Manager)
{
	for(uint8_t i=0;i<Manager->DataLength;i++){
		Manager->DataGroup[i]=0;
	}
}

void CON_ClearTempGroup(Connection_Manager * Manager)
{
	for(uint8_t i=0;i<Manager->TempLength;i++){
		Manager->TempGroup[i]=0;
	}
}

//包拆分
void CON_SplitPack(Connection_Manager * Manager,uint8_t * Pack)
{
	CON_ClearCMDGroup(Manager);
	CON_ClearDataGroup(Manager);
	for(uint8_t i=0;i<Manager->PackLength;i++){
		if(i<Manager->CMDLength){
			Manager->CMDGroup[i]=Pack[i];
		}else if(i<Manager->PackLength){
			Manager->DataGroup[i-Manager->CMDLength]=Pack[i];
		}
	}
	Manager->CMDGroupStatus=1;
	Manager->DataGroupStatus=1;
}

//指令识别
uint8_t CON_IdentifyCMDGroup(Connection_Manager * Manager,uint8_t *CommandGroup)
{
	uint8_t ret=1;
	for(uint8_t i=0;i<Manager->PackLength;i++){
		if(Manager->CMDGroup[i]!=CommandGroup[i])ret=0;
	}
	
	return ret;
}

//特定指令识别
uint8_t CON_IdentifyCMDSpecial(Connection_Manager * Manager,uint8_t Command,uint8_t Site)
{
	uint8_t ret=1;
	if(Manager->CMDGroup[Site]!=Command)ret=0;
	return ret;
}

/*
数据包加工函数

对openmv传来的由两个包组成的特征值包进行解析

传入指定数组中
*/

//专用于接收只有两个命令尾的复合包
uint8_t CON_GetDataPack2Tails(Connection_Manager * Manager,USART_Manager * UManager,uint16_t * Container,uint8_t CMDHead,uint8_t CMDTail1,uint8_t CMDTail2)
{
	uint8_t feedback=0;
	
	if(MyUSART_GetReceivePackStatus(UManager)){
		CON_SplitPack(Manager,MyUSART_GetRvPackPoint(UManager));
	}
	
	if(CON_GetCMDGroupStatus(Manager)){
		if(CON_IdentifyCMDSpecial(Manager,CMDHead,0)){
			if(Manager->GDP2T==0)Manager->GDP2T=1;
		}
	}
	
	if(Manager->GDP2T<3 && Manager->GDP2T>0){
		if(CON_IdentifyCMDSpecial(Manager,CMDTail1,1)){
			for(uint8_t i=0;i<3;i++){
				Container[i]=Manager->DataGroup[2*i]+Manager->DataGroup[2*i+1];
			}
			Manager->GDP2T++;
		}else if(CON_IdentifyCMDSpecial(Manager,CMDTail2,1)){
			for(uint8_t i=0;i<3;i++){
				Container[i+3]=Manager->DataGroup[2*i]+Manager->DataGroup[2*i+1];
			}
			Manager->GDP2T++;
		}
	}
	
	if(Manager->GDP2T==3){
		Manager->GDP2T=0;
		feedback=1;
	}
	return feedback;
}

//重复接收至结束命令尾
uint8_t CON_SpecifyCMDGetData(Connection_Manager * Manager,USART_Manager * UManager,uint8_t CMDHead,uint8_t CMDEnd)
{
	uint8_t P=0;
	uint8_t feedback=0;
	
	if(MyUSART_GetReceivePackStatus(UManager)){
		CON_SplitPack(Manager,MyUSART_GetRvPackPoint(UManager));
	}
	
	if(CON_GetCMDGroupStatus(Manager)){
		if(CON_IdentifyCMDSpecial(Manager,CMDHead,0)){
			if(Manager->SCGD==0){
				Manager->SCGD=1;
				Manager->TempGroupErrorState=0;
			}
		}
	}
	
	if(Manager->SCGD==1)
	{
		P=Manager->CMDGroup[1];
		for(uint8_t i=0;i<Manager->DataLength;i++){
			if(P*Manager->DataLength+i<Manager->TempLength){
				Manager->TempGroup[P*Manager->DataLength+i]=Manager->DataGroup[i];
			}else Manager->TempGroupErrorState=1;
		}
		
		if(P==CMDEnd){
			Manager->SCGD=2;
		}
	}
	
	if(Manager->SCGD==2){
		Manager->SCGD=0;
		feedback=1;
	}
	
	return feedback;
}

uint8_t CON_PossessTempGroup(Connection_Manager * Manager,uint16_t * Container,uint8_t Length,uint8_t Items)
{
	for(uint8_t i=0;i<Length*Items;i++)
	{
		Container[i/Items]=0;
	}
	for(uint8_t i=0;i<Length*Items;i++)
	{
		Container[i/Items]+=Manager->TempGroup[i];
	}
	
	return 1;
}

/*
专用于蓝牙模块回传信息的函数
*/

void CON_BTSpecial(int16_t* DataGroup,uint8_t Length,uint8_t BTCMDHead,uint8_t BTCMDTailEnd)
{
	uint8_t Definition=0;
	uint8_t DataHighSet=0;
	uint8_t DataLowSet=0;
	
	for(uint8_t i=0;i<Length;i+=2){
		
		uint8_t SendPackTemp[8]={0};
			
		SendPackTemp[0]=BTCMDHead;
		uint8_t Tail=i/2;
		
		if(Tail>BTCMDTailEnd){
			Tail=BTCMDTailEnd;
		}
		
		SendPackTemp[1]=Tail;
		
		for(uint8_t j=0;j<2;j++){
			int16_t tempData=DataGroup[i+j];
			
			if(tempData<-127){
				Definition=4;
				tempData=-tempData;
			}else if(tempData<0){
				Definition=3;
				tempData=-tempData;
			}else if(tempData>127){
				Definition=2;
			}else if(tempData>0){
				Definition=1;
			}
			
			if(Definition==2 || Definition==4){
				DataHighSet=(tempData >>8) & 0x00FF;
				DataLowSet=tempData & 0x00FF;
			}else{
				DataHighSet=0;
				DataLowSet=tempData;
			}
			
			SendPackTemp[2+j*3]=Definition;
			SendPackTemp[3+j*3]=DataHighSet;
			SendPackTemp[4+j*3]=DataLowSet;
			
		}
		
		MyUSART_SendPackage(&USART_BT,SendPackTemp);
	}
}
