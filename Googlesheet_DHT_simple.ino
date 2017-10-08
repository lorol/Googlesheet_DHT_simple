/*
 *  https://script.google.com/macros/s/<GScriptId>/exec?par1=0.4&par2=2.1&par3=8
 *  To read column header names (1=1st row) or last(0/anything) or 3=3rd or -4=4rd before the last 
 *  https://script.google.com/macros/s/<GScriptId>/exec?readrow=last
 *  To delete (except 1st row) add &delrow=yes to above 
 */
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "HTTPSRedirect.h"
#include "DHT.h"

#define DHTPIN  2  // D4 PULL UP <5K need
#define DHTTYPE DHT11   // 11, 21, 22=AM2320
#define WRITE_INTERVAL_MINITES 5

#define MAX_CONNECT 200 //reuse 2 times same clent
#define MAX_ERROR 100 //restart everything

const char *ssid =  "mySsId";
const char *pass =  "mYpaSSword";

const char* host = "script.google.com";
const int httpsPort = 443;

int seconds = (WRITE_INTERVAL_MINITES * 60) - 20; // schedule first sending to TS in 20 sec after start

char *GScriptId = "AKfy_abcdabcdabcdsjdhakshdjkhkhkjhsdhywewekhkHK7o3pQE8eU";   // Put your  GScriptId   here
char *fingerprint = "78:00:01:E8:12:0E:50:79:3C:03:10:80:B8:94:B1:36:23:B7:2E:9B"; // Optional: use generate_fingerprint.bat 

// Access to Google Spreadsheet
String url = String("/macros/s/") + GScriptId + "/exec?";

DHT dht(DHTPIN, DHTTYPE);
HTTPSRedirect* client = nullptr;

float humidity = 0.0;
float temperature = 0.0;

float hum;
float tem;

void setup() {
  Serial.begin(115200);
  Serial.println();
  delay(200);
  Serial.println("Starting ...");

  delay(200);
  dht.begin();

  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
 
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  if (WiFi.status() != WL_CONNECTED) WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);	
    Serial.println(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
}

void loop() {
  static int error_count = 0;
  static int connect_count = 0;
  static bool flag = false;

  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    flag = true;
    client->setPrintResponseBody(true); // or false and use String getResponseBody();
    client->setContentTypeHeader("application/json");  
    if (!client->connected()) client->connect(host, httpsPort);
    ++connect_count;
    Serial.println(fingerprint);
    if (client->verify(fingerprint, host)) {
      Serial.println("Certificate match.");
    } else {
      Serial.println("Certificate mis-match");
    }
  }
  
  Serial.print("------S: ");
  Serial.println(seconds);
  Serial.print("E: ");
  Serial.println(error_count);
  Serial.print("C: ");
  Serial.println(connect_count);

  if ((seconds % 3) == 0){ // every 3 sec
     hum = dht.readHumidity(); // (force = false)
     tem = dht.readTemperature();  //(s = false) - for Fahrenheit use true

     if ((tem>-50) && (tem<90)) temperature = tem;
     else Serial.println("T-err");

     if ((hum>=0) && (hum<=100)) humidity = hum;
     else Serial.println("H-err");
     
     Serial.print("T: ");
     Serial.println(temperature);
     Serial.print("H: ");
     Serial.println(humidity);
     Serial.print("I: ");
     Serial.println(WiFi.localIP());
  }
   
   if ((seconds >= (WRITE_INTERVAL_MINITES * 60)) && (temperature>-50) && (temperature<90) && (humidity>=0) && (humidity<=100)){
		seconds = 0;

    client->connect(host, httpsPort);

    Serial.println("-------------------------");
		Serial.println("Write to GS");
		if (client->GET(url + String("par1=") + String(temperature) + String("&par2=") + String(humidity) + String("&par3=") + String(error_count), host)){
			++connect_count;
		} else {
			++error_count;
			Serial.println("Push fail");
      seconds = (WRITE_INTERVAL_MINITES * 60) - 10; // stage again in 10 sec
		}
 
   }

   // 30 sec after writing 
   if (seconds == 30){

    if (client != nullptr){
      if (!client->connected()){
        client->connect(host, httpsPort);
      }
    }
    else{
      Serial.println("Error2 creating client object!");
      error_count = -10;
    }

    Serial.println("=========================");
    if (client->GET(url + String("readrow=last"), host)){
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(client->getResponseBody());
      //json.printTo(Serial);
      float gt = json["values"][1];
      float gh = json["values"][2];
      Serial.println(gt);
      Serial.println(gh);
      ++connect_count;
    } else {
      ++error_count;
      Serial.println("GET Last row val failed!");
      seconds = 10; // stage again in 10 sec
    }

   }

   if (error_count > MAX_ERROR){
     Serial.println("Errors > MAX_ERROR..."); 
     //delete client;
     //client = nullptr;
     //connect_count = -1;
     //error_count = 0;
     //flag = false;
     ESP.restart(); 
   }

   if (connect_count > MAX_CONNECT){
      Serial.println("Connects > MAX_CONNECT... rebuild");
      connect_count = -1;
      flag = false;
      delete client;
     
      //return;  //where?
   }
   
   seconds++;
   delay(1000); //1 sec loop
}
