#include "application.h"

#include "Kegerator.h"
#include "Onewire/OneWire.h"
#include "SparkCoreDallasTemperature/spark-dallas-temperature.h"







int hotRelay = D5; 
int coldRelay = A0;
int statet;

DallasTemperature temp1(new OneWire(D1));
DallasTemperature temp2(new OneWire(D2));


void turnOnCold(int pin){
    //Do I really need to explain this?
    digitalWrite(hotRelay,LOW);   
    digitalWrite(coldRelay,HIGH);
}
void turnOnHeat(int pin){
    //This is self documenting code. YES KYS.
    digitalWrite(coldRelay,LOW);
    digitalWrite(hotRelay,HIGH);   
}
void turnOffTemperatureControls(){
	digitalWrite(coldRelay,LOW);
	digitalWrite(hotRelay,LOW);
} 
int setTempInternet(String a){
    int t = a.toInt();
    if (t != 0){
	    settings.setTemp = t;
	    EEPROM.put(0,settings);
	    return 1;
    }
    return 0;   
}
void postDataToServer(int temp1, int temp2, int temp3, char *State){
    Serial.println("TCPClient Connect");
    String req = String("token=BF7e1pbqhST0zrJS4VIK&Temp1=")+String(temp1)+String("&Temp2=")+String(temp2)+String("&Temp3=")+String(temp3)+String("&State=")+String(State);
    Serial.print("REQUEST BODY: ");
    Serial.println(req);
    client.connect(API_DOMAIN,5000);
    Serial.println("TCP OPEN");
    client.println("POST /incoming HTTP/1.1");
    client.print("Host: ");
    client.println(API_DOMAIN);
    client.print("Content-length: ");
    client.println(req.length());
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println();
    client.println(req);
    client.flush();
    Serial.println("TCP END");
    delay(200);
    client.stop();
    

}

void setup() { 
    
    Serial.begin(9600); //Start usb serial logging

    Particle.syncTime();
    Serial.print("Initial Time sync: "); 
    Serial.println(Time.timeStr());

    pinMode(hotRelay,OUTPUT);
    pinMode(coldRelay,OUTPUT);

    RGB.control(true);

    tempCoef = 2;
 //  setTemp = 45; 
    probe1 = 100;
    probe2 = 100;
 
    temp1.begin();
    temp2.begin();
    
    EEPROM.get(0,settings);     
    if (settings.version != 1){
	    eeprom_settings defSettings = {1,45};
	    settings = defSettings;
    } 
    
    String("Null").toCharArray(stateString,5);
    //Setup Variable Block
    Particle.variable("setTemp",&settings.setTemp,INT);
    Particle.variable("probe1",&probe1,INT);
    Particle.variable("probe2",&probe2,INT);
    Particle.variable("curState",stateString,STRING);
    Particle.function("setTempFunc",setTempInternet);

    Serial.println("Setup Complete.");
    

   
     
 }

void loop() {
    

    //If mode is hold for 10ms or more enter update loop
    if(System.buttonPushed() > 10){
	RGB.color(0,200,0);
	debugloop=true;

    }
    if(debugloop){
     	Serial.println("Waiting for Update");
        return;
    }

    //only execute once every 10+ seconds
    if (millis() - lastrun < TEN_SECONDS_MILLIS){
          return;
    }
    lastrun = millis();

    if (millis() - lastSync > ONE_DAY_MILLIS){
 	//Sync Clock if One day has passed.    
        Particle.syncTime();
        lastSync= millis();
        Serial.print("Time updated new day: ");
        Serial.println(Time.timeStr());
    }
        
    //#TODO
    // checkScheduler();

    temp1.requestTemperatures();
    temp2.requestTemperatures();
    probe1 = temp1.getTempFByIndex( 0 );
    probe2 = temp2.getTempFByIndex( 0 );
    Serial.print("Temperature 1: "); Serial.println(probe1) ;
    Serial.print("Temperature 2: "); Serial.println(probe2) ;
     

    switch(state){
        case Hold:
            Serial.println("System State:Hold");
	    if (probe1 > (settings.setTemp+tempCoef)){
		if (millis() - lastColdEnable > THREE_MINUTES_MILLIS){
			state= stateType::Cool;
			lastColdEnable = millis();
			
		}
		
	    }
	    else if (probe1 < (settings.setTemp-tempCoef)){
	        state = stateType::Heat;
	    }
	    turnOffTemperatureControls();
	    RGB.color(200,200,200);
	    String("Hold").toCharArray(stateString,5);
            break;
        case Cool:
            Serial.println("System State:Cool");
            if (probe1 <= (settings.setTemp)){
	        state = stateType::Hold;
	    }
            turnOnCold(coldRelay);
	    RGB.color(0,200,200);
	    String("Cool").toCharArray(stateString,5);
	    break;
        case Heat:
            //  Serial.println("System State:Heat");
	    if (probe1 > (settings.setTemp)){
	        state = stateType::Hold;
	    }
	    turnOnHeat(hotRelay);
            //No heater yet,
	    RGB.color(200,0,0);
	    String("Heat").toCharArray(stateString,5);
            break;
    }
    //Only send data every 10 minutes or so. 
    if (millis() - lastPush >= THREE_MINUTES_MILLIS){ 
	  lastPush = millis();
    }
 //    postDataToServer(probe1,probe2,0,stateString);
  
}
