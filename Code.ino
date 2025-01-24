/**********************************************************************************
 *  TITLE: ESP RainMaker + IR + Switch control 1 Fan & 6 Relays using ESP32
 *  This code is provided free for project purpose and fair use only.
 *  Please do mail us to trimaptechsolutions@gmail.com if you want to use it commercially.
 *  Copyrighted © by TriMAP Techsolutions
 *  
 *  Preferences--> Aditional boards Manager URLs : 
 *  http://arduino.esp8266.com/stable/package_esp8266com_index.json,https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *  
 *  Download Board ESP32 (2.0.5) : https://github.com/espressif/arduino-esp32
 *
 *  Download the libraries 
 *  IRremote Library (3.6.1): https://github.com/Arduino-IRremote/Arduino-IRremote
 *  SimpleTimer Library (1.0.0): https://github.com/kiryanenko/SimpleTimer
 *  AceButton Library (1.9.2): https://github.com/bxparks/AceButton
 **********************************************************************************/

// define the Node Name
char nodeName[] = "ESP32_SmartHome";

// define the Device Names
char deviceName_1[] = "Switch1";
char deviceName_2[] = "Switch2";
char deviceName_3[] = "Switch3";
char deviceName_4[] = "Switch4";
char deviceName_5[] = "Switch5"
char deviceName_6[] = "Switch6"

//Update the HEX code of IR Remote buttons 0x<HEX CODE>
#define IR_Button_1   0x1FEE01F
#define IR_Button_2   0x1FE10EF
#define IR_Button_3   0x1FE906F
#define IR_Button_4   0x1FE50AF
#define IR_Button_5   0x1FED827
#define IR_Button_6   0x1FED82F
#define IR_Fan_Up     0x1FE609F
#define IR_Fan_Down   0x1FEA05F
#define IR_All_On     0x1FE807F
#define IR_All_Off    0x1FE48B7
 
#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"
#include <IRremote.h>
#include <SimpleTimer.h>
#include <Preferences.h>
#include <AceButton.h>
using namespace ace_button;

Preferences pref;
SimpleTimer Timer;

const char *service_name = "PROV_TriMAPTechsolutions";
const char *pop = "TMTS123MAP3";

// define the Chip Id
uint32_t espChipId = 5;


// define the GPIO connected with Relays and switches
static uint8_t RelayPin1 = 22;  //D22
static uint8_t RelayPin2 = 21;  //D21
static uint8_t RelayPin3 = 19;  //D19
static uint8_t RelayPin4 = 18;  //D18
static uint8_t RelayPin5 = 5;  //D5
static uint8_t RelayPin6 = 17;  //D17


static uint8_t SwitchPin1 = 13;  //D13
static uint8_t SwitchPin2 = 12;  //D12
static uint8_t SwitchPin3 = 14;  //D14
static uint8_t SwitchPin4 = 27;  //D27
static uint8_t SwitchPin5 = 26;  //D26
static uint8_t SwitchPin6 = 25;  //D25

static uint8_t FanRelay1 = 16;  //D16
static uint8_t FanRelay2 = 4;  //D4
static uint8_t FanRelay3 = 2;  //D2
static uint8_t FanRelay4 = 15;  //D15

static uint8_t FanSwitch1 = 32;  //D32
static uint8_t FanSwitch2 = 23;  //D23
static uint8_t FanSwitch3 = 34;  //D34
static uint8_t FanSwitch4 = 35;  //D35


static uint8_t gpio_reset  = 0 ;   // Press BOOT to reset WiFi Details
static uint8_t IR_RECV_PIN = 33;   // D35 (IR receiver pin)

int currSpeed = 0;

// Relay State
bool toggleState_1 = LOW; //Define integer to remember the toggle state for relay 1
bool toggleState_2 = LOW; //Define integer to remember the toggle state for relay 2
bool toggleState_3 = LOW; //Define integer to remember the toggle state for relay 3
bool toggleState_4 = LOW; //Define integer to remember the toggle state for relay 4
bool toggleState_5 = LOW; //Define integer to remember the toggle state for relay 5
bool toggleState_6 = LOW; //Define integer to remember the toggle state for relay 6

bool fanSpeed_0 = LOW;
bool fanSpeed_1 = LOW; 
bool fanSpeed_2 = LOW; 
bool fanSpeed_3 = LOW; 
bool fanSpeed_4 = LOW; 

int wifiFlag = 0;
bool first_run = true;

IRrecv irrecv(IR_RECV_PIN);
decode_results results;

ButtonConfig config1;
AceButton button1(&config1);
ButtonConfig config2;
AceButton button2(&config2);
ButtonConfig config3;
AceButton button3(&config3);
ButtonConfig config4;
AceButton button4(&config4);
ButtonConfig config5;
AceButton button5(&config5);
ButtonConfig config6;
AceButton button6(&config6);

void handleEvent1(AceButton*, uint8_t, uint8_t);
void handleEvent2(AceButton*, uint8_t, uint8_t);
void handleEvent3(AceButton*, uint8_t, uint8_t);
void handleEvent4(AceButton*, uint8_t, uint8_t);
void handleEvent5(AceButton*, uint8_t, uint8_t);
void handleEvent6(AceButton*, uint8_t, uint8_t);

//The framework provides some standard device types like switch, lightbulb, fan, temperature sensor.
static Switch my_switch1(deviceName_1, &RelayPin1);
static Switch my_switch2(deviceName_2, &RelayPin2);
static Switch my_switch3(deviceName_3, &RelayPin3);
static Switch my_switch4(deviceName_4, &RelayPin4);
static Switch my_switch5(deviceName_5, &RelayPin5);
static Switch my_switch6(deviceName_6, &RelayPin6);

void sysProvEvent(arduino_event_t *sys_event)
{
    switch (sys_event->event_id) {      
        case ARDUINO_EVENT_PROV_START:
#if CONFIG_IDF_TARGET_ESP32
        Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on BLE\n", service_name, pop);
        printQR(service_name, pop, "ble");
#else
        Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on SoftAP\n", service_name, pop);
        printQR(service_name, pop, "softap");
#endif        
        break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        Serial.printf("\nConnected to Wi-Fi!\n");
        digitalWrite(wifiLed, true);
        break;
    }
}

void write_callback(Device *device, Param *param, const param_val_t val, void *priv_data, write_ctx_t *ctx)
{
  const char *device_name = device->getDeviceName();
  const char *param_name = param->getParamName();

  if(strcmp(device_name, deviceName_1) == 0) {
    
    Serial.printf("Switch value = %s\n", val.val.b? "true" : "false");
    
    if(strcmp(param_name, "Power") == 0) {
        Serial.printf("Received value = %s for %s - %s\n", val.val.b? "true" : "false", device_name, param_name);
      toggleState_1 = val.val.b;
      (toggleState_1 == false) ? digitalWrite(RelayPin1, HIGH) : digitalWrite(RelayPin1, LOW);
      param->updateAndReport(val);
      pref.putBool("Relay1", toggleState_1);
    }
    
  } else if(strcmp(device_name, deviceName_2) == 0) {
    
    Serial.printf("Switch value = %s\n", val.val.b? "true" : "false");

    if(strcmp(param_name, "Power") == 0) {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b? "true" : "false", device_name, param_name);
      toggleState_2 = val.val.b;
      (toggleState_2 == false) ? digitalWrite(RelayPin2, HIGH) : digitalWrite(RelayPin2, LOW);
      param->updateAndReport(val);
      pref.putBool("Relay2", toggleState_2);
    }

  } else if(strcmp(device_name, deviceName_3) == 0) {
    
    Serial.printf("Switch value = %s\n", val.val.b? "true" : "false");

    if(strcmp(param_name, "Power") == 0) {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b? "true" : "false", device_name, param_name);
      toggleState_3 = val.val.b;
      (toggleState_3 == false) ? digitalWrite(RelayPin3, HIGH) : digitalWrite(RelayPin3, LOW);
      param->updateAndReport(val);
      pref.putBool("Relay3", toggleState_3);
    }

  } else if(strcmp(device_name, deviceName_4) == 0) {
    
    Serial.printf("Switch value = %s\n", val.val.b? "true" : "false");

    if(strcmp(param_name, "Power") == 0) {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b? "true" : "false", device_name, param_name);
      toggleState_4 = val.val.b;
      (toggleState_4 == false) ? digitalWrite(RelayPin4, HIGH) : digitalWrite(RelayPin4, LOW);
      param->updateAndReport(val);
      pref.putBool("Relay4", toggleState_4);
    }     
     
  } else if(strcmp(device_name, deviceName_5) == 0) {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("Received Fan power = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      toggleState_5 = val.val.b;
      (toggleState_5 == false) ? fanSpeedControl(0) : fanSpeedControl(currSpeed);
      param->updateAndReport(val);
      pref.putBool("Fan_Power", toggleState_5);
    }  
    if (strcmp(param_name, "My_Speed") == 0)
    {
      Serial.printf("\nReceived value = %d for %s - %s\n", val.val.i, device_name, param_name);
      currSpeed = val.val.i;
      if(toggleState_5 == 1){
        fanSpeedControl(currSpeed);
      }
      param->updateAndReport(val);
      pref.putInt("Fan_Speed", currSpeed);
    }
 } else if(strcmp(device_name, deviceName_6) == 0) {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("Received Fan power = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      toggleState_6 = val.val.b;
      (toggleState_6 == false) ? fanSpeedControl(0) : fanSpeedControl(currSpeed);
      param->updateAndReport(val);
      pref.putBool("Fan_Power", toggleState_6);
    }  
    if (strcmp(param_name, "My_Speed") == 0)
    {
      Serial.printf("\nReceived value = %d for %s - %s\n", val.val.i, device_name, param_name);
      currSpeed = val.val.i;
      if(toggleState_6 == 1){
        fanSpeedControl(currSpeed);
      }
      param->updateAndReport(val);
      pref.putInt("Fan_Speed", currSpeed);
    }
 }
}

void ir_remote(){
  if (irrecv.decode(&results)) {
      switch(results.value){
          case IR_Button_1:  
            digitalWrite(RelayPin1, toggleState_1);
            toggleState_1 = !toggleState_1;
            pref.putBool("Relay1", toggleState_1);
            my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_1);
            delay(100);            
            break;
          case IR_Button_2:  
            digitalWrite(RelayPin2, toggleState_2);
            toggleState_2 = !toggleState_2;
            pref.putBool("Relay2", toggleState_2);
            my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_2);
            delay(100);            
            break;
          case IR_Button_3:  
            digitalWrite(RelayPin3, toggleState_3);
            toggleState_3 = !toggleState_3;
            pref.putBool("Relay3", toggleState_3);
            my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_3);
            delay(100);            
            break;
          case IR_Button_4:  
            digitalWrite(RelayPin4, toggleState_4);
            toggleState_4 = !toggleState_4;
            pref.putBool("Relay4", toggleState_4);
            my_switch4.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_4);
            delay(100);            
            break;
          case IR_Button_5:  
            digitalWrite(RelayPin5, toggleState_5);
            toggleState_5 = !toggleState_5;
            pref.putBool("Relay5", toggleState_5);
            my_switch5.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_5);
            delay(100);            
            break;
          case IR_Button_6:  
            digitalWrite(RelayPin6, toggleState_6);
            toggleState_6 = !toggleState_6;
            pref.putBool("Relay6", toggleState_6);
            my_switch6.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_6);
            delay(100);            
            break;
          case IR_Fan_Up: 
            if(currSpeed < 5){
              currSpeed = currSpeed + 1;
              fanSpeedControl(currSpeed);
              pref.putInt("Fan_Speed", currSpeed);
              my_fan.updateAndReportParam("My_Speed", currSpeed);
              delay(100); 
            }           
            break;
          case IR_Fan_Down: 
            if(currSpeed > 0){
              currSpeed = currSpeed - 1;
              fanSpeedControl(currSpeed);
              pref.putInt("Fan_Speed", currSpeed);
              my_fan.updateAndReportParam("My_Speed", currSpeed);
              delay(100); 
            }       
            break;
          case IR_All_Off:
            all_SwitchOff();  
            break;
          case IR_All_On:
            all_SwitchOn();  
            break;
          default : break;         
        }   
        //Serial.println(results.value, HEX);    
        irrecv.resume();   
  } 
}

void all_SwitchOff(){
  toggleState_1 = 0; digitalWrite(RelayPin1, HIGH); pref.putBool("Relay1", toggleState_1); my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_1); delay(100);
  toggleState_2 = 0; digitalWrite(RelayPin2, HIGH); pref.putBool("Relay2", toggleState_2); my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_2); delay(100);
  toggleState_3 = 0; digitalWrite(RelayPin3, HIGH); pref.putBool("Relay3", toggleState_3); my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_3); delay(100);
  toggleState_4 = 0; digitalWrite(RelayPin4, HIGH); pref.putBool("Relay4", toggleState_4); my_switch4.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_4); delay(100);
  toggleState_5 = 0; digitalWrite(RelayPin5, HIGH); pref.putBool("Relay5", toggleState_5); my_switch5.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_5); delay(100);
  toggleState_6 = 0; digitalWrite(RelayPin6, HIGH); pref.putBool("Relay6", toggleState_6); my_switch6.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_6); delay(100);
  
}

void all_SwitchOn(){
  toggleState_1 = 1; digitalWrite(RelayPin1, LOW); pref.putBool("Relay1", toggleState_1); my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_1); delay(100);
  toggleState_2 = 1; digitalWrite(RelayPin2, LOW); pref.putBool("Relay2", toggleState_2); my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_2); delay(100);
  toggleState_3 = 1; digitalWrite(RelayPin3, LOW); pref.putBool("Relay3", toggleState_3); my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_3); delay(100);
  toggleState_4 = 1; digitalWrite(RelayPin4, LOW); pref.putBool("Relay4", toggleState_4); my_switch4.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_4); delay(100);
  toggleState_5 = 1; digitalWrite(RelayPin5, LOW); pref.putBool("Relay5", toggleState_5); my_switch5.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_5); delay(100);
  toggleState_6 = 1; digitalWrite(RelayPin6, LOW); pref.putBool("Relay6", toggleState_6); my_switch6.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_6); delay(100);

void getRelayState()
{
  toggleState_1 = pref.getBool("Relay1", 0);
  digitalWrite(RelayPin1, !toggleState_1); 
  my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_1);
  delay(200);
  toggleState_2 = pref.getBool("Relay2", 0);
  digitalWrite(RelayPin2, !toggleState_2); 
  my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_2);
  delay(200);
  toggleState_3 = pref.getBool("Relay3", 0);
  digitalWrite(RelayPin3, !toggleState_3); 
  my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_3);
  delay(200);
  toggleState_4 = pref.getBool("Relay4", 0);
  digitalWrite(RelayPin4, !toggleState_4); 
  my_switch4.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_4);
  delay(200);
  toggleState_5 = pref.getBool("Relay5", 0);
  digitalWrite(RelayPin5, !toggleState_5); 
  my_switch4.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_5);
  delay(200);
  toggleState_6 = pref.getBool("Relay6", 0);
  digitalWrite(RelayPin6, !toggleState_6); 
  my_switch6.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_6);
  delay(200);
}  

void setup()
{
  Serial.begin(115200);
  //Open namespace in read-write mode
  pref.begin("Relay_State", false);
  
  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(RelayPin3, OUTPUT);
  pinMode(RelayPin4, OUTPUT);
  pinMode(FanRelay1, OUTPUT);
  pinMode(FanRelay2, OUTPUT);
  pinMode(FanRelay3, OUTPUT);
  pinMode(FanRelay4, OUTPUT);

  pinMode(wifiLed, OUTPUT);

  pinMode(SwitchPin1, INPUT_PULLUP);
  pinMode(SwitchPin2, INPUT_PULLUP);
  pinMode(SwitchPin3, INPUT_PULLUP);
  pinMode(SwitchPin4, INPUT_PULLUP);
  pinMode(SwitchPin5, INPUT_PULLUP);
  pinMode(SwitchPin5, INPUT_PULLUP);
  pinMode(FanSwitch1, INPUT_PULLUP);
  pinMode(FanSwitch2, INPUT_PULLUP);
  pinMode(FanSwitch3, INPUT_PULLUP);
  pinMode(FanSwitch4, INPUT_PULLUP);
  pinMode(gpio_reset, INPUT);

  //During Starting all Relays should TURN OFF
  digitalWrite(RelayPin1, !toggleState_1);
  digitalWrite(RelayPin2, !toggleState_2);
  digitalWrite(RelayPin3, !toggleState_3);
  digitalWrite(RelayPin4, !toggleState_4);
  digitalWrite(FanRelay1, HIGH);
  digitalWrite(FanRelay2, HIGH);
  digitalWrite(FanRelay3, HIGH);

  digitalWrite(wifiLed, LOW);

  config1.setEventHandler(button1Handler);
  config2.setEventHandler(button2Handler);
  config3.setEventHandler(button3Handler);
  config4.setEventHandler(button4Handler);
  config5.setEventHandler(button5Handler);

  button1.init(SwitchPin1);
  button2.init(SwitchPin2);
  button3.init(SwitchPin3);
  button4.init(SwitchPin4);
  button5.init(SwitchPin5);
  button5.init(SwitchPin5);

  irrecv.enableIRIn(); // Enabling IR sensor

  Node my_node;    
  my_node = RMaker.initNode(nodeName);

  
  //Standard switch device
  my_switch1.addCb(write_callback);
  my_switch2.addCb(write_callback);
  my_switch3.addCb(write_callback);
  my_switch4.addCb(write_callback);
  my_switch5.addCb(write_callback);
  my_switch6.addCb(write_callback);

  //Define Param Fan Speed
  Param speed("My_Speed",ESP_RMAKER_PARAM_RANGE , value(0), PROP_FLAG_READ | PROP_FLAG_WRITE);
  speed.addBounds(value(0), value(4), value(1));
  speed.addUIType(ESP_RMAKER_UI_SLIDER);
  my_fan.addParam(speed);

  //Add switch device to the node   
  my_node.addDevice(my_switch1);
  my_node.addDevice(my_switch2);
  my_node.addDevice(my_switch3);
  my_node.addDevice(my_switch4);
  my_node.addDevice(my_switch5);
  my_node.addDevice(my_switch4);

  Timer.setInterval(30000); //send Data after every 30 seconds
  
  delay(1000);

  //This is optional 
  RMaker.enableOTA(OTA_USING_PARAMS);
  //If you want to enable scheduling, set time zone for your region using setTimeZone(). 
  //The list of available values are provided here https://rainmaker.espressif.com/docs/time-service.html
  // RMaker.setTimeZone("Asia/Shanghai");
  // Alternatively, enable the Timezone service and let the phone apps set the appropriate timezone
  RMaker.enableTZService();
  RMaker.enableSchedule();

  Serial.printf("\nStarting ESP-RainMaker\n");
  RMaker.start();

    WiFi.onEvent(sysProvEvent);
#if CONFIG_IDF_TARGET_ESP32
    WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name);
#else
    WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, pop, service_name);
#endif

  delay(200);
  
  getRelayState(); //fetch data from NVS Flash Memory
}

void loop()
{  
  // Read GPIO0 (external button to reset device
  if(digitalRead(gpio_reset) == LOW) { //Push button pressed
    Serial.printf("Reset Button Pressed!\n");
    // Key debounce handling
    delay(100);
    int startTime = millis();
    while(digitalRead(gpio_reset) == LOW) delay(50);
    int endTime = millis();

    if ((endTime - startTime) > 10000) {
      // If key pressed for more than 10secs, reset all
      Serial.printf("Reset to factory.\n");
      RMakerFactoryReset(2);
    } else if ((endTime - startTime) > 3000) {
      Serial.printf("Reset Wi-Fi.\n");
      // If key pressed for more than 3secs, but less than 10, reset Wi-Fi
      RMakerWiFiReset(2);
    }
  }
  delay(100);

  if (WiFi.status() != WL_CONNECTED)
  {
    //Serial.println("WiFi Not Connected");
    digitalWrite(wifiLed, false);
  }
  else
  {
    //Serial.println("WiFi Connected");
    digitalWrite(wifiLed, true);
    if (Timer.isReady()) { 
      //Serial.println("Sending Sensor Data");
      Timer.reset();      // Reset a second timer
    }
  }

  //Control Switches Manualy
  button1.check();
  button2.check();
  button3.check();
  button4.check();
  button5.check(); 
  button6.check(); 
  
  
  fanRegularor(); //Control Fan Speed Manualy

  ir_remote(); //IR remote Control
}

void fanRegularor(){
  if (digitalRead(FanSwitch1) == HIGH && digitalRead(FanSwitch2) == HIGH && digitalRead(FanSwitch3) == HIGH && digitalRead(FanSwitch4) == HIGH && digitalRead(FanSwitch5) == HIGH && fanSpeed_0 == LOW)
  {
    if(first_run == false){
      currSpeed = 0;
      pref.putInt("Fan_Speed", currSpeed);
      my_fan.updateAndReportParam("My_Speed", currSpeed);
    }    
    fanSpeed_1 = LOW;
    fanSpeed_2 = LOW;
    fanSpeed_3 = LOW;
    fanSpeed_4 = LOW;
    fanSpeed_5 = LOW;
    fanSpeed_0 = HIGH;
  }
  if (digitalRead(FanSwitch1) == LOW && fanSpeed_1 == LOW)
  {
    if(first_run == false){
      currSpeed = 1;
      if(toggleState_5 == 1){
        fanSpeedControl(currSpeed); 
      }
      pref.putInt("Fan_Speed", currSpeed);
      my_fan.updateAndReportParam("My_Speed", currSpeed);
    }
    fanSpeed_1 = HIGH;
    fanSpeed_2 = LOW;
    fanSpeed_3 = LOW;
    fanSpeed_4 = LOW;
    fanSpeed_5 = LOW;
    fanSpeed_0 = LOW;
  }
  if (digitalRead(FanSwitch2) == LOW && digitalRead(FanSwitch3) == HIGH && fanSpeed_2 == LOW)
  {
    if(first_run == false){
      currSpeed = 2;
      if(toggleState_5 == 1){
        fanSpeedControl(currSpeed); 
      }
      pref.putInt("Fan_Speed", currSpeed);
      my_fan.updateAndReportParam("My_Speed", currSpeed);
    }
    fanSpeed_1 = LOW;
    fanSpeed_2 = HIGH;
    fanSpeed_3 = LOW;
    fanSpeed_4 = LOW;
    fanSpeed_5 = LOW;
    fanSpeed_0 = LOW;
  }
  if (digitalRead(FanSwitch2) == LOW && digitalRead(FanSwitch3) == LOW && fanSpeed_3 == LOW)
  {
    if(first_run == false){
      currSpeed = 3;
      if(toggleState_5 == 1){
        fanSpeedControl(currSpeed); 
      }
      pref.putInt("Fan_Speed", currSpeed);
      my_fan.updateAndReportParam("My_Speed", currSpeed);
    }
    fanSpeed_1 = LOW;
    fanSpeed_2 = LOW;
    fanSpeed_3 = HIGH;
    fanSpeed_4 = LOW;
    fanSpeed_5 = LOW;
    fanSpeed_0 = LOW;
  }
  if (digitalRead(FanSwitch4) == LOW  && fanSpeed_4 == LOW)
  {
    if(first_run == false){
      currSpeed = 4;
      if(toggleState_5 == 1){
        fanSpeedControl(currSpeed); 
      }
      pref.putInt("Fan_Speed", currSpeed);
      my_fan.updateAndReportParam("My_Speed", currSpeed);
    }
    fanSpeed_1 = LOW;
    fanSpeed_2 = LOW;
    fanSpeed_3 = LOW;
    fanSpeed_4 = HIGH;
    fanSpeed_5 = LOW;
    fanSpeed_0 = LOW;
  }
  first_run = false;
}

void fanSpeedControl(int fanSpeed){
   switch(fanSpeed){
    case 0:
      digitalWrite(FanRelay1, HIGH);
      digitalWrite(FanRelay2, HIGH);
      digitalWrite(FanRelay3, HIGH); 
      digitalWrite(FanRelay4, HIGH);       
    break;
    case 1:
      digitalWrite(FanRelay1, HIGH);
      digitalWrite(FanRelay2, HIGH);
      digitalWrite(FanRelay3, HIGH);
      digitalWrite(FanRelay4, HIGH);
      delay(500);
      digitalWrite(FanRelay1, LOW);
    break;
    case 2:
      digitalWrite(FanRelay1, HIGH);
      digitalWrite(FanRelay2, HIGH);
      digitalWrite(FanRelay3, HIGH);
      digitalWrite(FanRelay4, HIGH);
      delay(500);
      digitalWrite(FanRelay2, LOW);
    break;
    case 3:
      digitalWrite(FanRelay1, HIGH);
      digitalWrite(FanRelay2, HIGH);
      digitalWrite(FanRelay3, HIGH);
      digitalWrite(FanRelay4, HIGH);
      delay(500);
      digitalWrite(FanRelay2, LOW);
      digitalWrite(FanRelay3, LOW);
    break;
    case 4:
      digitalWrite(FanRelay1, HIGH);
      digitalWrite(FanRelay2, HIGH);
      digitalWrite(FanRelay3, HIGH);
      digitalWrite(FanRelay4, HIGH);
      delay(500);
      digitalWrite(FanRelay1, LOW);
      digitalWrite(FanRelay2, LOW);
      digitalWrite(FanRelay3, LOW);
    break;   
    case 5:
      digitalWrite(FanRelay1, HIGH);
      digitalWrite(FanRelay2, HIGH);
      digitalWrite(FanRelay3, HIGH);
      digitalWrite(FanRelay4, HIGH);
      delay(500);
      digitalWrite(FanRelay4, LOW);
    break;        
    default : break;         
   } 
}

void button1Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("EVENT1");
  switch (eventType) {
    case AceButton::kEventPressed:
      Serial.println("kEventPressed");
      toggleState_1 = 1;
      digitalWrite(RelayPin1, LOW);
      pref.putBool("Relay1", toggleState_1);
      my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_1);
      break;
    case AceButton::kEventReleased:
      Serial.println("kEventReleased");
      toggleState_1 = 0;
      digitalWrite(RelayPin1, HIGH);
      pref.putBool("Relay1", toggleState_1);
      my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_1);  
      break;
  }
}

void button2Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("EVENT2");
  switch (eventType) {
    case AceButton::kEventPressed:
      Serial.println("kEventPressed");
      toggleState_2 = 1;
      digitalWrite(RelayPin2, LOW);
      pref.putBool("Relay2", toggleState_2);
      my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_2);  
      break;
    case AceButton::kEventReleased:
      Serial.println("kEventReleased");
      toggleState_2 = 0;
      digitalWrite(RelayPin2, HIGH);
      pref.putBool("Relay2", toggleState_2);
      my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_2);   
      break;
  }
}

void button3Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("EVENT3");
  switch (eventType) {
    case AceButton::kEventPressed:
      Serial.println("kEventPressed");
      toggleState_3 = 1;
      digitalWrite(RelayPin3, LOW);
      pref.putBool("Relay3", toggleState_3);
      my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_3);  
      break;
    case AceButton::kEventReleased:
      Serial.println("kEventReleased");
      toggleState_3 = 0;
      digitalWrite(RelayPin3, HIGH);
      pref.putBool("Relay3", toggleState_3);
      my_switch3.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_3);  
      break;
  }
}

void button4Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("EVENT4");
  switch (eventType) {
    case AceButton::kEventPressed:
      Serial.println("kEventPressed");
      toggleState_4 = 1;
      digitalWrite(RelayPin4, LOW);
      pref.putBool("Relay4", toggleState_4);
      my_switch4.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_4);
      break;
    case AceButton::kEventReleased:
      Serial.println("kEventReleased");
      toggleState_4 = 0;
      digitalWrite(RelayPin4, HIGH);
      pref.putBool("Relay4", toggleState_4);
      my_switch4.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_4);
      break;
  }
}

void button5Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("EVENT5");
  switch (eventType) {
    case AceButton::kEventPressed:
      Serial.println("kEventPressed");
      toggleState_5 = 1;
      digitalWrite(RelayPin5, LOW);
      pref.putBool("Relay5", toggleState_5);
      my_switch5.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_5);
      break;
    case AceButton::kEventReleased:
      Serial.println("kEventReleased");
      toggleState_5 = 0;
      digitalWrite(RelayPin5, HIGH);
      pref.putBool("Relay5", toggleState_5);
      my_switch5.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_5);
      break;
  }
}

void button6Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("EVENT6");
  switch (eventType) {
    case AceButton::kEventPressed:
      Serial.println("kEventPressed");
      toggleState_6 = 1;
      digitalWrite(RelayPin6, LOW);
      pref.putBool("Relay6", toggleState_6);
      my_switch6.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_6);
      break;
    case AceButton::kEventReleased:
      Serial.println("kEventReleased");
      toggleState_6 = 0;
      digitalWrite(RelayPin6, HIGH);
      pref.putBool("Relay6", toggleState_6);
      my_switch6.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_6);
      break;
  }
}