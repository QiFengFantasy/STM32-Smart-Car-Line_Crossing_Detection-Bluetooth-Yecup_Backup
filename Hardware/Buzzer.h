#ifndef __BUZZER_H
#define __BUZZER_H

void Buzzer_Init(void);
void Buzzer_Bzz(void);
void Buzzer_NoBzz(void);
void Buzzer_TickBzz(uint16_t ms);
void Buzzer_AutoCheck(uint8_t Holdms);


#endif
