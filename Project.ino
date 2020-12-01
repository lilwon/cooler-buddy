#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include <WiFiEspServer.h>
#include <WiFiEspUdp.h>

#include <math.h>
#include <Wire.h>
#include "rgb_lcd.h"

#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(10, 11);
#endif

// switching between 9 and 3 depends on TACH pin
#define PWMPIN 3 // controls the PWM of fan

// Set up the temp sensor
int tempSensor = A0; 

float temp; // show current temp in room

char ssid[]= "X";
char pass[] = "X";
int status = WL_IDLE_STATUS; 
char server[] = "X"; 

// change to char*? 
char get_request[300];
// Initialize Ethernet client
WiFiEspClient client;

rgb_lcd lcd; // display lcd

void setup() {
  Serial.begin(115200);
 
  lcd.begin(16, 2);
  lcd.print("Setup Fan");

  // ESP8266 Wifi stuff
  Serial1.begin(115200); 
  WiFi.init(&Serial1);
  // check for presence of shield
  if ( WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // dont continue
    while (true);
  }
  // atmpt to connect to wifi
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // connect to wpa/wpa2 network
    status = WiFi.begin(ssid, pass);
  }
  Serial.println("You're connected to the network");
  printWifiStatus(); 

  // now connect PinMode of fan
  pinMode(PWMPIN, OUTPUT);
  // fan is set to low, it will always continue running even if 
  // there's no output bc connected fan pins..
  analogWrite(PWMPIN, 0); // RPM = 230
  delay(5000); // make sure NOTHING is spinning

}

void loop() {  
  temp = calcTemp(); 
  Serial.print("Temperature (F): ");
  Serial.println(temp);

  lcd.clear();
  lcd.print("Temp(F): ");
  lcd.print(temp);

  if ( temp > 80.0 ) {
    // 100% cycle 
    analogWrite(PWMPIN, 255); //rpm = 1350
  }
  else {
    // 20% cycle
    analogWrite(PWMPIN, 64);// rpm ~800
  }

  // Connection to Server 
  Serial.println();
  
  if (!client.connected() ) {
    Serial.println("Starting connection to server...");
    client.connect(server, 5000);
  }

  // Serial.println("Connected to server");
  char tempVal[6];
  dtostrf(temp, 3, 2, tempVal); 

  // char tempHum[6] = "5.0";
  
  //sprintf(get_request,"GET /?var=%s HTTP/1.1\r\nHost: 3.15.181.173\r\nConnection: close \r\n\r\n", var);
  //sprintf(get_request, "GET /?temp=%d HTTP/1.1\r\nHost: 3.15.181.173\r\nConnection: close \r\n\r\n", temp);
  sprintf(get_request, "GET /?tempVal=%s HTTP/1.1\r\nHost: 3.15.181.173\r\nConnection: close \r\n\r\n", tempVal);

  client.print(get_request);
  delay(500);
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }
  delay(10000);
}

// Calculates the temperature from thermistor to Celsius to Fahrenheit
// Adapted from SEEED Grove from their thermistor sensor
float calcTemp() {
  int a = analogRead(tempSensor);
  float R = 1023.0/a-1.0; 
  R = 100000 * R; 

  // conv to temperature C
  float tempC = 1.0/(log(R/100000)/4275+1/298.15)-273.15; 
  float tempF = (tempC * 1.8) + 32.0; 

  return tempF; 
}

// To print Wifi Status 
void printWifiStatus() {
  // print SSID 
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // print wifi shield ip adr
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip); 

  // print received signal str
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}
