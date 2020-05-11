/*
 * AM320 Temperature and humidity monitoring using ESP8266 and the askSensors 
 * Description: This examples connects the ESP to wifi, and sends Temperature and humidity to askSensors IoT platfom over HTTPS GET Request.
 *  github: https://github.com/RVduino/RV-Storage-Monitor
 *  author: mcj7247@comcast.net
 * 
 */

// includes
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_AM2320.h"
Adafruit_AM2320 am2320 = Adafruit_AM2320();
#include <Wire.h>
#include <INA219.h>
INA219 monitor_A; // if more than 1 device you'll need to assign a unique I2C address _A(0x40) or _B or _C or _D. 
                  // empty addr "monitor_A  " is default = 0x40 but 41 - 45 can also be assigned.



// user config: TODO
const char* wifi_ssid = "xxxxx";             // SSID from your network
const char* wifi_password = "xxxx";   	   // WIFI password from your network
const char* apiKeyIn = "xxxxxxxxxxxxx";      // API KEY IN from your ASKSENSORS account
const unsigned int writeInterval = 25000; // write interval (in ms)

// ASKSENSORS config:
const char* https_host = "api.asksensors.com";         // ASKSENSORS host name
const int https_port = 443;                        // https port
const char* https_fingerprint =  "B5 C3 1B 2C 0D 5D 9B E5 D6 7C B6 EF 50 3A AD 3F 9F 1E 44 75";     // ASKSENSORS HTTPS SHA1 certificate
int status = WL_IDLE_STATUS;
float myTemperature = 0, myHumidity = 0; 

// voltage divider config: if using voltage divider instead of INA219 module be sure to comment out the INA219 voltage module code below
float Aref = 3.21; // ***calibrate here*** | change this to the actual Aref or (Vcc) voltage of ---YOUR--- Arduino
unsigned int total; // can hold max 64 readings
float voltage;      // converted to volt

// create ASKSENSORS client
WiFiClientSecure client;
// 
void setup() {
  am2320.begin(); // initiate the humidity sensor function
  monitor_A.begin();  // initiate the power sensor function
  Wire.begin(5,4); // initiate the I2C function and assign (SDA,SCL) pins
  Serial.begin(115200); // initiate the serial print function
  Serial.println();
  Serial.print("********** connecting to WIFI : ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password); // initiate the Wifi function and pass it your local wifi logon data
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("-> WiFi connected");
  Serial.println("-> IP address: ");
  Serial.println(WiFi.localIP());
  
  // Set Client to insecure
  client.setInsecure();
}

void loop() {
// Use WiFiClientSecure class to create TLS connection
  Serial.print("********** connecting to HOST : ");
  Serial.println(https_host);
  if (!client.connect(https_host, https_port)) {
    Serial.println("-> connection failed");
    //return;
  }

// Read data from voltage divider. Comment this code out if using the INA219 module below for voltage. 
//  for (int x = 0; x < 64; x++) { // multiple analogue readings for averaging
//    total = total + analogRead(A0); // add each value to a total
    //Serial.println(analogRead(A0));
//  }
//  voltage = (total / 64) * 14.38 / 1024;
// convert readings to volt. adjust scaling factor based on your resistor values 
// print to serial monitor
//  if (total == (1023 * 64)) { // if overflow
//    Serial.print("voltage too high");
//  }
//  else {
    //Serial.print("The battery is ");
    //Serial.print(voltage);
    //Serial.println(" volts");
//  }
//  total = 0; // reset value

// Read data from power monitor INA219 via I2C
Serial.print(monitor_A.shuntCurrent() * 1000, 3);Serial.print(",");
Serial.println(monitor_A.busVoltage(), 3);
voltage = monitor_A.busVoltage();
  
  // Create a URL for the request
  String url = "/write/";
  url += apiKeyIn;
    url += "?module1=";
    myTemperature = am2320.readTemperature();
    myTemperature = (myTemperature*9/5)+32; 
    myHumidity = am2320.readHumidity();
    //Serial.println(am2320.readTemperature());
    //Serial.println(am2320.readHumidity());
    
  url += myTemperature;
  url += "&module2=";
  url += myHumidity;
  url += "&module3=";
  url += voltage;
  
  
  Serial.print("********** requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + https_host + "\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("> Request sent to ASKSENSORS");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
        String line = client.readStringUntil('\n');
        Serial.println("********** ASKSENSORS reply:");
        Serial.println(line);
        Serial.println("********** closing connection");
      
        break;
    }
  }

  delay(writeInterval );     // delay in msec
}
