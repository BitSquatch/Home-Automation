

#include <ESP8266WiFi.h>
#include <SPI.h>
#include <BlynkSimpleEsp8266.h>
#include <FastLED.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SimpleTimer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Servo.h>
#include <math.h>       /* floor */

Servo myservo;

/**************************************************
		DHT11 Setup
***************************************************/
#define DHTPIN 0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


/**************************************************
		FastLED Setup
***************************************************/
#define NUM_LEDS 2
#define DATA_PIN 2
CRGB leds[NUM_LEDS];
int number_led = 2;


/**************************************************
			Relay
***************************************************/
const int RELAY_PIN = 15;


/**************************************************
		Reed Switch
***************************************************/
const int REED_PIN = 16;

/**************************************************
		Blynk Setup
***************************************************/
char auth[] = "###";
SimpleTimer timer; //Create a timer

char hub_auth[] = "###";

WidgetBridge hub_bridge(V10);


/**************************************************
		Internet Variables
***************************************************/
const char* ssid = "###";
const char* password = "###";

//Static IP Setup
IPAddress staticIP(192, 168, 1, 11);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);


/**************************************************
				Global Variables
***************************************************/
float h = 20;
float t = 50;
int door_status = 0;
int door_counter = 0;
int door_counter_long = 0;
int door_timer_override = 0;
bool isFirstConnect = true;

//First time blynk runs set up bridge
BLYNK_CONNECTED() {
	if (isFirstConnect == true) {
		hub_bridge.setAuthToken(hub_auth);
		isFirstConnect = false;
	}
}


//From Hub Door Switch
BLYNK_WRITE(V6) {
  
	int appdata = param.asInt();

	if (appdata != door_status) {
 
		//Tell servo to push button
		myservo.write(45);
		delay(500);
		myservo.write(180);
	}
}

//From App Garage Switch
BLYNK_WRITE(V0) {

	int appdata = param.asInt();

	if (appdata != door_status) {

		//Tell servo to push button
		myservo.write(45);
		delay(500);
		myservo.write(180);
	}
}

//Door auto shut override
BLYNK_WRITE(V3) {

	door_timer_override = param.asInt();

}



void setup() {

	//Define pinmodes
	pinMode(RELAY_PIN, OUTPUT);
	pinMode(REED_PIN, INPUT_PULLUP);

	//Setup LEDS
	FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
	leds[0] = CRGB::Purple;
	FastLED.show();

	Serial.begin(115200);

	//Start Wifi
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	WiFi.config(staticIP, gateway, subnet);
	leds[0] = CRGB::Yellow;
	FastLED.show();

	// Setup for OTA updates
	ArduinoOTA.setHostname("Garage Door ESP");
	ArduinoOTA.onStart([]() {
		Serial.println("Start");
	});
	ArduinoOTA.onEnd([]() {
		Serial.println("\nEnd");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
	});

	ArduinoOTA.begin();

	//Start temp/hum sensor
	dht.begin();

	//Start Servo
	myservo.attach(14); //D8 on node mcu
	myservo.write(180); // Send servo to starting location

	//Start Blynk Server
	Blynk.begin(auth, ssid, password, IPAddress(192, 168, 1, 143));

	//Start timers
	timer.setInterval(30000L, thirtysectimer);
	timer.setInterval(3000L, threesectimer);

	//Notify that completed Setup
	leds[0] = CRGB::Green;
	FastLED.show();
}


void loop() {
	//Determine LED color. Green good. Yellow door open
	if (door_status == 0) leds[0] = CRGB::Green;
	else leds[0] = CRGB::Yellow;
	FastLED.show();

	//Read reed sensor. Update app
	door_status = digitalRead(REED_PIN);
	if (door_status == 0)door_status = 1;
	else door_status = 0;

	Blynk.virtualWrite(V0, door_status);

	//Send door status to hub
	Blynk.run();
	timer.run();
	ArduinoOTA.handle();

}


void threesectimer(){

  //Send temp, hum, and door status to hub
  hub_bridge.virtualWrite(V10, t, h, door_status);
  
}


//30 Second timer loop
void thirtysectimer() {
	leds[0] = CRGB::Blue;
	FastLED.show();
	
	//Read temp/hum from dht. Make sure not nan
	float h_temp = dht.readHumidity();
	if (isnan(h_temp) || h_temp > h * 5) {}
	else h = h_temp;
	
	float t_temp = dht.readTemperature(true);
	if (isnan(t_temp) || t_temp > t * 2) {}
	else t = t_temp;

	int t_int = floor(t); //Hopfully should round and truncate decimals
	int h_int = floor(h);

	//Send t/h to app
	Blynk.virtualWrite(V2, h_int); //Was just h
	Blynk.virtualWrite(V1, t_int);

	//Starts counter to see how long door is open
	if (door_status == 1) {

		door_counter++;
	}
	else {
		door_counter = 0;
		door_counter_long = 0;
	}
	
	//If been open too long notify phone
	if (door_counter >= 3) {

		if (door_timer_override == 0){Blynk.notify("Garage door has been open too long");}
		
		door_counter = 0;
		door_counter_long++;
		
		//If open way too long auto shut garage
		if(door_counter_long == 2 && door_timer_override == 0){
				
			door_counter_long = 0;

			//Tell servo to push button
			myservo.write(45);
			delay(500);
			myservo.write(180);

			Blynk.notify("Closing Garage");

		}

		else if (door_counter_long >= 2) {

			door_counter_long = 0;

		}
	}
}
