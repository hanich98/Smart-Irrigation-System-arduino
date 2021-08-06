#include <Wire.h>  // I2C protocol
#include <Adafruit_BMP280.h>
#include <WiFi.h>

#include "SPIFFS.h"
#include "time.h"
#include "ESPmDNS.h"
#include "WebServer.h"

#include "ESPAsyncWebServer.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"
#include <SimpleKalmanFilter.h>
#include "ESP32_MailClient.h"

const char *host = "esp32"; // hostname that is used to access the webpage

const char *ssid = "LAPTOP";          // the name of the local router
const char *password = "76930139";    // the password of the router

const char *http_username = "admin";  // login username
const char *http_password = "admin";  // login password

AsyncWebServer server(80);

String daysOfTheWeek[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char *ntpServer = "pool.ntp.org";  // Network Time Protocol
const long gmtOffset_sec = 7200; // in seconds
const int daylightOffset_sec = 3600;
String Time = "", Date = "";
bool SetTime = false;
int h, m, period;
struct tm timeinfo;
int pump_pin = 32;

int Rain_Prob = 0; // number of rain probability during the next 12 hours

void Get_LocalTime()
{

    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }
    Time = String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min) + ":" + String(timeinfo.tm_sec) + " - " + String(daysOfTheWeek[timeinfo.tm_wday]);
    Date = " - " + String(timeinfo.tm_mday) + "/" + String(timeinfo.tm_mon + 1) + "/" + String(timeinfo.tm_year + 1900);
    // return (String(Time) + String(Date));
    // Serial.println(String(Time) + String(Date));
}

void connectWiFi()
{
    // Connect to Wi-Fi network with SSID and password
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Get_LocalTime();
    if (MDNS.begin(host))
    {
        Serial.println("MDNS responder started");
    }
    // Print local IP address and start web server
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    pinMode(2, OUTPUT);
    pinMode(36, INPUT); // Battery voltage pin
    pinMode(pump_pin, OUTPUT);
    digitalWrite(pump_pin, LOW);
    digitalWrite(2, HIGH);
    delay(1000);
    digitalWrite(2, LOW);
}


void setup() {
  Serial.begin(115200);
  Time_Update(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  BMP_Setup();
  connectWiFi();
  File_System();
  Server();
  GetWxData();
}

void loop() {
  Get_LocalTime();
  Time_Update(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  Update_weather(timeinfo.tm_min, timeinfo.tm_sec);

}
