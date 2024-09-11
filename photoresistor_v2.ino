#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_sleep.h>
#include "time.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

const char *ssid = "xxxxxxxxxxxxxxx";
const char *password = "xxxxxxxxxxxxx";
const char* ntpServer = "pool.ntp.org";

const int sensorPin = 2;
String timeStamp;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

int out = 0; // timeout counter for updating time

double hour = 0; // current time displayed as a single hour
int ontime = 0;

void setup() { 
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); 
  connectToWifi();
  timeClient.begin();
  timeClient.setTimeOffset(-18000); 
  while(!timeClient.update()) {    // Get EST network time
    timeClient.forceUpdate();
    delay(1000);
    out++;
    if (out >= 10){
      ESP.restart();
    }
  }
  ontime = getOntime(); // Gets time the device should be active for
  Serial.println(timeClient.getFormattedTime());
  Serial.println("Will be active for " + String(ontime / (60.0 * 60)) + " hours.");
  WiFi.mode(WIFI_OFF); // Wifi off to save power
}

void loop() { // checks light level every second
  checkDeepSleep(); // Chechs if ESP32 should be sleeping
  int analogValue = analogRead(sensorPin); // Reads from photoresistor
  if (analogValue > 700){ // If light level is past 700, ping the server
    WiFi.mode(WIFI_STA);
    connectToWifi();
    sendToServer(analogValue);
    WiFi.mode(WIFI_OFF);
    delay(5000);
    ontime -= 5;
  }
  delay(1000);
  ontime--;   // decrements its active time every second 
}


void checkDeepSleep(){
  if (hour >= 16.5){
    sleep(24.0 - (hour - 12)); // If ESP32 is turned on after 16:00, sleep for 24 - (hour - 12)
  }
  if (hour < 12){
    sleep(12.0 - hour); // If ESP32 is turned on before 12:00, sleep for 12 - hour
  }
  if (ontime <= 0){  // Sleeps for 10 hrs if time is 16:00, wakes up at midnight to sync clock, goes back to sleep until 12:00
    sleep(10.0);
  }
}

int getOntime(){ // Calculates how long esp32 should stay awake for 
  String time1 = timeClient.getFormattedTime();
  hour = (time1.substring(0,2).toInt()) + (time1.substring(3,5).toInt() / 60.0) + (time1.substring(6).toInt() / 3600.0);
  return (16.5 - hour) * 60 * 60;
}

void sleep(double hrs){ // Sleep method for a number of hours
  uint64_t microseconds = hrs * 3600 * 1000000;
  Serial.println("Going to sleep for " + String(hrs) + " hours; "+ String(microseconds));
  delay(100);
  esp_sleep_enable_timer_wakeup(microseconds);
  esp_deep_sleep_start();
}

void connectToWifi(){
  int temp = 0;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    ontime--;
    Serial.println("Connecting to WiFi...");
    temp++;
    if (temp % 10 == 0){ // Reconnects if it takes longer than 10 sec
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
    delay(1000);
  }
  Serial.println("Connected.");
}

void sendToServer(int32_t rssi) { // Sends photoresistor value to server 
  HTTPClient http;

  String url = "http://192.168.1.182:3000/storeSignalStrength?rssi=" + String(rssi);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}
