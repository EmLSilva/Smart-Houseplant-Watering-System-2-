/* 
 * Project Smart Watering System
 * Author: Emily Silva
 * Date: 08-NOV-2023
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include <Adafruit_MQTT.h>
#include "credentials.h"
#include <Adafruit_MQTT/Adafruit_MQTT_SPARK.h>
#include <Adafruit_MQTT/Adafruit_MQTT.h>

const int soilMoisturePin = A2;
const int pumpPin = D5;
//const int publishTime = 30000; 


int buttonValue;  
int moistureValue;
int pumpLastTime;
int currentTime;

/************ Global State (you don't need to change this!) ***   ***************/ 
TCPClient TheClient; 

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details. 
Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY); 

/****************************** Feeds ***************************************/ 
// Setup Feeds to publish or subscribe 
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname> 
Adafruit_MQTT_Subscribe buttonFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/buttononoff"); 
Adafruit_MQTT_Publish soilFeed=Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Moisture");

/************Declare Variables*************/
unsigned int last, lastTime;
float subValue,pubValue;

/************Declare Functions*************/
void MQTT_connect();
bool MQTT_ping();
float buttononoff;


SYSTEM_MODE(AUTOMATIC);


void setup() {
  //WiFi.on();
   //WiFi.connect();
   Serial.begin(9600);
   pinMode(soilMoisturePin, INPUT);
   pinMode(pumpPin,OUTPUT);
   Time.zone(-7);
   Particle.syncTime();
   //Wire.begin();
   mqtt.subscribe(&buttonFeed);
   
}

void loop() {
  MQTT_connect();
  MQTT_ping();
  currentTime=millis();
  moistureValue=analogRead(soilMoisturePin); 
  Serial.printf("moistureValue %i \n",moistureValue);
  if((currentTime-lastTime) > 30000){
    if(moistureValue>=3100){
      Serial.printf("turningPumpOn");
      digitalWrite(pumpPin, HIGH);
      delay(5000);
      digitalWrite(pumpPin, LOW);
    }
    if(mqtt.Update()){
      soilFeed.publish(moistureValue);
     Serial.printf("Publishing %i \n",moistureValue); 
        
    } 
    lastTime=millis();  
  }
    
      
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription=mqtt.readSubscription(4000))) {
    if (subscription==&buttonFeed) {
      buttonValue=atoi((char *)buttonFeed.lastread);
      Serial.printf("buttonValue%i\n",buttonValue); 
    }
  }
  if (buttonValue==HIGH) {
        digitalWrite(pumpPin, HIGH);
        Serial.printf("Manual Water Button ON \n"); 
      }
      else{
        digitalWrite(pumpPin, LOW);
        Serial.printf("Manual Water Button OFF \n"); 
      }
}

void MQTT_connect() {
  int8_t ret;
 
  // Return if already connected.
  if (mqtt.connected()) { //checks to see if connected to function
    return;
  }
 
  Serial.print("Connecting to MQTT... "); 
 
  while ((ret = mqtt.connect()) != 0) { 
      Serial.printf("Error Code %s\n",mqtt.connectErrorString(ret)); 
      Serial.printf("Retrying MQTT connection in 5 seconds...\n"); 
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.printf("MQTT Connected!\n"); 
}
bool MQTT_ping(){
  static unsigned int last;
  bool pingStatus;

  if ((millis()-last)>120000) {
      Serial.printf("Pinging MQTT \n"); 
      pingStatus = mqtt.ping();
        if(!pingStatus) {
          Serial.printf("Disconnecting \n");
          mqtt.disconnect();
        }
      last = millis();
  }
  return pingStatus;
}