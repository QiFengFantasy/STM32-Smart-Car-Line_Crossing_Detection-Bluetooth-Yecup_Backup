#include "stm32f10x.h"                  // Device header

/*以下是端口定义*/
const uint16_t	PWMAPin		=	GPIO_Pin_6;		//pwma端口
const uint16_t	PWMBPin		=	GPIO_Pin_7;		//pwmb端口
GPIO_TypeDef* 	PWMGPIO		=	GPIOA;

const uint16_t 	STBYPin		=	GPIO_Pin_14;
const uint16_t 	AIN1Pin		=	GPIO_Pin_8;		//注意：该pin接在gpioa
const uint16_t 	AIN2Pin 	=	GPIO_Pin_15;
const uint16_t 	BIN1Pin		=	GPIO_Pin_12;
const uint16_t 	BIN2Pin		=	GPIO_Pin_13;

GPIO_TypeDef* 	NormalGPIO	=	GPIOB;

GPIO_TypeDef* 	STBYGPIO    =	GPIOB;
GPIO_TypeDef*	AIN1GPIO	=	GPIOA;
GPIO_TypeDef* 	AIN2GPIO    =	GPIOB;
GPIO_TypeDef* 	BIN1GPIO    =	GPIOB;
GPIO_TypeDef* 	BIN2GPIO    =	GPIOB;

/*以下是修正量*/
const int8_t 	LeftFix		= 	0;
const int8_t	RightFix	=	6;

typedef enum {A = 0, B = 1} NeedleChoice;
typedef enum {L = 0, H = 1} TurnChoice;

void MPU_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin=PWMAPin | PWMBPin;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PWMGPIO,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin=BIN1Pin | BIN2Pin | STBYPin | AIN2Pin;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(NormalGPIO,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin=AIN1Pin;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(AIN1GPIO,&GPIO_InitStructure);
 	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period=200 - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler=18 - 1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState=ENABLE;
	TIM_OCInitStructure.TIM_Pulse=40;
	TIM_OC1Init(TIM3,&TIM_OCInitStructure);
	
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState=ENABLE;
	TIM_OCInitStructure.TIM_Pulse=40;
	TIM_OC2Init(TIM3,&TIM_OCInitStructure);
	
	TIM_Cmd(TIM3,ENABLE);
}

void MPU_SetPWMACPR(uint8_t CPR)
{
	CPR+=LeftFix;
	TIM_SetCompare1(TIM3,CPR);
}

void MPU_SetPWMBCPR(uint8_t CPR)
{
	CPR+=RightFix;
	TIM_SetCompare2(TIM3,CPR);
}

void MPU_SetSTBY(FunctionalState State)
{
	if(State){
		GPIO_SetBits(STBYGPIO,STBYPin);
	}else{
		GPIO_ResetBits(STBYGPIO,STBYPin);
	}
}

void MPU_SetIN(NeedleChoice Needle,FunctionalState State1,FunctionalState State2)
{
	if(Needle){
		if(State1){
			GPIO_SetBits(BIN1GPIO,BIN1Pin);
		}else{
			GPIO_ResetBits(BIN1GPIO,BIN1Pin);
		}
		
		if(State2){
			GPIO_SetBits(BIN2GPIO,BIN2Pin);
		}else{
			GPIO_ResetBits(BIN2GPIO,BIN2Pin);
		}
	}else{
		if(State1){
			GPIO_SetBits(AIN1GPIO,AIN1Pin);
		}else{
			GPIO_ResetBits(AIN1GPIO,AIN1Pin);
		}
		
		if(State2){
			GPIO_SetBits(AIN2GPIO,AIN2Pin);
		}else{
			GPIO_ResetBits(AIN2GPIO,AIN2Pin);
		}
	}
}

void MPU_NeedleHold(NeedleChoice Needle)
{
	MPU_SetIN(Needle,DISABLE,DISABLE);
}

void MPU_SetTurn(NeedleChoice Needle,TurnChoice Turn)
{
	if(Needle){
		if(Turn){
			MPU_SetIN(B,ENABLE,DISABLE);
		}else{
			MPU_SetIN(B,DISABLE,ENABLE);
		}
	}else{
		if(Turn){
			MPU_SetIN(A,ENABLE,DISABLE);
		}else{        
			MPU_SetIN(A,DISABLE,ENABLE);
		}
	}
}

void MPU_SetSpeed(uint8_t SpeedA,uint8_t SpeedB)
{
	MPU_SetPWMACPR(SpeedA);
	MPU_SetPWMBCPR(SpeedB);
}

void MPU_SetNeedleSpeed(uint8_t Needle,uint8_t Speed)
{
	if(Needle){
		MPU_SetPWMBCPR(Speed);
	}else{
		MPU_SetPWMACPR(Speed);
	}
}

void MPU_Start(void)
{
	MPU_SetSTBY(ENABLE);
	MPU_NeedleHold(A);
	MPU_NeedleHold(B);
}

void MPU_Stop(void)
{
	MPU_SetSTBY(DISABLE);
	MPU_NeedleHold(A);
	MPU_NeedleHold(B);
}

void MPU_SetSpeedAndTurn(int16_t SpeedA,int16_t SpeedB)
{
	if(SpeedA > 0){
		MPU_SetTurn(A,H);
		MPU_SetNeedleSpeed(A,SpeedA);
	}else if(SpeedA < 0){
		MPU_SetTurn(A,L);
		MPU_SetNeedleSpeed(A,-SpeedA);
	}else{
		MPU_NeedleHold(A);
		MPU_SetNeedleSpeed(A,0);
	}
	
	if(SpeedB > 0){
		MPU_SetTurn(B,H);
		MPU_SetNeedleSpeed(B,SpeedB);
	}else if(SpeedB < 0){
		MPU_SetTurn(B,L);
		MPU_SetNeedleSpeed(B,-SpeedB);
	}else{
		MPU_NeedleHold(B);
		MPU_SetNeedleSpeed(B,0);
	}
}





/*以下是用以便捷调试电机方向的函数*/

void MPU_AllAhead(void)
{
	MPU_SetTurn(A,H);
	MPU_SetTurn(B,H);
}

void MPU_LeftSideARightSideB(void)
{
	MPU_SetTurn(A,H);
	MPU_SetTurn(B,L);
}

void MPU_LeftSideBRightSideA(void)
{
	MPU_SetTurn(A,L);
	MPU_SetTurn(B,H);
}

void MPU_AllBehind(void)
{
	MPU_SetTurn(A,L);
	MPU_SetTurn(B,L);
} 
