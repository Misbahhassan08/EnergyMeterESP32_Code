#include <AWS_IOT.h>
#include <WiFiUdp.h>
#include <WiFi.h>

///////////////////////////////////////////////////////////////////////////////////
#define WIFI_ENABLED

#define MUX_BASED_IRMS_CALCULATIONS
/////////////////////
// Pin Definitions //
/////////////////////
const int Mux_Pins[2][3] = {{25,26,27},{21, 23, 22}}; 
//const int zOutput = 15;
const int Mux_Out_Pins[2] = {36,33}; 

#define TOTAL_MUX_CTS    8
#define MUX_PRIMARY 0
#define MUX_SECONDARY 1

double muxIrms[TOTAL_MUX_CTS], muxadcBiasing[TOTAL_MUX_CTS];

enum PinConfigurations {
  IRMS_PHASE_1_PIN = 34,
  IRMS_PHASE_2_PIN = 35,
  IRMS_PHASE_3_PIN = 32,

};

enum Phases {
  PHASE_A,
  PHASE_B,
  PHASE_C,
  TOTAL_PHASES
};

double IadcBiasingValue[3], VadcBiasingValue[3] ;
uint8_t pinNumIadc[TOTAL_PHASES] = {IRMS_PHASE_1_PIN, IRMS_PHASE_2_PIN, IRMS_PHASE_3_PIN};
//uint8_t pinNumV[TOTAL_PHASES] = {VRMS_PHASE_1_PIN, VRMS_PHASE_2_PIN, VRMS_PHASE_3_PIN};

struct Parameters {
  double Irms, Vrms;
};
Parameters P[TOTAL_PHASES];
////////////////////////////////////////////////////////////////////////////////////
#ifdef WIFI_ENABLED

AWS_IOT hornbill;
// Mudassir Home...
//char WIFI_SSID[] = "Karachi_Home";
//char WIFI_PASSWORD[] = "@h@dbaig137";
// Mudassir Office...
//char WIFI_SSID[] = "Bilal Khan";
//char WIFI_PASSWORD[] = "032120884690";
// DHA OFFICE (Device 0002)...
char WIFI_SSID[] = "HUAWEI-j2eS";
char WIFI_PASSWORD[] = "kCNBHRuG";
char HOST_ADDRESS[] = "a3ssxa7ec6v567-ats.iot.ap-south-1.amazonaws.com";
char CLIENT_ID[] = "Baig1";
char TOPIC_NAME[] = "energy";

int status = WL_IDLE_STATUS;
int tick = 0, msgCount = 0, msgReceived = 0;
char payload[512];
char rcvdPayload[512];

void mySubCallBackHandler (char *topicName, int payloadLen, char *payLoad)
{
  strncpy(rcvdPayload, payLoad, payloadLen);
  rcvdPayload[payloadLen] = 0;
  msgReceived = 1;
}

#endif
char jsonBuff[500];


void setup() {
  Serial.begin(115200);
  delay(1000);
  //////////////////////////////////////////////////////////////
   for (int i=0; i<3; i++)
    {
      pinMode(Mux_Pins[0][i], OUTPUT);
      pinMode(Mux_Pins[1][i], OUTPUT);

    }
    pinMode(Mux_Out_Pins[0], INPUT); 
    pinMode(Mux_Out_Pins[1], INPUT); 
    
  pinMode(IRMS_PHASE_1_PIN, INPUT);
  pinMode(IRMS_PHASE_2_PIN, INPUT);
  pinMode(IRMS_PHASE_3_PIN, INPUT);

#ifdef WIFI_ENABLED
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // wait 5 seconds for connection:
    delay(5000);
  }

  Serial.println("Connected to wifi");
  
  if (hornbill.connect(HOST_ADDRESS, CLIENT_ID)== 0)
  {
    Serial.println("Connected to AWS");
    delay(1000);

    else
    {
      Serial.println("Subscribe Failed, Check the Thing Name and Certificates");
      while (1);
    }
  }
  else
  {
    Serial.println("AWS connection failed, Check the HOST Address");
    while (1);
  }
#endif// WIFI_ENABLED

 
|}
void loop() {

   
  calculateVoltCurrent(PHASE_A);
  calculateVoltCurrent(PHASE_B);
  calculateVoltCurrent(PHASE_C);

  MuX_Code();

if(Serial.available()){
  if(Serial.read() == 'C'){
    Serial.println("Calibrating ADCs");
      calculateADC_BiasingValue();
  }
}
  if ((millis() - prevTime) >= 1000){
     unsigned long thisTime;
     thisTime  = millis();
     thisTime /= 1000;
   
#ifdef WIFI_ENABLED
     sprintf(jsonBuff, "{\"Device_Id\":\"0002\",\"time\":%ld,\"V\":[%f,%f,%f],\"C\":[%.1f,%.1f,%.1f],\"X\":[%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f]}\r\n", tmstmp,P[PHASE_A].Vrms, P[PHASE_B].Vrms, P[PHASE_C].Vrms,
            P[PHASE_A].Irms, P[PHASE_B].Irms, P[PHASE_C].Irms, muxIrms[0], muxIrms[1], muxIrms[2], muxIrms[3], muxIrms[4], muxIrms[5], muxIrms[6], muxIrms[7]);
          
     sendDataToCloud();
            		   
     prevTime = millis();
  }


}
///////////////////////////////////////////////////// When WiFi ON////////////////////////////////////////////////////////////////////////////////
#ifdef WIFI_ENABLED
static float sclaerVrms[TOTAL_PHASES] = {1, 1, 1}, scalerIrms[TOTAL_PHASES] = {1, 1, 1}; 



void calculateVoltCurrent(uint8_t phase) {

  static uint16_t zcCount[TOTAL_PHASES] = {0, 0, 0}, adcCount[TOTAL_PHASES] = {0, 0, 0};
  bool zcTransition[TOTAL_PHASES] = {false, false, false};
  static bool  prevZCTransition[TOTAL_PHASES] = {false, false, false};
  int Iadc[TOTAL_PHASES], Vadc[TOTAL_PHASES];
  static float IfilteredAdc[TOTAL_PHASES] = {0,0,0};
  static float vSum[TOTAL_PHASES], iSum[TOTAL_PHASES];
  float iSqr[TOTAL_PHASES], vSqr[TOTAL_PHASES];
  float rawVrms, rawIrms;
  static unsigned long prevTime[TOTAL_PHASES] ={0, 0, 0};
  
  selectMuxPin(MUX_PRIMARY, phase);
  Vadc[phase] = analogRead(Mux_Out_Pins[MUX_PRIMARY]);
  #ifdef MUX_BASED_IRMS_CALCULATIONS
    selectMuxPin(MUX_PRIMARY, phase+3);
    Iadc[phase] = analogRead(Mux_Out_Pins[MUX_PRIMARY]);
  #else
  Iadc[phase] = analogRead(pinNumIadc[phase]);
  #endif//MUX_BASED_IRMS_CALCULATIONS
  
//  IfilteredAdc[phase] = IfilteredAdc[phase]*0.5 + Iadc[phase]*0.5;
//  if(phase == 0){
//   Serial.print(Vadc[phase]);
//   Serial.print(",");
//   Serial.println(Iadc[phase]);
//  }
  adcCount[phase]++;

  if (Vadc[phase] < (VadcBiasingValue[phase] + 100) && Vadc[phase] > (VadcBiasingValue[phase] - 100)) {
    zcTransition[phase] = true;
  }


  if (prevZCTransition[phase] ==  true && zcTransition[phase] == false ) {
    zcCount[phase]++;
  }
  prevZCTransition[phase] = zcTransition[phase];

  Iadc[phase] = Iadc[phase] - IadcBiasingValue[phase];
  Vadc[phase] = Vadc[phase] - VadcBiasingValue[phase];



  iSqr[phase] = Iadc[phase] * Iadc[phase];
  vSqr[phase] = Vadc[phase] * Vadc[phase];

  iSum[phase] += iSqr[phase];
  vSum[phase] += vSqr[phase];

  if (zcCount[phase] >= 5) {
    iSum[phase] = iSum[phase] / ((float)adcCount[phase]);
    vSum[phase] = vSum[phase] / ((float)adcCount[phase]);

    rawIrms = sqrt(iSum[phase]);
    rawIrms  *= scalerIrms[phase];

    rawVrms = sqrt(vSum[phase]);
    rawVrms  *= sclaerVrms[phase];

    //IIR filter....
    
//          P[phase].Vrms =P[phase].Vrms*0.1 + rawVrms*0.1;
          P[phase].Irms = P[phase].Irms*0.5 + rawIrms*0.5;
    P[phase].Vrms = rawVrms;
//    P[phase].Irms = rawIrms;
//       Serial.print(P[phase].Vrms);

    vSum[phase] = 0;
    iSum[phase] = 0;
    adcCount[phase] = 0;
    zcCount[phase] = 0;
    prevTime[phase] =millis();
  }

  if((millis() - prevTime[phase]) >= 600 ){
     P[phase].Irms = 0;
     P[phase].Vrms = 0;
    prevTime[phase] =millis();
  }
//  
  
  //    Serial.print("P");
  //    Serial.print(phase+1);
  //    Serial.print("-->");
  //    Serial.print(" ACnt :");
  //    Serial.print(adcCount[phase]);
  //    Serial.print(", ZCnt : ");
  //    Serial.print(zcCount[phase]);
  //    Serial.print(", Adc : ");
  //    Serial.println(Vadc[phase]);
  //    Serial.print(", ZC : ");
  //    Serial.print(zcCount);
  //    Serial.print("C : ");
  //    Serial.print(adcCount);
  //    Serial.println();

}

void calculateADC_BiasingValue() {
  IadcBiasingValue[0] = 0;
  IadcBiasingValue[1] = 0;
  IadcBiasingValue[2] = 0;
  VadcBiasingValue[0] = 0;
  VadcBiasingValue[1] = 0;
  VadcBiasingValue[2] = 0;
  unsigned long prevTime= millis();
  unsigned long count1 = 0, count2 = 0;
  uint8_t phaseNum=0;
  uint8_t ctNum=0;
  bool timeOut = false;

while(timeOut == false || phaseNum != 0 || ctNum != 0){

if(millis() - prevTime >= 5000){
  timeOut = true;
}
//  for (int j = 0; j < 3; j++) {
//    for (int i = 0; i < 3000; i++) {
    if(timeOut == false || phaseNum != 0){
      selectMuxPin(MUX_PRIMARY, phaseNum);
      VadcBiasingValue[phaseNum] += analogRead(Mux_Out_Pins[MUX_PRIMARY]);
      selectMuxPin(MUX_PRIMARY, phaseNum+3);
      IadcBiasingValue[phaseNum] += analogRead(Mux_Out_Pins[MUX_PRIMARY]);
      phaseNum++;
      phaseNum %= 3;
      count1++;
    }
}

    
  for (int j = 0; j < 3; j++) {
    Serial.print(" IadcBiasingValue[");
    Serial.print(j);
    Serial.print("]");
    Serial.println(IadcBiasingValue[j]);
    Serial.print(" VadcBiasingValue[");
    Serial.print(j);
    Serial.print("]");
    Serial.println(VadcBiasingValue[j]);
    
    IadcBiasingValue[j] /= count1;
    VadcBiasingValue[j] /= count1;
    
    Serial.print("I-Biasing ");
    Serial.print(j + 1);
    Serial.print(" : ");
    Serial.print(IadcBiasingValue[j]);
    Serial.print(", V-Biasing ");
    Serial.print(j + 1);
    Serial.print(" : ");
    Serial.println(VadcBiasingValue[j]);
  }

    Serial.print(" count2: ");
    Serial.println(count2);
for(int j=0; j<TOTAL_MUX_CTS; j++){
    Serial.print(" muxadcBiasing[");
    Serial.print(j);
    Serial.print("]");
    Serial.println(muxadcBiasing[j]);
    
    muxadcBiasing[j] /= count2;
      Serial.print(muxadcBiasing[j]);
      Serial.print(" , ");
}
  Serial.println();
}

/*
void calculateADC_BiasingValue() {
  IadcBiasingValue[0] = 0;
  IadcBiasingValue[1] = 0;
  IadcBiasingValue[2] = 0;
  VadcBiasingValue[0] = 0;
  VadcBiasingValue[1] = 0;
  VadcBiasingValue[2] = 0;
unsigned long prevTime= millis();

while(millis() - prevTime <= 5000){
  for (int j = 0; j < 3; j++) {
    for (int i = 0; i < 3000; i++) {
      selectMuxPin(MUX_PRIMARY, j);
      delayMicroseconds(50);
      VadcBiasingValue[j] += analogRead(Mux_Out_Pins[MUX_PRIMARY]);
      selectMuxPin(MUX_PRIMARY, j+3);
      delayMicroseconds(50);
      IadcBiasingValue[j] += analogRead(Mux_Out_Pins[MUX_PRIMARY]);
    }
  }

  for (int j = 0; j < 3; j++) {
    IadcBiasingValue[j] /= 3000;
    VadcBiasingValue[j] /= 3000;
    Serial.print("I-Biasing ");
    Serial.print(j + 1);
    Serial.print(" : ");
    Serial.print(IadcBiasingValue[j]);
    Serial.print(", V-Biasing ");
    Serial.print(j + 1);
    Serial.print(" : ");
    Serial.println(VadcBiasingValue[j]);
  }
    for(int i=0; i<TOTAL_MUX_CTS; i++){
    selectMuxPin(MUX_SECONDARY, i);
    for(int j=0;j<3000; j++){
    muxadcBiasing[i] += analogRead(Mux_Out_Pins[MUX_SECONDARY]);
    //delay(1);
  }
  }
  for(int j=0; j<TOTAL_MUX_CTS; j++){
    muxadcBiasing[j] /= 3000;
      Serial.print(muxadcBiasing[j]);
      Serial.print(" , ");
}
  Serial.println();
}
}
*/

void PrintData() {

  for (int i = 0; i < TOTAL_PHASES; i++) {
    Serial.print("P");
    Serial.print(i + 1);
    Serial.print("-->");
    Serial.print("C");
    Serial.print(" : ");
    Serial.print(P[i].Irms);
    Serial.print(", V");
    Serial.print(" : ");
    Serial.print(P[i].Vrms);
    Serial.println();
  }

    for(int i = 0; i<TOTAL_MUX_CTS; i++){
    Serial.print("MUX");
    Serial.print(i+1);
    Serial.print("-->");
    Serial.print("C");
    Serial.print(" : ");
    Serial.print(muxIrms[i]);
    Serial.println();
    }
  

}

#ifdef WIFI_ENABLED

void sendDataToCloud() {
  if (msgReceived == 1)
  {
      msgReceived = 0;
 
  }

    // sprintf(payload,"Hello from baig ESP32 : %d",msgCount++);
    // sprintf (payload,"V [%f,%f,%f],C [%f,%f,%f]",P[PHASE_A].Vrms,P[PHASE_B].Vrms,P[PHASE_C].Vrms,P[PHASE_A].Irms,P[PHASE_B].Irms,P[PHASE_C].Irms);

    //        char json[] = "{\"Device Id\":0001,\"time\":1351824120,\"data\:[Voltage (V1 , V2 , V3) Current (I1, I2 , I3) CTs (CT1 , CT2 , CT3 , CT4 , CT5 , CT6 , CT7 , CT8)}";
    //        char json[] = "{\"Device Id\":0001,\"time\":1351824120,\"V\":[V1,V2,V3],\"C\":[I1,I2,I3],\"X\":[CT1,CT2,CT3,CT4,CT5,CT6,CT7,CT8]}";

    sprintf (payload, jsonBuff);
    //payload =  {
    //  "timestamp": [1312121212],
    //  "harmonics150": [140, 15, 7, 111, 13, 7, 133, 4, 7, 124, 15, 7, 143, 3, 7, 136, 13, 7, 141, 6, 7, 142, 10, 7, 131, 9, 7, 134, 9, 7, 138, 4, 7, 133, 6, 7, 123, 5, 7, 126, 0, 7, 122, 5, 7, 133, 5, 7, 134, 8, 7, 120, 28, 7, 137, 6, 7, 161, 33, 8, 140, 15, 7, 111, 13, 7, 133, 4, 7, 124, 15, 7, 143, 3, 7],
    //  "harmonics250": [43, 2, 2, 44, 1, 2, 41, 3, 2, 41, 0, 2, 40, 2, 2, 41, 2, 2, 39, 2, 2, 42, 1, 2, 41, 3, 2, 40, 3, 2, 38, 2, 2, 39, 2, 2, 39, 2, 2, 42, 1, 2, 42, 2, 2, 40, 1, 2, 41, 4, 2, 46, 2, 2, 39, 2, 2, 45, 3, 2, 43, 2, 2, 44, 1, 2, 41, 3, 2, 41, 0, 2, 40, 2, 2],
    //  "harmonics350": [27, 2, 1, 29, 0, 0, 28, 1, 1, 27, 2, 0, 30, 2, 0, 28, 2, 0, 29, 1, 0, 30, 1, 0, 29, 1, 0, 28, 3, 0, 27, 2, 0, 28, 2, 0, 29, 1, 0, 29, 0, 0, 28, 2, 0, 27, 2, 0, 26, 3, 0, 24, 5, 0, 28, 2, 0, 29, 6, 2, 27, 2, 1, 29, 0, 0, 28, 1, 1, 27, 2, 0, 30, 2, 0],
    //  "harmonics50": [22, 2, 2, 20, 1, 1, 21, 1, 1, 23, 0, 1, 23, 1, 1, 22, 2, 1, 22, 0, 1, 22, 1, 1, 22, 1, 1, 21, 1, 1, 22, 1, 0, 21, 1, 0, 22, 1, 0, 20, 0, 0, 23, 1, 0, 22, 1, 0, 22, 0, 1, 25, 1, 0, 22, 1, 0, 20, 1, 2, 22, 2, 2, 20, 1, 1, 21, 1, 1, 23, 0, 1, 23, 1, 1],
    //  "power": [347, 0, 0, 347, 0, 0, 246, 0, 0, 246, 0, 0, 246, 0, 0, 246, 0, 0, 246, 0, 0, 246, 0, 0, 246, 0, 0, 246, 0, 0, 246, 0, 0, 246, 0, 0, 246, 0, 0, 246, 0, 0, 246, 0, 0, 246, 0, 0, 246, 0, 0, 245, 0, 0, 246, 0, 0, 349, 0, 0, 347, 0, 0, 347, 0, 0, 246, 0, 0, 246, 0, 0, 246, 0, 0],
    //  "voltage": [246, 2, 2, 246, 1, 2, 246, 1, 1, 246, 1, 1, 246, 1, 1, 246, 1, 1, 246, 1, 1, 246, 1, 1, 246, 1, 1, 246, 1, 1, 246, 0, 0, 246, 0, 0, 246, 0, 0, 246, 0, 0, 246, 0, 0, 246, 0, 0, 246, 0, 0, 245, 1, 1, 246, 0, 0, 247, 1, 1, 246, 2, 2, 246, 1, 2, 246, 1, 1, 246, 1, 1, 246, 1, 1],
    //  "current": [1.4140000343, 0.0, 0.0, 1.4140000343, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.4140000343, 0.0, 0.0, 1.4140000343, 0.0, 0.0, 1.4140000343, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0]
    //};


    if (hornbill.publish(TOPIC_NAME, payload) == 0)
    {
      Serial.print("Publish Message:");
      Serial.println(payload);
    }
    else 
    {
      Serial.println("Publish failed");
      
    
    }
  
}
#endif
#ifdef WIFI_ENABLED
/////////////////////////////////////////When WiFi ON////////////////////////////////////////////////////
static float  scalerMuxIrms[TOTAL_MUX_CTS]={1,1,1,1,1,1,1,1};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void calcIrmsMux(uint8_t muxPin ){
  static uint16_t zcCount[TOTAL_MUX_CTS] = {0,0,0,0,0,0,0,0}, adcCount[TOTAL_MUX_CTS] = {0,0,0,0,0,0,0,0};
  bool zcTransition[TOTAL_MUX_CTS] = {false,false,false,false,false,false,false,false};
  static bool  prevZCTransition[TOTAL_MUX_CTS] = {false,false,false,false,false,false,false,false};
  int Iadc[TOTAL_MUX_CTS];
  static float iSum[TOTAL_MUX_CTS];
  float iSqr[TOTAL_MUX_CTS];
  float  rawIrms = 0;
  static unsigned long prevTime[TOTAL_MUX_CTS] = {0,0,0,0,0,0,0,0};

    Iadc[muxPin] = analogRead(Mux_Out_Pins[MUX_SECONDARY]);
    adcCount[muxPin]++;


    if(Iadc[muxPin] < (muxadcBiasing[muxPin]) && Iadc[muxPin] > (muxadcBiasing[muxPin])){
      zcTransition[muxPin] = true;
    }
    else{
      zcTransition[muxPin] = false;
    }

    if(prevZCTransition[muxPin] ==  true && zcTransition[muxPin] == false ){
      zcCount[muxPin]++;
    }
    prevZCTransition[muxPin] = zcTransition[muxPin];


    Iadc[muxPin] = Iadc[muxPin]- muxadcBiasing[muxPin];


   

    iSqr[muxPin] = Iadc[muxPin]*Iadc[muxPin];

    iSum[muxPin] += iSqr[muxPin];
  //  Serial.println(zcCount[muxPin]);//-------------------------------------------------------------------

    if(zcCount[muxPin] >= 5){
      iSum[muxPin] = iSum[muxPin]/((float)adcCount[muxPin]);

      rawIrms = sqrt(iSum[muxPin]);

    //IIR filter....

      muxIrms[muxPin] = muxIrms[muxPin]*0.5 + rawIrms*scalerMuxIrms[muxPin]*0.5;
   
      



//          Serial.print("P");
//          Serial.print(muxPin+1);
//          Serial.print("-->");
//          Serial.print(" ACnt :");
//          Serial.print(adcCount[muxPin]);
//          Serial.print(", ZCnt : ");
//          Serial.print(zcCount[muxPin]);
        //  Serial.print(", Adc : ");
          //Serial.println(Vadc[muxPin]);
          //Serial.print(", ZC : ");
    //  Serial.print(zcCount);
//      Serial.println();

      iSum[muxPin] = 0;
      adcCount[muxPin] = 0;
      zcCount[muxPin]=0;
      prevTime[muxPin] = millis();
    }

   
    
      if((millis() - prevTime[muxPin]) >= (600)){
        muxIrms[muxPin] = 0;
        prevTime[muxPin] = millis();
    }
    

}

// The selectMuxPin function sets the S0, S1, and S2 pins
// accordingly, given a pin from 0-7.

void MuX_Code(void)
{
  static unsigned long prevTime=0;

  // Loop through all eight pins.
//  if((micros() - prevTime) >= 500){
   for (int  pin=0; pin<8; pin++)
   {
     selectMuxPin(MUX_SECONDARY, pin); // Select one at a time
//     selectMuxPin(5); // Select one at a time
   //  int inputValue = analogRead(Mux_Out_Pins); // and read Z
//      calcIrmsMux(5);
      calcIrmsMux(pin);
  //   Serial.print("\t");
 
   }
  prevTime = micros();
//}
//  
}

void selectMuxPin(uint8_t muxNum, byte pin)
{
  for (int i=0; i<3; i++)
  {
    if (pin & (1<<i))
      digitalWrite(Mux_Pins[muxNum][i], HIGH);
    else
      digitalWrite(Mux_Pins[muxNum][i], LOW);
  }

}