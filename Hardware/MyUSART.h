#ifndef __MYUSART_H
#define __MYUSART_H

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

void MyUSART_ClearRvPack(USART_Manager * Manager);
uint8_t MyUSART_ManagerInit(USART_Manager * Manager);

void MyUSART_Init(USART_Manager * Manager);

uint8_t MyUSART_ManagerInit(USART_Manager * Manager);
void MyUSART_Manager_Deinit(USART_Manager* Manager);

uint8_t MyUSART_GetTxStatus(USART_Manager * Manager);
uint8_t MyUSART_GetTxData(USART_Manager * Manager);
void MyUSART_SendTxData(USART_Manager * Manager,uint8_t Data);
uint8_t MyUSART_GetRxStatus(USART_Manager * Manager);
uint8_t MyUSART_GetRxData(USART_Manager * Manager);
uint8_t MyUSART_GetRxDataTemp(USART_Manager * Manager);

void MyUSART_ArrU8CPY(uint8_t *ArrSource,uint8_t *ArrTarget,uint8_t length);
void MyUSART_StringCPY(char *ArrSource,char *ArrTarget);
int32_t MyUSART_Pow(int32_t Num,uint8_t Times);

void MyUSART_SendArray(USART_Manager * Manager,uint8_t Array[],uint16_t Length);
void MyUSART_SendString(USART_Manager * Manager,char *String);
void MyUSART_SendNum(USART_Manager * Manager,int32_t Num);
void MyUSART_SendNumStr(USART_Manager * Manager,int32_t Num);

void MyUSART_AddEnter(USART_Manager * Manager);
uint8_t MyUSART_SendMutipleType(USART_Manager * Manager,enum MutipleType Type,uint8_t *DataPoint,uint32_t length);

uint8_t * MyUSART_GetRxDataTempGroupPoint(USART_Manager * Manager);
void MyUSART_FormatTempGroup(USART_Manager * Manager,uint8_t data);

uint8_t MyUSART_GetSendPackStatus(USART_Manager * Manager);
uint8_t MyUSART_GetReceivePackStatus(USART_Manager * Manager);
uint8_t MyUSART_GetReceivePackStatusTemp(USART_Manager * Manager);
uint8_t MyUSART_GetReceivePackErrorStatus(USART_Manager * Manager);
uint8_t MyUSART_GetReceivePackErrorStatusTemp(USART_Manager * Manager);

uint8_t MyUSART_GetRvCNT(USART_Manager * Manager);

void MyUSART_SendPackage(USART_Manager * Manager,uint8_t* SdPackP);
uint8_t MyUSART_SendUnlimited(USART_Manager * Manager,uint8_t *SdGroupP,uint8_t Length);

void MyUSART_ClearRvPack(USART_Manager * Manager);

uint8_t *MyUSART_GetRvPackPoint(USART_Manager * Manager);

void MyUSART_AutoPack(USART_Manager * Manager,uint8_t TempData);

void MyUSART_ReceiveUnlimit(USART_Manager * Manager,uint8_t * ContainerPoint,uint8_t Max);

void MyUSART_ClearString(USART_Manager * Manager);
uint8_t MyUSART_ScanString(USART_Manager * Manager,char* Command,uint8_t CommandCNT);

void MyUSART_ITConfig(USART_Manager * Manager);

#endif
