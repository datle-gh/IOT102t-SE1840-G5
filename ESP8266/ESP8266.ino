////define pin with GPIO 
//static const uint8_t D0 = 16;
//static const uint8_t D1 = 5;
//static const uint8_t D2 = 4;
//static const uint8_t D3 = 0;
//static const uint8_t D4 = 2;
//static const uint8_t D5 = 14;
//static const uint8_t D6 = 12;
//static const uint8_t D7 = 13;
//static const uint8_t D8 = 15;
//static const uint8_t D9 = 3;
//static const uint8_t D10 = 1;

//define blynk token
#define BLYNK_TEMPLATE_ID "TMPL6zLs-GJfK"
#define BLYNK_TEMPLATE_NAME "Pet Feeder"
#define BLYNK_AUTH_TOKEN "iL0Bwv9Lgm39QtufdqnMf9QVuRgJuYIV"

#define BLYNK_PRINT Serial

//include library
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SoftwareSerial.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//wifi credential
char ssid[] = "FPt-Student"; //Wifi name
char pass[] = "87654321"; //Wifi password

//Blynk: Virtual timer
#define VIRTUAL_PIN_HOUR_INPUT V1
#define VIRTUAL_PIN_MINUTE_INPUT V2
#define VIRTUAL_PIN_WEIGHT_OUTPUT V3

//Blynk: Set up Real time on blynk
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 7 * 3600, 60000);

//Blynk: creates the timer object. 
//allows you to send data periodically. 
//prevents spamming the Blynk.Cloud with thousands of messages from your hardware
BlynkTimer timer;

//Set up pin to communicate with Arduino by UART
#define RX_PIN D9 
#define TX_PIN D10 
SoftwareSerial arduinoSerial(RX_PIN, TX_PIN);

// Declare LCD (I2C address, columns, rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define SDA_PIN D2
#define SCL_PIN D1

//define variable
float weightLevelVal;
String message;

// variable to store feeding time
int feedingHour = 0;
int feedingMinute = 0;
// variable to store the current time
int currentHour = 0;
int currentMinute = 0;

//-------------------SET UP-------------------------------------------------

void setup() {
  //debug console
  Serial.begin(9600);
  //set up to communicate with ESP
  arduinoSerial.begin(9600);

  //set up Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Initialize NTP Client
  timeClient.begin();

  //set up a function to be called at any time
  //  5000 milliseconds (5 seconds), and the letter L is used to specify that this value is a long integer
  timer.setInterval(5000L , myTimerEvent);

  //set up lcd
  Wire.begin(SDA_PIN, SCL_PIN); //set up I2C pins
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for time");
}

//--------------------METHOD--------------------------------------

// Event handler from Blynk: Get Hour input
//Input: virtual pin V1 (on Blynk)
BLYNK_WRITE(VIRTUAL_PIN_HOUR_INPUT) {
  feedingHour = param.asInt();  // Get value from Slider 1 (hour)
  Serial.print("Selected Hour: ");
  Serial.println(feedingHour);
}

// Event handler from Blynk: Get Minute input
//Input: virtual pin V2 (on Blynk)
BLYNK_WRITE(VIRTUAL_PIN_MINUTE_INPUT){
  feedingMinute = param.asInt();  // Get value from Slider 2 (minute)
  Serial.print("Selected Minute: ");
  Serial.println(feedingMinute);
}

//call method after a time period based on timer.setInterval, ignore loop()
void myTimerEvent(){
  getRealTime();
  sendBlynkWeight();
  displayTimeSet();
}

//Receive data from Arduino and send the weight value to Blynk
void sendBlynkWeight(){
  message = "";
  Serial.println(message);
  message = arduinoSerial.read();
  weightLevelVal = message.toFloat(); 
  Blynk.virtualWrite(VIRTUAL_PIN_WEIGHT_OUTPUT, weightLevelVal);
}

// Get current time through Blynk
void getRealTime(){
  timeClient.update();
  currentHour = timeClient.getHours();
  currentMinute = timeClient.getMinutes();
}

// Display the time on LCD
void displayTimeSet(){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Feed Time Set:");
    lcd.setCursor(0, 1);
    lcd.print(feedingHour);
    lcd.print(":");
    if (feedingMinute < 10) {
      lcd.print("0");  // Add leading zero if minute < 10
    }
    lcd.print(feedingMinute);
}

//------------------------LOOP----------------------------------------------

void loop() {
  // Run Blynk
  Blynk.run();
  //run timer to send data periodically
  timer.run();

  //send signal to feed
  if (currentHour == feedingHour && currentMinute == feedingMinute) {
    Serial.println("Feeding the pet...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Feeding now!");
    arduinoSerial.println("FEED");
  }
}
