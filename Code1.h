#ifndef Code1_H
#define Code1_H
#include "Arduino.h"
//add your includes for the project Power_Meter here
#include "config.h"

//add your function definitions for the project Energy_Meter here
float calcIRMS1(uint8_t pin);
float calcIRMS2(uint8_t pin);
float calcIRMS3(uint8_t pin);
void calculateADC_BiasingValue_1(uint8_t pin);
void calculateADC_BiasingValue_2(uint8_t pin);
void calculateADC_BiasingValue_3(uint8_t pin);

//add your function definitions for the project Energy_Meter here
float calcIRMS1(uint8_t pin);
float calcIRMS2(uint8_t pin);
float calcIRMS3(uint8_t pin);
void calculateADC_BiasingValue_1(uint8_t pin);
void calculateADC_BiasingValue_2(uint8_t pin);
void calculateADC_BiasingValue_3(uint8_t pin);

void connectToWiFi(void);
void connectToCertf(void);
void sendDataToCloud(void);
void connectToAWS(void);
void callback(String &topic, String &payload);
void gettingData( void * pvParameters );
void sendingData( void * pvParameters );

#endif
