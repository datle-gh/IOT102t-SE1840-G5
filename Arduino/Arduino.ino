#include <Wire.h>
#include <Arduino.h>
#include <HX711.h>
#include <Servo.h>

#define ESP8266_I2C_ADDRESS 8

  //load cell
const int LOADCELL_DOUT_PIN = A0;
const int LOADCELL_SCK_PIN = A1;
HX711 scale;
const float CALIBRATING = 413.96;// this value is obtained by calibrating the scale with known weights
const int weightHighestVal = 100;
float weightCurrentVal = 0;
float weightFoodSpout = 18;

  //food door: servo motor
Servo foodDoor;
const int foodDoorPin = 9;
bool isDoorOpen = false;
int closedAngle = 0;
int openedAngle = 90;
bool isTimeReached;

const int ledPin =  4;
int ledState = LOW;

const int buttonPin =  7;
int buttonState = LOW;

//--------------------SET UP----------------------------

void setup() {
  pinMode(buttonPin, INPUT);  

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  foodDoor.attach(foodDoorPin);
  foodDoor.write(closedAngle); 

  Serial.println("Initializing the scale");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(CALIBRATING); // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();                    

  Serial.begin(9600);

  Wire.begin(ESP8266_I2C_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}

//-------------------------METHOD------------------------

//read the button state
void readButton(){
  buttonState = digitalRead(buttonPin);
}

//turn on the led
void turnLedOn(){
  if(ledState == LOW){
    ledState = HIGH;
  }
  digitalWrite(ledPin, ledState);
}

//turn off the led
void turnLedOff(){
  if(ledState == HIGH){
    ledState = LOW;
  }
  digitalWrite(ledPin, ledState);
}

//rotate servo to open
void openFoodDoor() {
  if(isDoorOpen == false){
    foodDoor.write(openedAngle);  
    isDoorOpen = true;
  }
}

//rotate servo to close
void closeFoodDoor() {
  if(isDoorOpen == true){
    foodDoor.write(closedAngle); 
    isDoorOpen = false;
  }
}

//check if the food amount has reached the maximum weight
bool checkEnoughFood(){
  bool isEnough = false;
  if(weightCurrentVal >= weightHighestVal){
    isEnough = true;
  }
  return isEnough;
}

//read data from load cell sensor
void readScale() {
  Serial.println("After setting up the scale:");
  weightCurrentVal = scale.get_units();
  weightCurrentVal = weightCurrentVal - weightFoodSpout;
  if(weightCurrentVal <=0){
    weightCurrentVal = 0;
  }
  Serial.print("Food weight (g): ");
  Serial.println(weightCurrentVal);
}

// Hàm gửi dữ liệu cân nặng cho ESP8266 khi có yêu cầu
void requestEvent() {
  Wire.write((byte*)&weightCurrentVal, sizeof(weightCurrentVal));
}

// Hàm nhận tín hiệu từ ESP8266 (Master) để cho ăn
void receiveEvent(int bytes) {
  if (bytes == 1) {
    byte signal = Wire.read();
    if (signal == 1) {
      Serial.println("Time to feed the pet!");
      isTimeReached = true;
    }
  }
}

//-----------------------------LOOP-------------------------------------

void loop() {
  readScale();
  readButton();

  //check button state and feeding time to dispense food
  if ( (buttonState == HIGH) || isTimeReached) {
    do {
      turnLedOn();
      openFoodDoor();
      readScale();
    } while (!checkEnoughFood());
    isTimeReached = false;
  }
  turnLedOff();
  closeFoodDoor();
}