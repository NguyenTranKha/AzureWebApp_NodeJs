/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-date-time-ntp-client-server-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <DHT.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

ESP8266WiFiMulti WiFiMulti;

// Replace with your network credentials
//const char *ssid     = "Kha Vy";
//const char *password = "21071997";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
#define DHTPIN 2     // Digital pin connected to the DHT sensor
#define DHTTYPE    DHT11     // DHT 11
DHT dht(DHTPIN, DHTTYPE);
String Temp = "";
String Humi = "";
String Time = "";

String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);


  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Kha Vy", "21071997");
  
  // Connect to Wi-Fi
//  Serial.print("Connecting to ");
//  Serial.println(ssid);
//  WiFi.begin(ssid, password);
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(25200);
}

void loop() {
  GetEvironment();
  sendMongodb(Time, Temp, Humi);
  Serial.print("Day: ");
  Serial.println(Time);
  Serial.println("");
  Serial.print("Temp: ");
  Serial.println(Temp);
  Serial.println("");
  Serial.print("Humi: ");
  Serial.println(Humi);
  Serial.println("");

  delay(2000);
}

void sendMongodb(String Time, String Temp, String Humi){
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    //client->setFingerprint(fingerprint);
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
    client->setInsecure();

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://ap-southeast-1.aws.data.mongodb-api.com/app/data-ikhcm/endpoint/data/v1/action/updateOne")) {  // HTTPS
      
      https.addHeader("Content-Type", "application/json"); 
      https.addHeader("Access-Control-Request-Headers", "*");
      https.addHeader("Accept", "application/json"); 
      https.addHeader("api-key", "tJrnnyPizDZ4Xiber4MLXIx7EWbAqofP64ekxAZh9dUGgTeYXzmnVC7C2hAmvfWs");
      
      String payload = "{\"dataSource\":\"Cluster0\",\"database\":\"Earth\",\"collection\":\"Weather\",\"filter\":{\"_id\":{\"$oid\":\"650bc4a2e0ddb653931e3400\"}},\"update\":{\"$set\":{\"Time\":\"" + Time +"\",\"Evironment\":{\"Temp\":\""+ Temp +"\",\"Humi\":\""+Humi+"\"}}}}";
      
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.POST(payload);

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }

  Serial.println("Wait 10s before next round...");
  delay(10000);
}

void GetEvironment(){
  float newT = dht.readTemperature();
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

  float newH = dht.readHumidity();
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
  if(newH > 100){
    newH /= 2;
    newT *=2;
  }
  Humi = String(newH);
  Temp = String(newT);
  Time = GetTime();
}

String GetHumi(){
  float a = dht.readHumidity();
  return String(a);
}

String GetTime(){
  timeClient.update();
  String formattedTime = timeClient.getFormattedTime();
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  String currentMonthName = months[currentMonth-1];
  int currentYear = ptm->tm_year+1900;
  String currentDate = String(monthDay) + "-" + String(currentMonth) + "-" + String(currentYear);

  return formattedTime + " " + currentDate;
}
