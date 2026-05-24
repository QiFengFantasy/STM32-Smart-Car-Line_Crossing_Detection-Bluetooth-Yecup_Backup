#include "stm32f10x.h"                  // Device header

typedef enum{
	US_IDLE=0,
	US_SendingTrig,
	US_SendedTrig,
	US_GotEchoRising,
	US_GotEchoFalling,
	US_Completed,
}US_StateSetting;

typedef enum{
	WaitingRising=0,
	WaitingFalling,
}US_EchoStateSetting;

/*各种端口配置*/
const uint16_t	Trig_Pin	=	GPIO_Pin_1;
GPIO_TypeDef * 	Trig_GPIO	=	GPIOA;

const uint16_t	Echo_Pin	=	GPIO_Pin_0;
GPIO_TypeDef * 	Echo_GPIO	=	GPIOA;

TIM_TypeDef *	UsingTIM	=	TIM2;

#define	US_TIM_IRQHandler TIM2_IRQHandler

const uint16_t UsingTIMITChannel 	=	TIM_IT_CC1;

/*变量储存与配置*/

US_StateSetting US_State;		//状态位
US_EchoStateSetting Echo_State;

//时间记录
uint32_t TrigDuration=0;
uint32_t EchoDuration=0;

//超时记录
uint8_t EchoWaitingRisingErr=0;
uint8_t EchoWaitingFallingErr=0;

uint8_t US_CompletedCNT=0;

//常量规定
const float WaveVolyacity = 340;

float Distance=0;

void US_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin=Trig_Pin;					//Trig
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(Trig_GPIO,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin=Echo_Pin;					//Echo
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(Echo_GPIO,&GPIO_InitStructure);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period=0xFFFF;
	TIM_TimeBaseInitStructure.TIM_Prescaler=72 - 1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(UsingTIM,&TIM_TimeBaseInitStructure);
	
	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_ICInitStructure.TIM_Channel=TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICFilter=0;
	TIM_ICInitStructure.TIM_ICPolarity=TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICPrescaler=TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICSelection=TIM_ICSelection_DirectTI;
	TIM_ICInit(UsingTIM,&TIM_ICInitStructure);
	
	Echo_State=WaitingRising;								//标记等待
	
	TIM_ITConfig(UsingTIM,TIM_IT_Update,ENABLE);
	TIM_ITConfig(UsingTIM,UsingTIMITChannel,ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=3;
	NVIC_Init(&NVIC_InitStructure);
}

void US_SendTrig(void)
{
	if(US_State == US_IDLE){
		TIM_SetCounter(UsingTIM,0);
		TIM_SetAutoreload(UsingTIM,10);
		GPIO_SetBits(Trig_GPIO,Trig_Pin);
		TIM_Cmd(UsingTIM,ENABLE);
		
		US_State=US_SendingTrig;			//进入Trig计数
	}
}

uint8_t US_GetRisingErr(void)
{
	return EchoWaitingRisingErr;
}

uint8_t US_GetFallingErr(void)
{
	return EchoWaitingFallingErr;
}

uint8_t US_GetUSState(void)
{
	return US_State;
}

uint8_t US_GetCompletedCNT(void)
{
	return US_CompletedCNT;
}

void US_CalculateResult(void)
{
	if(US_State==US_GotEchoFalling){
		Distance = WaveVolyacity * EchoDuration / 2.0;
		US_State=US_Completed;
		US_CompletedCNT++;
	}
}

uint16_t US_GetDistance(void)
{
	if(US_State == US_Completed)US_State = US_IDLE;
	return (uint16_t)(Distance/100);					//返回毫米
}

void US_TIM_IRQHandler(void)
{
	if(TIM_GetITStatus(UsingTIM,TIM_IT_Update)){
		
		//trig发送结束
		if(US_State==US_SendingTrig){
			GPIO_ResetBits(Trig_GPIO,Trig_Pin);
			TIM_Cmd(UsingTIM,DISABLE);
			TIM_SetCounter(UsingTIM,0);
			
			US_State=US_SendedTrig;			//进入等待Echo
			
			TIM_SetAutoreload(UsingTIM,0xFFFF);		//对等待时间计时
			TIM_Cmd(UsingTIM,ENABLE);
		}
		
		//等待echo上升沿超时
		else if(US_State == US_SendedTrig){
			TIM_Cmd(UsingTIM,DISABLE);
			TIM_SetCounter(UsingTIM,0);
			US_State=US_IDLE;
			
			EchoWaitingRisingErr++;
		}
		
		//等待echo下降沿超时
		else if(US_State == US_GotEchoRising){
			TIM_Cmd(UsingTIM,DISABLE);
			TIM_SetCounter(UsingTIM,0);
			US_State=US_IDLE;
			
			EchoWaitingFallingErr++;
		}
		
		TIM_ClearITPendingBit(UsingTIM,TIM_IT_Update);
	}
	else if(TIM_GetITStatus(UsingTIM,UsingTIMITChannel)){
		
		//接收到echo上升沿
		
		if(US_State==US_SendedTrig){
			//记录等待时间
			TIM_Cmd(UsingTIM,DISABLE);
			TrigDuration=TIM_GetCounter(UsingTIM);
			
			//开始记录echo
			TIM_SetCounter(UsingTIM,0);
			TIM_SetAutoreload(UsingTIM,0xFFFF);
			TIM_Cmd(UsingTIM,ENABLE);
			
			//切换状态位
			US_State=US_GotEchoRising;
			
			//切换捕获模式
			TIM_OC1PolarityConfig(UsingTIM,TIM_ICPolarity_Falling);
		}
		
		//接收到Echo下降沿
		else if(US_State == US_GotEchoRising){
			//记录时间间隔
			TIM_Cmd(UsingTIM,DISABLE);
			EchoDuration=TIM_GetCounter(UsingTIM);
			
			//清空计数器
			TIM_SetCounter(UsingTIM,0);
			
			//切换状态位
			US_State=US_GotEchoFalling;
			
			//切换捕获模式
			TIM_OC1PolarityConfig(UsingTIM,TIM_ICPolarity_Rising);
		}
		
		
		TIM_ClearITPendingBit(UsingTIM,UsingTIMITChannel);
	}
}
