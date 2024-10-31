#define BLYNK_TEMPLATE_ID "TMPL6zLs-GJfK"
#define BLYNK_TEMPLATE_NAME "Pet Feeder"
#define BLYNK_AUTH_TOKEN "iL0Bwv9Lgm39QtufdqnMf9QVuRgJuYIV"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//wifi credential
char ssid[] = "Bich Tram"; //Wifi name :FPTU_Library     FPt-Student     Bích Trâm   Bich Tram
char pass[] = "18042004"; //Wifi password         18042004

//Blynk: Virtual timer
#define VIRTUAL_PIN_HOUR_INPUT V1
#define VIRTUAL_PIN_MINUTE_INPUT V2
#define VIRTUAL_PIN_WEIGHT_OUTPUT V3

//Blynk: Set up Real time on blynk
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 7 * 3600, 60000);

//Blynk: creates the timer object. 
//allows you to send data periodically. 
BlynkTimer timer;

// Địa chỉ I2C của LCD và Arduino
#define LCD_I2C_ADDRESS 0x27  // Địa chỉ I2C của LCD
#define ARDUINO_I2C_ADDRESS 8  // Địa chỉ I2C của Arduino

// Declare LCD (I2C address, columns, rows)
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, 16, 2);
#define SDA_PIN D2
#define SCL_PIN D1

float weightLevelVal;
String message;

int feedingHour = 0;
int feedingMinute = 0;

int currentHour = 0;
int currentMinute = 0;
int currentSeconds = 0;

//--------------------METHOD--------------------------------------

//Get Hour input
BLYNK_WRITE(VIRTUAL_PIN_HOUR_INPUT) {
  feedingHour = param.asInt(); 
  Serial.println("Time set: ");
  // Serial.println(feedingHour);
  printSerialTime(feedingHour, feedingMinute, 0);
}

//Get Minute input
BLYNK_WRITE(VIRTUAL_PIN_MINUTE_INPUT){
  feedingMinute = param.asInt();
  Serial.println("Time set: ");
  // Serial.println(feedingMinute);
  printSerialTime(feedingHour, feedingMinute, 0);
}


// Get current time through Blynk
void getRealTime(){
  timeClient.update();
  currentHour = timeClient.getHours();
  currentMinute = timeClient.getMinutes();
  currentSeconds = timeClient.getSeconds();
  Serial.println("Real time: ");
  printSerialTime(currentHour, currentMinute, currentSeconds);
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
      lcd.print("0"); 
    }
    lcd.print(feedingMinute);
}

void printSerialTime(int hour, int minute, int seconds){
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.println(seconds);
}

bool checkTimeSet(){
  bool isTimeReached = false;
  if((currentHour == feedingHour) && (currentMinute == feedingMinute) && (currentSeconds == 0))
  {
    isTimeReached = true;
  }
  return isTimeReached;
}

//Receive data from Arduino and send the weight value to Blynk
void receiveWeightData() {
  Wire.requestFrom(ARDUINO_I2C_ADDRESS, sizeof(weightLevelVal));
  if (Wire.available()) {
    byte buffer[sizeof(weightLevelVal)];
    for (int i = 0; i < sizeof(weightLevelVal); i++) {
      buffer[i] = Wire.read();
    }
    memcpy(&weightLevelVal, buffer, sizeof(weightLevelVal));
    Serial.print("Received weight: ");
    Serial.println(weightLevelVal);
    Blynk.virtualWrite(VIRTUAL_PIN_WEIGHT_OUTPUT, weightLevelVal);  // Gửi dữ liệu đến Blynk
  }
}

//send signal to open food door
void sendFeedingSignal() {
  Wire.beginTransmission(ARDUINO_I2C_ADDRESS);
  byte signal = (checkTimeSet()) ? 1 : 0;
  Wire.write(signal);
  Wire.endTransmission();
}

//call method after a time period based on timer.setInterval, ignore loop()
void myTimerEvent(){
  getRealTime();
  receiveWeightData();
  sendFeedingSignal();
  displayTimeSet();
}

//-------------------SET UP-------------------------------------------------

void setup() {
  Serial.begin(9600);

  Wire.begin(SDA_PIN, SCL_PIN); //set up I2C pins

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Initialize NTP Client
  timeClient.begin();

  //set up a function to be called at any time
  timer.setInterval(1000L , myTimerEvent);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for time");
}

//------------------------LOOP----------------------------------------------

void loop() {
  Blynk.run();
  timer.run();

  // send signal to feed
  if (checkTimeSet()) {
    Serial.println("Feeding the pet...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Feeding now!");
  }
}
