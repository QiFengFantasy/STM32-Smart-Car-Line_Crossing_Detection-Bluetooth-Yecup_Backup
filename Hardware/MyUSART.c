#include "stm32f10x.h"                  // Device header
#include "delay.h"
#include <string.h>
#include <stdlib.h>

enum MutipleType
{
	MTArray = 1,
	MTString =2,
	MTNum = 3
};

/*定义一个概括串口信息的结构体*/
typedef struct{
	
	/*以下变量在使用结构体时需要初始化*/
	uint32_t RCC_APB2Periph_GPIO;		//RCC GPIO 通道
	uint32_t RCC_APBPeriph_USART;		//RCC USART 通道
		
	GPIO_TypeDef * GPIO;				//使用的GPIO通道
	uint16_t TxPin;						//用作TX的PIN
	uint16_t RxPin;						//用作RX的PIN
	
	USART_TypeDef * USART;				//使用的USART通道
	IRQn_Type IRQ_Channel;				//USART对应通道的中断通道
	uint8_t PreemptionPriority;			//抢占优先级
	uint8_t SubPriority;				//响应优先级
	uint16_t BaudingRate;				//波特率
	
	uint8_t RxDataTempMax;				//缓冲区大小
	
	uint8_t PackageTitle;				//数据包包头
	uint8_t PackageEnd;					//数据包包尾
	uint8_t PackageLength;				//数据包包长（不含头尾）
	uint8_t PackageFill;				//无数据时默认填充
	
	uint16_t RvStringMax;				//未定型命令数组大小
	
	/*以下变量在定义结构体时无需初始化*/
	
	uint8_t TxStatus;					//用于记录发送状态的状态位
	uint8_t TxData;						//用于记录发送的历史数据的变量
	uint8_t RxStatus;					//用于记录接收状态的状态位
	uint8_t RxData;						//用于记录接收的历史数据的变量
	
	uint8_t SendPackStatus; 			//记录发送数据包的状态位
	uint8_t ReceivePackStatus; 			//记录接收数据包的状态位
	uint8_t ReceiveErrorStatus;			//记录收包时接收到错误数据的状态位
	uint8_t RvCNT;						//接收数据包计数器
		
	uint8_t RvPackP;					//收包函数私有变量，指示数据包接收位置
	uint8_t RvPackStatus;				//收包函数私有变量，指示收包完成状态
	
	uint8_t* RxDataTempGroup;			//收包缓冲区指针
	uint8_t* ReceiveDataPackage;		//数据包接收数组指针
	char* ReceiveString;				//未定型指令指针

}USART_Manager;

USART_Manager * USART1_Config=0;
USART_Manager * USART2_Config=0;
USART_Manager * USART3_Config=0;

/*结构体初始化样例本*/

/*

USART_Manager USART_XXXXXX={
	.RCC_APB2Periph_GPIO	=	RCC_APB2Periph_GPIOx,
	.RCC_APBPeriph_USART	=	RCC_APB2Periph_USARTx,

	.GPIO	=	GPIOx,
	.TxPin	=	GPIO_Pin_x,
	.RxPin	=	GPIO_Pin_x,
	
	.USART			=	USARTx,
	.IRQ_Channel	=	USARTx_IRQn,
	.PreemptionPriority=x,
	.SubPriority	=	x,
	.BaudingRate	=	x,
	
	.RxDataTempMax	=	x,
	
	.PackageTitle	=	0xFF,	//推荐值
	.PackageEnd		=	0xFE,	//推荐值
	.PackageLength	=	8,		//推荐值
	.PackageFill	=	0,		//推荐值
	
	.RvStringMax	=	127		//推荐值
};

*/

void MyUSART_ClearRvPack(USART_Manager * Manager);		//详细见下定义
uint8_t MyUSART_ManagerInit(USART_Manager * Manager);

void MyUSART_Init(USART_Manager * Manager){
	//结构体可变数组初始化
	MyUSART_ManagerInit(Manager);
	
	//开启时钟
	RCC_APB2PeriphClockCmd(Manager->RCC_APB2Periph_GPIO,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	if(Manager->USART ==USART1){
		RCC_APB2PeriphClockCmd(Manager->RCC_APBPeriph_USART,ENABLE);	//USART1时钟
	}else{
		RCC_APB1PeriphClockCmd(Manager->RCC_APBPeriph_USART,ENABLE);
	}
	//GPIO 	TX 初始化
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin=Manager->TxPin;//TX
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	//RX 初始化
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin=Manager->RxPin;//RX
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	//USART 初始化
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate=Manager->BaudingRate;
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode=USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_Parity=0;
	USART_InitStructure.USART_StopBits=1;
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;
	USART_Init(Manager->USART,&USART_InitStructure);
	
	//中断配置
	USART_ITConfig(Manager->USART,USART_IT_RXNE,ENABLE);
	
	//NVIC初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=Manager->IRQ_Channel;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=Manager->PreemptionPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=Manager->SubPriority;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(Manager->USART,ENABLE);
	
	MyUSART_ClearRvPack(Manager);
}

//结构体可变数组初始化
uint8_t MyUSART_ManagerInit(USART_Manager * Manager)
{
	if(Manager->USART==USART1){
		USART1_Config=Manager;
	}else if(Manager->USART==USART2){
		USART2_Config=Manager;
	}else if(Manager->USART==USART3){
		USART3_Config=Manager;
	}
	// 分配缓冲区内存
    Manager->RxDataTempGroup = (uint8_t*)malloc(Manager->RxDataTempMax * sizeof(uint8_t));
    Manager->ReceiveDataPackage = (uint8_t*)malloc(Manager->PackageLength * sizeof(uint8_t));
    Manager->ReceiveString = (char*)malloc(Manager->RvStringMax * sizeof(char));
    
    // 检查内存分配是否成功
    if (!Manager->RxDataTempGroup || !Manager->ReceiveDataPackage || !Manager->ReceiveString) {
        // 内存分配失败，释放已分配的内存
        free(Manager->RxDataTempGroup);
        free(Manager->ReceiveDataPackage);
        free(Manager->ReceiveString);
        return 0; // 初始化失败
    }
    
    // 初始化缓冲区内容
    memset(Manager->RxDataTempGroup, 0, Manager->RxDataTempMax);
    memset(Manager->ReceiveDataPackage, 0, Manager->PackageLength);
    memset(Manager->ReceiveString, 0, Manager->RvStringMax);
    
    // 初始化状态变量
    Manager->TxStatus = 0;
    Manager->RxStatus = 0;
    Manager->SendPackStatus = 0;
    Manager->ReceivePackStatus = 0;
    Manager->ReceiveErrorStatus = 0;
    Manager->RvCNT = 0;
    Manager->RvPackP = 0;
    Manager->RvPackStatus = 0;
    
    return 1; // 初始化成功
}

//释放内存
void MyUSART_Manager_Deinit(USART_Manager* Manager) {
    if (Manager->RxDataTempGroup) free(Manager->RxDataTempGroup);
    if (Manager->ReceiveDataPackage) free(Manager->ReceiveDataPackage);
    if (Manager->ReceiveString) free(Manager->ReceiveString);
    
    Manager->RxDataTempGroup = NULL;
    Manager->ReceiveDataPackage = NULL;
    Manager->ReceiveString = NULL;
}

//获取发送状态位，观察数据是否发送成功
uint8_t MyUSART_GetTxStatus(USART_Manager * Manager){
	return Manager->TxStatus;
}

//获取发送的历史数据（即时覆盖）
uint8_t MyUSART_GetTxData(USART_Manager * Manager){
	return Manager->TxData;
}

//发送数据
void MyUSART_SendTxData(USART_Manager * Manager,uint8_t Data){
	Manager->TxStatus=1;
	Manager->TxData=Data;
	USART_SendData(Manager->USART,Manager->TxData);
	while(USART_GetFlagStatus(Manager->USART,USART_FLAG_TXE)==RESET);
	Manager->TxStatus=0;
	Delay_us(2);
}

//获取接受状态位
uint8_t MyUSART_GetRxStatus(USART_Manager * Manager){
	return Manager->RxStatus;
}

//获取接收数据
uint8_t MyUSART_GetRxData(USART_Manager * Manager){
	uint8_t RxTemp=Manager->RxData;
//	RxData=0;
	Manager->RxStatus=0;
	return RxTemp;
}

//获取接收数据不挂灯
uint8_t MyUSART_GetRxDataTemp(USART_Manager * Manager){
	uint8_t RxTemp=Manager->RxData;
	return RxTemp;
}

/*以下三个为可随意调用的功能性函数*/

//复制uint8_t数组
void MyUSART_ArrU8CPY(uint8_t *ArrSource,uint8_t *ArrTarget,uint8_t length)
{
	for(uint8_t i=0;i<length;i++){
		ArrTarget[i]=ArrSource[i];
	}
}

//复制字符串
void MyUSART_StringCPY(char *ArrSource,char *ArrTarget)
{
	strcpy(ArrSource,ArrTarget);
}

//次幂计算
int32_t MyUSART_Pow(int32_t Num,uint8_t Times){
	int32_t ret=1;
	for(uint8_t i=0;i<Times;i++){
		ret*=Num;
	}
	return ret;
}

//发送数组
void MyUSART_SendArray(USART_Manager * Manager,uint8_t Array[],uint16_t Length){
	for(uint16_t i=0;i<Length;i++){
		MyUSART_SendTxData(Manager,Array[i]);
	}
}

//发送字符串
void MyUSART_SendString(USART_Manager * Manager,char *String){
	for(uint16_t i=0;String[i]!=0;i++){
		MyUSART_SendTxData(Manager,String[i]);
	}
}

//发送数字
void MyUSART_SendNum(USART_Manager * Manager,int32_t Num){
	if(Num){
		int32_t NumTemp=Num;
		uint8_t CNT=0;
		while(NumTemp){
			NumTemp/=10;
			CNT++;
		}
		NumTemp=Num;
		for(uint8_t i=0;CNT!=0;i++){
			MyUSART_SendTxData(Manager,NumTemp/MyUSART_Pow(10,CNT-1));
			NumTemp=NumTemp%MyUSART_Pow(10,CNT-1);
			CNT--;
		}
	}else{
		MyUSART_SendTxData(Manager,0);
	}
}

//发送字符串数字
void MyUSART_SendNumStr(USART_Manager * Manager,int32_t Num){
	if(Num){
		int32_t NumTemp=Num;
		uint8_t CNT=0;
		while(NumTemp){
			NumTemp/=10;
			CNT++;
		}
		NumTemp=Num;
		for(uint8_t i=0;CNT!=0;i++){
			MyUSART_SendTxData(Manager,NumTemp/MyUSART_Pow(10,CNT-1)+48);
			NumTemp=NumTemp%MyUSART_Pow(10,CNT-1);
			CNT--;
		}
	}else{
		MyUSART_SendTxData(Manager,0);
	}
}

//优化调试可视化
void MyUSART_AddEnter(USART_Manager * Manager)
{
	MyUSART_SendString(Manager,"\r\n");
}

//多样化发送数据
uint8_t MyUSART_SendMutipleType(USART_Manager * Manager,enum MutipleType Type,uint8_t *DataPoint,uint32_t length){
	uint8_t ret=1;
	switch(Type){
		case MTArray:
			MyUSART_SendArray(Manager,DataPoint,length);
			break;
		case MTString:
			MyUSART_SendString(Manager,(char *)DataPoint);
			break;
		case MTNum:
			MyUSART_SendNum(Manager,*DataPoint);
			break;
		default:
			ret = 0;
			break;
	}
	return ret;
}

//获取缓存区数组指针
uint8_t * MyUSART_GetRxDataTempGroupPoint(USART_Manager * Manager)
{
	return Manager->RxDataTempGroup;
}


//缓冲区数组排序+添加数据进入缓冲区
void MyUSART_FormatTempGroup(USART_Manager * Manager,uint8_t data)
{
	uint8_t i=0;
	for(;i<Manager->RxDataTempMax-1;i++){
		Manager->RxDataTempGroup[i]=Manager->RxDataTempGroup[i+1];
	}
	Manager->RxDataTempGroup[i]=data;
}

//获取发送包状态位
uint8_t MyUSART_GetSendPackStatus(USART_Manager * Manager)
{
	return Manager->SendPackStatus;
}

//获取接受包状态位
uint8_t MyUSART_GetReceivePackStatus(USART_Manager * Manager)
{
	uint8_t temp=Manager->ReceivePackStatus;
	Manager->ReceivePackStatus=0;
	return temp;
}

//获取接受包临时状态位
uint8_t MyUSART_GetReceivePackStatusTemp(USART_Manager * Manager)
{
	return Manager->ReceivePackStatus;
}

//获取接受包错误状态位
uint8_t MyUSART_GetReceivePackErrorStatus(USART_Manager * Manager)
{
	uint8_t temp=Manager->ReceiveErrorStatus;
	Manager->ReceiveErrorStatus=0;
	return temp;
}

//获取接受包临时错误状态位
uint8_t MyUSART_GetReceivePackErrorStatusTemp(USART_Manager * Manager)
{
	return Manager->ReceiveErrorStatus;
}

//获取收包计数器计数
uint8_t MyUSART_GetRvCNT(USART_Manager * Manager)
{
	return Manager->RvCNT;
}

//打包发送
void MyUSART_SendPackage(USART_Manager * Manager,uint8_t* SdPackP)
{
	while(Manager->SendPackStatus);
	Manager->SendPackStatus=1;
	MyUSART_SendTxData(Manager,Manager->PackageTitle);
	for(uint8_t i=0;i<Manager->PackageLength;i++){
		MyUSART_SendTxData(Manager,SdPackP[i]);
	}
	MyUSART_SendTxData(Manager,Manager->PackageEnd);
	Manager->SendPackStatus=0;
}

//无规则发送
uint8_t MyUSART_SendUnlimited(USART_Manager * Manager,uint8_t *SdGroupP,uint8_t Length)
{
	MyUSART_SendTxData(Manager,Manager->PackageTitle);
	for(uint8_t i=0;i<Length;i++){
		MyUSART_SendTxData(Manager,SdGroupP[i]);
	}
	MyUSART_SendTxData(Manager,Manager->PackageEnd);
	return 1;
}

//清除接收包数组数据
void MyUSART_ClearRvPack(USART_Manager * Manager)
{
	for(uint8_t i=0;i<Manager->PackageLength;i++){
		Manager->ReceiveDataPackage[i]=0;
	}
}

//获取接收包数组地址
uint8_t *MyUSART_GetRvPackPoint(USART_Manager * Manager)
{
	return Manager->ReceiveDataPackage;
}

//调用时自动打包函数
void MyUSART_AutoPack(USART_Manager * Manager,uint8_t TempData)
{
	static uint8_t RvPackP = 0;
	static uint8_t RvPackStatus = 0;
		
		if(RvPackStatus==0){
			if(TempData==Manager->PackageTitle){
				RvPackP=0;
				Manager->ReceivePackStatus=0;
				Manager->ReceiveErrorStatus=0;
				MyUSART_ClearRvPack(Manager);
				RvPackStatus=1;
			}
		}else if(RvPackStatus==1){
			Manager->ReceiveDataPackage[RvPackP]=TempData;
			RvPackP++;
			if(RvPackP==Manager->PackageLength)RvPackStatus=2;
		}else if(RvPackStatus==2){
			if(TempData==Manager->PackageEnd){
				Manager->ReceivePackStatus=1;
				Manager->RvCNT++;
				RvPackStatus=0;
			}else{
				Manager->ReceiveDataPackage[Manager->PackageLength-1]=TempData;
				Manager->ReceiveErrorStatus=1;
			}
		}
}

//无规则填充式接收
void MyUSART_ReceiveUnlimit(USART_Manager * Manager,uint8_t * ContainerPoint,uint8_t Max)
{
	Manager->ReceiveErrorStatus=0;
	if(MyUSART_GetRxData(Manager)==Manager->PackageTitle){
		for(uint8_t i=0;i<Max;i++){
			Manager->ReceiveDataPackage[i]=MyUSART_GetRxData(Manager);
		}
	}
	if(MyUSART_GetRxData(Manager)!=Manager->PackageEnd){
		Manager->ReceiveErrorStatus=1;
	}
}

//清空字符串位置
void MyUSART_ClearString(USART_Manager * Manager)
{
	char temp[]="";
	strcpy(Manager->ReceiveString,temp);
}

//获取比较命令
uint8_t MyUSART_ScanString(USART_Manager * Manager,char* Command,uint8_t CommandCNT)
{
	uint8_t ret=0;
	if(CommandCNT<Manager->RxDataTempMax){
		char temp[CommandCNT+1];
		uint8_t i=Manager->RxDataTempMax-CommandCNT;
		for(uint8_t j=0;j<CommandCNT;j++){
			temp[j]=(char)Manager->RxDataTempGroup[i];
			i++;
		}
		temp[CommandCNT]=0;
		if(strcmp(temp,Command)==0)ret=1;
	}
	return ret;
}

//中断函数调用库
void MyUSART_ITConfig(USART_Manager * Manager)
{
	//串口获取数据
		uint8_t DataTemp =USART_ReceiveData(USART1);
		
		//填充数据
		Manager->RxData=DataTemp;
		MyUSART_FormatTempGroup(Manager,DataTemp);
		MyUSART_AutoPack(Manager,DataTemp);
		
		//挂灯
		Manager->RxStatus=1;
}

//中断函数
void USART1_IRQHandler(void){
	if(USART_GetITStatus(USART1,USART_IT_RXNE)==SET){
		MyUSART_ITConfig(USART1_Config);
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	}
}

void USART2_IRQHandler(void){
	if(USART_GetITStatus(USART2,USART_IT_RXNE)==SET){
		MyUSART_ITConfig(USART2_Config);
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);
	}
}

void USART3_IRQHandler(void){
	if(USART_GetITStatus(USART3,USART_IT_RXNE)==SET){
		MyUSART_ITConfig(USART3_Config);
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
	}
}
