#include "stm32f10x.h"                  // Device header
#include "OS.h"

uint16_t Duration;
uint32_t LastTaskTick;
uint32_t TickTack=0;
uint8_t State=0;

const uint32_t 	Buzzer_Pin	=	GPIO_Pin_11;
GPIO_TypeDef *	Buzzer_GPIO	=	GPIOA;

void Buzzer_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin=Buzzer_Pin;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(Buzzer_GPIO,&GPIO_InitStructure);
	
	GPIO_SetBits(Buzzer_GPIO,Buzzer_Pin);
}

void Buzzer_Bzz(void)
{
	State=1;
	GPIO_ResetBits(Buzzer_GPIO,Buzzer_Pin);
}

void Buzzer_NoBzz(void)
{
	State=0;
	GPIO_SetBits(Buzzer_GPIO,Buzzer_Pin);
}

void Buzzer_AutoCheck(uint8_t Holdms)
{
	if(State==1){
		if(OS_GetOSTimeTick() - LastTaskTick > Duration){
			Buzzer_NoBzz();
			
			if(Holdms){
				State=2;
				Duration=Holdms;
				LastTaskTick=OS_GetOSTimeTick();
			}
		}
	}
	
	if(State==2){
		if(OS_GetOSTimeTick() - LastTaskTick > Duration){
			State=0;
		}
	}
}

void Buzzer_TickBzz(uint16_t ms)
{
	if(State==0){
		Buzzer_Bzz();
		Duration=ms;
		LastTaskTick=OS_GetOSTimeTick();
	}
}
