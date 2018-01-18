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


/**************************************************
		DHT22 Setup
***************************************************/
#define DHTPIN 0
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

/**************************************************
		FastLED Setup
***************************************************/
#define NUM_LEDS 1
#define DATA_PIN 2
CRGB leds[NUM_LEDS];
int number_led = 1;

/**************************************************
		Blynk Setup
***************************************************/
char auth[] = "";
SimpleTimer timer; //Create a timer

char hub_auth[] = "#####";

WidgetBridge hub_bridge(V10);

/**************************************************
	Internet Variables
***************************************************/
const char* ssid = "#####";
const char* password = "#####";

//Static IP Setup
IPAddress staticIP#####);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);


//First time blynk runs set up bridge
BLYNK_CONNECTED() {
	if (isFirstConnect == true) {
		hub_bridge.setAuthToken(hub_auth);
		isFirstConnect = false;
	}
}

//From Hub Door Switch
//BLYNK_WRITE(V6) {
//
//	int appdata = param.asInt();
//
//	if (appdata != door_status) {
//
//		//Tell servo to push button
//		myservo.write(45);
//		delay(500);
//		myservo.write(180);
//	}
//}


void setup() {

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

	//Start Blynk Server
	Blynk.begin(auth, ssid, password, IPAddress(192, 168, 1, 143));
	
	//Start timers
	timer.setInterval(60000L, onemintimer);

	//Notify that completed Setup
	leds[0] = CRGB::Green;
	FastLED.show();
}


/**************************************************
		Global Variables
***************************************************/

bool is_day;
float temp, hum, uv, soil_moist, soil_temp, battery;

void loop() {




}

void onemintimer() {
	
	if (battery > 15){

		//read light
		//read temp
		//read hum
		//read UV
		//read soil_temp
		//read soil_moist

		//send data to hub / Blynk	
	
	
	}


	


}
