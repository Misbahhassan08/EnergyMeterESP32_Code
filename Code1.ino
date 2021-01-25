
#include "SPIFFS.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include "Code1.h"

#define WIFI_ENABLED
#define MUX_BASED_IRMS_CALCULATIONS
#define TOTAL_MUX_CTS    8
#define MUX_PRIMARY 0
#define MUX_SECONDARY 1
TaskHandle_t _senseData;
TaskHandle_t _postData;

String Read_rootca;
String Read_cert;
String Read_privatekey;
String timestamp = String("");

const int Mux_Pins[2][3] = {{25,26,27},{21, 23, 22}}; 
const int Mux_Out_Pins[2] = {36,33}; 


double muxIrms[TOTAL_MUX_CTS], muxadcBiasing[TOTAL_MUX_CTS];

enum Phases {
  PHASE_A,
  PHASE_B,
  PHASE_C,
  TOTAL_PHASES
};



JsonArray Info_c1_V;
JsonArray Info_c1_C;
JsonArray Info_c1_X; 

boolean sampleTime = false;
boolean active = true;
double IadcBiasingValue[3], VadcBiasingValue[3] ;
uint8_t pinNumIadc[TOTAL_PHASES] = {IRMS_PHASE_1_PIN, IRMS_PHASE_2_PIN, IRMS_PHASE_3_PIN};
struct Parameters {
  double Irms, Vrms;
};
Parameters P[TOTAL_PHASES];
////////////////////////////////////////////////////////////////////////////////////
// Wifi credentials
const char *WIFI_SSID = "Alvi's";
const char *WIFI_PASSWORD = "RajaG121";

#define AWS_IOT_ENDPOINT "a3ssxa7ec6v567-ats.iot.ap-southeast-1.amazonaws.com"
#define AWS_MAX_RECONNECT_TRIES 50
#define PORT 8883
#define AWS_IOT_TOPIC "esp_OUT"
#define AWS_IOT_SUB "esp_IN"



String DEVICE_NAME = String("d2");
unsigned short SAMPLE = 50;
unsigned short mqtt_Buffer = 3256;
unsigned const short cloud_buffer = 3200;

//8kb still left 
WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(mqtt_Buffer);
int status = WL_IDLE_STATUS;
int tick = 0, msgCount = 0, msgReceived = 0;
long lastMsg = 0;
void calculateVoltCurrent(uint8_t);
void MuX_Code(void); 
void calculateADC_BiasingValue(void);


const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 18000;// +5 x 3600
const int   daylightOffset_sec = 3600;


////////////////////////////////////////////////////////////////////////////////////
// SETUP
void setup() {
  Serial.begin(115200);
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
    Serial.println("Setup Mode Start : ");
  connectToWiFi();
  delay(1000);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  connectToCertf();
  delay(2000);
  client.begin(AWS_IOT_ENDPOINT, PORT, net);
  client.onMessage(callback);
  delay(2000);
  while(1)
  {
    if (!client.connected()) {
       // Loop until we're reconnected
        while (!client.connect("arduino", "try", "try")) {
          Serial.print(".");
          delay(100);
        }
        
        Serial.println("AWS IoT Connected!");
        }
    else break;
  }
  client.subscribe(AWS_IOT_SUB);
  Serial.println(getTime());
  delay(2000);
  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    gettingData,   
                    "Task1",     
                    10000,       
                    NULL,       
                    1,          
                    &_senseData,     
                    0);                          
  delay(500); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
 xTaskCreatePinnedToCore(
                    sendingData,   
                    "Task2",     
                    10000,       
                    NULL,       
                    1,          
                    &_postData,     
                    1);         
  delay(500); 


}


////////////////////////////////////////////////////////////////////////////////////
// LOOP()

void loop() {
  //PrintData();
 if(active) 
 {
  active = false;
  delay(1000);
  }
 else if(active == false)
 {
  active = true;
  delay(1000);
  }
 client.loop();
}


////////////////////////////////////////////////////////////////////////////////////
// WIFI CONNECT

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Only try 15 times to connect to the WiFi
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 15){
    delay(500);
    Serial.print(".");
    retries++;
  }
  // If we still couldn't connect to the WiFi, go to deep sleep for a minute and try again.
  if(WiFi.status() != WL_CONNECTED){
    esp_sleep_enable_timer_wakeup(1 * 60L * 1000000L);
    esp_deep_sleep_start();
  }
  else
  {
    Serial.println("WIFI CONECTED");
    }
}
////////////////////////////////////////////////////////////////////////////////////
// GET TIME 

String getTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "Failed to obtain time";
  }
  return asctime(&timeinfo);
}

////////////////////////////////////////////////////////////////////////////////////
// Call back response from Cloud
/*
 incoming: esp_IN - {
  "samplesize": "10"
  "deviceNo": "3"
}
SAMPLE , DEVICE_NAME
//serializeJsonPretty(doc, Serial);
  //Serial.println("Json-Size: ");
  //Serial.println(_mem);
  Serial.println("Publishing message to AWS...");
  //serializeJson(doc, Serial);
  char jsonBuffer[cloud_buffer];
  serializeJson(doc, jsonBuffer);
  Serial.println(jsonBuffer);
 */


void callback(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  StaticJsonDocument<200> docr;
  DeserializationError error = deserializeJson(docr, payload);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  else
  {
    unsigned short _sample = docr["samplesize"];
    SAMPLE = _sample;
    }
}



////////////////////////////////////////////////////////////////////////////////////
// Getting data from sensors Thread function

void gettingData( void * pvParameters ){
  Serial.print("Task1 running on core Getting values ");
  Serial.println(xPortGetCoreID());
  while(1)
  {
    vTaskDelay( pdMS_TO_TICKS( 10 ) );
    calculateVoltCurrent(PHASE_A);
    calculateVoltCurrent(PHASE_B);
    calculateVoltCurrent(PHASE_C);
    MuX_Code();
    if(Serial.available()){
      if(Serial.read() == 'C'){
        active = false;
        Serial.println("Calibrating ADCs");
        calculateADC_BiasingValue();
      }
    }
  }
}

//Task2code:
void sendingData( void * pvParameters ){
  Serial.print("Task2 running on core Sending Values ");
  Serial.println(xPortGetCoreID());
  double prevTime = 0.00;
  while(1)
  {
    vTaskDelay( pdMS_TO_TICKS( 10 ) );
    if(active) // no callibration in process 
    {
      sendDataToCloud();
    }
    
    }
}




///////////////////////////////////////////////////// When WiFi ON////////////////////////////////////////////////////////////////////////////////

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
  #ifndef MUX_BASED_IRMS_CALCULATIONS
    selectMuxPin(MUX_PRIMARY, phase+3);
    Iadc[phase] = analogRead(Mux_Out_Pins[MUX_PRIMARY]);
  #else
  Iadc[phase] = analogRead(pinNumIadc[phase]);
  #endif//MUX_BASED_IRMS_CALCULATIONS
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
//        P[phase].Vrms =P[phase].Vrms*0.1 + rawVrms*0.1;
          P[phase].Irms = P[phase].Irms*0.5 + rawIrms*0.5;
          P[phase].Vrms = rawVrms;
//        P[phase].Irms = rawIrms;
//        Serial.print(P[phase].Vrms);

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



void sendDataToCloud() {
   timestamp = getTime();
   JsonObject Info[SAMPLE];
   DynamicJsonDocument doc(ESP.getMaxAllocHeap());
  doc["D_Id"] = DEVICE_NAME;
  doc["ttp"] = timestamp;
  JsonArray Info_doc = doc.createNestedArray("Info");
  for(int i = 1; i<=SAMPLE ; i++)
  {
    /*
    Info[i] = Info_doc.createNestedObject();
    Info_c1_V = Info[i].createNestedArray("V");
    Info_c1_V.add(P[PHASE_A].Vrms);
    Info_c1_V.add(P[PHASE_B].Vrms);
    Info_c1_V.add(P[PHASE_C].Vrms);
    Info_c1_C = Info[i].createNestedArray("C");
    Info_c1_C.add(P[PHASE_A].Irms);
    Info_c1_C.add(P[PHASE_B].Irms);
    Info_c1_C.add(P[PHASE_C].Irms);
    Info_c1_X = Info[i].createNestedArray("X");
    Info_c1_X.add(muxIrms[0]);
    Info_c1_X.add(muxIrms[1]);
    Info_c1_X.add(muxIrms[2]);
    Info_c1_X.add(muxIrms[3]);
    Info_c1_X.add(muxIrms[4]);
    Info_c1_X.add(muxIrms[5]);
    Info_c1_X.add(muxIrms[6]);
    Info_c1_X.add(muxIrms[7]);
   */
    Info[i] = Info_doc.createNestedObject();
    Info_c1_V = Info[i].createNestedArray("V");
    Info_c1_V.add(i);
    Info_c1_V.add(i);
    Info_c1_V.add(i);
    Info_c1_C = Info[i].createNestedArray("C");
    Info_c1_C.add(i);
    Info_c1_C.add(i);
    Info_c1_C.add(i);
    Info_c1_X = Info[i].createNestedArray("X");
    Info_c1_X.add(i);
    Info_c1_X.add(i);
    Info_c1_X.add(i);
    Info_c1_X.add(i);
    Info_c1_X.add(i);
    Info_c1_X.add(i);
    Info_c1_X.add(i);
    Info_c1_X.add(i);
   
  }
  unsigned short _mem = doc.memoryUsage();
  //serializeJsonPretty(doc, Serial);
  //Serial.println("Json-Size: ");
  //Serial.println(_mem);
  Serial.println("Publishing message to AWS...");
  //serializeJson(doc, Serial);
  char jsonBuffer[cloud_buffer];
  serializeJson(doc, jsonBuffer);
  Serial.println(jsonBuffer);
  client.publish(AWS_IOT_TOPIC, jsonBuffer );
  vTaskDelay(50 / portTICK_RATE_MS);
}
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
    if(zcCount[muxPin] >= 5){
      iSum[muxPin] = iSum[muxPin]/((float)adcCount[muxPin]);
      rawIrms = sqrt(iSum[muxPin]);
      muxIrms[muxPin] = muxIrms[muxPin]*0.5 + rawIrms*scalerMuxIrms[muxPin]*0.5;
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
void MuX_Code(void)
{
   static unsigned long prevTime=0;
   for (int  pin=0; pin<8; pin++)
   {
     selectMuxPin(MUX_SECONDARY, pin); // Select one at a time
     calcIrmsMux(pin);
   }
  prevTime = micros(); 
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

void connectToCertf()
{
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  //=======================================
  //Root CA File Reading.
  File file2 = SPIFFS.open("/AmazonRootCA1.pem", "r");
  if (!file2) {
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.println("Root CA File Content:");
  while (file2.available()) {
    Read_rootca = file2.readString();
    Serial.println(Read_rootca);
  }
  //=============================================
  // Cert file reading
  File file4 = SPIFFS.open("/84f48202f3-certificate.pem.crt", "r");
  if (!file4) {
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.println("Cert File Content:");
  while (file4.available()) {
    Read_cert = file4.readString();
    Serial.println(Read_cert);
  }
  //=================================================
  //Privatekey file reading
  File file6 = SPIFFS.open("/84f48202f3-private.pem.key", "r");
  if (!file6) {
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.println("privateKey File Content:");
  while (file6.available()) {
    Read_privatekey = file6.readString();
    Serial.println(Read_privatekey);
  }
  //=====================================================

  char* pRead_rootca;
  pRead_rootca = (char *)malloc(sizeof(char) * (Read_rootca.length() + 1));
  strcpy(pRead_rootca, Read_rootca.c_str());

  char* pRead_cert;
  pRead_cert = (char *)malloc(sizeof(char) * (Read_cert.length() + 1));
  strcpy(pRead_cert, Read_cert.c_str());

  char* pRead_privatekey;
  pRead_privatekey = (char *)malloc(sizeof(char) * (Read_privatekey.length() + 1));
  strcpy(pRead_privatekey, Read_privatekey.c_str());

  Serial.println("================================================================================================");
  Serial.println("Certificates that passing to espClient Method");
  Serial.println();
  Serial.println("Root CA:");
  Serial.write(pRead_rootca);
  Serial.println("================================================================================================");
  Serial.println();
  Serial.println("Cert:");
  Serial.write(pRead_cert);
  Serial.println("================================================================================================");
  Serial.println();
  Serial.println("privateKey:");
  Serial.write(pRead_privatekey);
  Serial.println("================================================================================================");

  net.setCACert(pRead_rootca);
  net.setCertificate(pRead_cert);
  net.setPrivateKey(pRead_privatekey);
  
  }
