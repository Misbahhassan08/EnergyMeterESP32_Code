// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _Power_Meter_H_
#define _Power_Meter_H_
#include "Arduino.h"
//add your includes for the project Power_Meter here
#include "config.h"

//end of add your includes here


//add your function definitions for the project Energy_Meter here
float calcIRMS1(uint8_t pin);
float calcIRMS2(uint8_t pin);
float calcIRMS3(uint8_t pin);
void calculateADC_BiasingValue_1(uint8_t pin);
void calculateADC_BiasingValue_2(uint8_t pin);
void calculateADC_BiasingValue_3(uint8_t pin);


//end of add your includes here


//add your function definitions for the project Power_Meter here




//Do not add code below this line
#endif /* _Power_Meter_H_ */
