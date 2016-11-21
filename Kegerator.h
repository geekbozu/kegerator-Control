#ifndef Kegerator_h
#define Kegerator_h

#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)
#define TEN_MINUTES_MILLIS ( 1000 * 60 * 10 ) 
#define THREE_MINUTES_MILLIS ( 1000 * 60 * 3 ) 
#define TEN_SECONDS_MILLIS ( 1000 * 10 )
#define API_DOMAIN "razzi.rcfreak0.com"
#include "Onewire/OneWire.h"
#include "SparkCoreDallasTemperature/spark-dallas-temperature.h"
//Start prototypes here

#define MODE_PIN D5
enum stateType {
    Heat,
    Cool,
    Hold
};
//Setttings stored in eeprom
struct eeprom_settings {
        int version;
	int setTemp;
};
eeprom_settings settings = {1,45};   
stateType state = stateType::Hold;
unsigned long lastSync = millis();
unsigned long lastPush = millis();
unsigned long lastColdEnable = 180000;
unsigned long lastrun = millis();
//void button_handler(system_event_t event, int duration, void* );
bool debugloop = false; 
int setTemp;
int probe1;
int probe2;
int tempCoef;
char stateString[10];
void turnOnCold(int pin); 
void turnOnHeat(int pin);
void turnOffTemperatureControls();
int setTempInternet(String a);
void postDataToServer(int temp1, int temp2, int temp3, char *State);

TCPClient client;

#endif  
