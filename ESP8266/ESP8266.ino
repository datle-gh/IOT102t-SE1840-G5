//define blynk token
#define BLYNK_TEMPLATE_ID "TMPL6zLs-GJfK"
#define BLYNK_TEMPLATE_NAME "Pet Feeder"
#define BLYNK_AUTH_TOKEN "iL0Bwv9Lgm39QtufdqnMf9QVuRgJuYIV"

#define BLYNK_PRINT Serial

//include library
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
// #include <SoftwareSerial.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//wifi credential
char ssid[] = "Bich Tram"; //Wifi name :FPTU_Library     FPt-Student     Bích Trâm
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
//prevents spamming the Blynk.Cloud with thousands of messages from your hardware
BlynkTimer timer;

// //Set up pin to communicate with Arduino by UART
// #define RX_PIN D5 //D7  
// #define TX_PIN D6 //D8
// SoftwareSerial arduinoSerial(RX_PIN, TX_PIN);

// Địa chỉ I2C của LCD và Arduino
#define LCD_I2C_ADDRESS 0x27  // Địa chỉ I2C của LCD
#define ARDUINO_I2C_ADDRESS 8  // Địa chỉ I2C của Arduino

// Declare LCD (I2C address, columns, rows)
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, 16, 2);
#define SDA_PIN D2//D2
#define SCL_PIN D1//D1

//define variable
float weightLevelVal;
String message;

// variable to store feeding time
int feedingHour = 0;
int feedingMinute = 0;
// variable to store the current time
int currentHour = 0;
int currentMinute = 0;
int currentSeconds = 0;

//--------------------METHOD--------------------------------------

// Event handler from Blynk: Get Hour input
//Input: virtual pin V1 (on Blynk)
BLYNK_WRITE(VIRTUAL_PIN_HOUR_INPUT) {
  feedingHour = param.asInt();  // Get value from Slider 1 (hour)
  Serial.println("Time set: ");
  // Serial.println(feedingHour);
  printSerialTime(feedingHour, feedingMinute, 0);
}

// Event handler from Blynk: Get Minute input
//Input: virtual pin V2 (on Blynk)
BLYNK_WRITE(VIRTUAL_PIN_MINUTE_INPUT){
  feedingMinute = param.asInt();  // Get value from Slider 2 (minute)
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
      lcd.print("0");  // Add leading zero if minute < 10
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
  if((currentHour == feedingHour) && (currentMinute == feedingMinute) && (currentSeconds = 0))
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

void sendFeedingSignal() {
  Wire.beginTransmission(ARDUINO_I2C_ADDRESS);
  byte signal = (checkTimeSet()) ? 1 : 0;
  Wire.write(signal);
  Wire.endTransmission();
}

//call method after a time period based on timer.setInterval, ignore loop()
void myTimerEvent(){
  getRealTime();
  // receiveBlynkWeight();
  receiveWeightData();
  sendFeedingSignal();
  displayTimeSet();
}

//-------------------SET UP-------------------------------------------------

void setup() {
  //debug console
  Serial.begin(9600);
  //set up to communicate with ESP
  // arduinoSerial.begin(9600);
    //set up lcd
  Wire.begin(SDA_PIN, SCL_PIN); //set up I2C pins

  //set up Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Initialize NTP Client
  timeClient.begin();

  //set up a function to be called at any time
  //  5000 milliseconds (5 seconds), and the letter L is used to specify that this value is a long integer
  timer.setInterval(5000L , myTimerEvent);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for time");
}

//------------------------LOOP----------------------------------------------

void loop() {
  // Run Blynk
  Blynk.run();
  //run timer to send data periodically
  timer.run();


  // sendBlynkWeight();
  // send signal to feed
  if (checkTimeSet()) {
    Serial.println("Feeding the pet...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Feeding now!");
  }
  // delay(800);
}
