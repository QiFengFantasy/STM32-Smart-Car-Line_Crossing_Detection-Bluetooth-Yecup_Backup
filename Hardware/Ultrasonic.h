#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H

void US_Init(void);

void US_SendTrig(void);

uint8_t US_GetRisingErr(void);
uint8_t US_GetFallingErr(void);
uint8_t US_GetUSState(void);
uint8_t US_GetCompletedCNT(void);

void US_CalculateResult(void);

uint16_t US_GetDistance(void);


#endif
