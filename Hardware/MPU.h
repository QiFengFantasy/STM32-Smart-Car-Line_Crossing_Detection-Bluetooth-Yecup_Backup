#ifndef __MPU_H
#define __MPU_H

void MPU_Init(void);

void MPU_SetSpeed(uint8_t SpeedA,uint8_t SpeedB);

void MPU_Start(void);
void MPU_Stop(void);

void MPU_SetSpeedAndTurn(int16_t SpeedA,int16_t SpeedB);


void MPU_AllAhead(void);
void MPU_LeftSideARightSideB(void);
void MPU_LeftSideBRightSideA(void);
void MPU_AllBehind(void);


#endif
