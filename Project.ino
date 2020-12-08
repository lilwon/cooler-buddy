// Using ESP-01 to send AT commands to ThingSpeak

#include "ThingSpeak.h"
#include "WiFiEsp.h"
#include <Wire.h>
#include <math.h>
#include "rgb_lcd.h"

#define FANPIN 3  // Arctic PWM Fan
#define TEMPPIN A0 // Temp sensor
#define TOUCHPIN 4 // touch sensor
#define LEDPIN 7  // led light

WiFiEspClient client; 

#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial esp(10, 11);
#define ESP_BAUDRATE  19200
#else
#define ESP_BAUDRATE  115200 
#endif

int fanSetting = 0;
rgb_lcd lcd; 

// data for ThingSpeak and other
float temp; 
int rpm; 
unsigned long intr = 0; // interrupt timer

void setup() {
  Serial.begin(115200);
  //esp.begin(115200); // must set baudrate with esp.. 

  // Setting baudrate of esp
  setEspBaudRate(ESP_BAUDRATE);

  lcd.begin(16, 2); 
  lcd.print("Fan Setup"); 
  
  // make fan speed low
  pinMode(FANPIN, OUTPUT);
  // reset the timers on Arduino board to change 25kHz
  pwmFanSetup();

  // Setup ESP8266
  Serial.print("Searching for ESP8266");
  WiFi.init(&esp);

  if ( WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true); 
  }

  Serial.println("Connected");

  ThingSpeak.begin(client);

  // Interrupt on PIN 2 tach
  digitalWrite(2, HIGH); 
}

void loop() {
  // check status of network
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to network: ");
    Serial.println("");
    while (WiFi.status() != WL_CONNECTED ) {
      // testing if string works..
      // WiFi.begin(SSID, PW) 
      WiFi.begin("", ""); 
      Serial.print("."); 
      delay(5000); 
    }
    Serial.println("\nConnected."); 
  }

  // Start getting data! 
  int touch = digitalRead(TOUCHPIN); // if it's touched.. 
  // if touch sensor touched show confirmation on board. 
  if ( touch == 1 ) {
    digitalWrite(LEDPIN, HIGH); 
    fanSetting++; // set fan setting + 1

    if ( fanSetting > 3 ) {
      fanSetting = 0; // reset the fan
    }
  }
  else {
    digitalWrite(LEDPIN, LOW); 
  }
  
  calcTemp(); // get the temperature Sensor
  Serial.print("Fan Setting: ");
  Serial.println(fanSetting);   
  Serial.print("Temp: ");
  Serial.println(temp);
    
  lcd.clear();
  lcd.print("Temp(F): ");
  lcd.print(temp);

  // Manual or auto
  if (fanSetting > 0 ) {
    manMode(fanSetting); 
  }
  else {
    autoMode(temp);
  }
    
  calcRPM(); 
  Serial.print("RPM: ");
  Serial.println(rpm); 

  lcd.setCursor(0, 1);
  lcd.print("RPM: ");
  lcd.print(rpm); 

  lcd.setCursor(10, 1);
  lcd.print("Md: ");
  lcd.print(fanSetting);

  // Start sending data to ThingSpeak
  // Set Fields with vals
  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, rpm);

  // Write to thingspeak channel
  // ThingSpeak.writeFields(channelID, API)
  int x = ThingSpeak.writeFields(, "");
  // send HTTP 200
  if ( x == 200 ) {
    Serial.println("Successfully updated.");
  }
  else {
    Serial.println("HTTP error code " + String(x)); 
  }
  // Free accounts get a 15s update.
  delay(20000); 
}

// Used to setup 25kHz frequency to PWM Fan
// Have to use the registers to reset clocks/timing
void pwmFanSetup() {
  TCCR2A = 0;
  TCCR2B = 0;

  TCCR2A |= ( 1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
  TCCR2B |= (1 << WGM22) | ( 1 << CS21); 

  OCR2A = 255; // this controls MAX duty/pwm cycle
  OCR2B = 0; // change this to control PWM speed
}

void calcTemp() {
  int a = analogRead(TEMPPIN);
  float R = 1023.0/a-1.0; 
  R = 100000 * R; 
  // conv to temperature C
  float tempC = 1.0/(log(R/100000)/4275+1/298.15)-273.15; 
  temp = (tempC * 1.8) + 32.0; // convert to F
}

// calculate RPM
void calcRPM() {
  intr = pulseIn(2, HIGH); // get interrupt/tachometer 
  rpm = ( 1000000 * 60 ) / (intr * 4 ); 
}

// Switch to manual setting
// Overrides auto mode
void manMode(int curFan ) {
  if ( curFan == 3 ) {
    OCR2B = 255; // max rpm'
  }
  else if ( curFan == 2 ) {
    OCR2B = 192; // 75% duty cycle
  }
  else if ( curFan == 1 ) {
    OCR2B = 128;  // 50% 
  }  
}

// Automode, only reads by temperature. 
void autoMode(float currentTemp) {
  if ( currentTemp >= 90.0 ) {
    OCR2B = 255; 
  }
  else if ( currentTemp >= 85 && currentTemp < 90 ) {
    OCR2B = 192;
  }
    else if ( currentTemp > 80 && currentTemp < 85 ) {
    OCR2B = 128;
  }
  else {
    // default speed when nothing is set/used.
    OCR2B = 64; // 25% duty cycle = default speed
  }  
}

// This function attempts to set the ESP8266 baudrate. Boards with additional hardware serial ports
// can use 115200, otherwise software serial is limited to 19200.
void setEspBaudRate(unsigned long baudrate){
  long rates[6] = {115200,74880,57600,38400,19200,9600};

  Serial.print("Setting ESP8266 baudrate to ");
  Serial.print(baudrate);
  Serial.println("...");

  for(int i = 0; i < 6; i++){
    esp.begin(rates[i]);
    delay(100);
    esp.print("AT+UART_DEF=");
    esp.print(baudrate);
    esp.print(",8,1,0,0\r\n");
    delay(100);  
  }
    
  esp.begin(baudrate);
}