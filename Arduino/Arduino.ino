#include <Wire.h>
#include <Arduino.h>
#include <HX711.h>
#include <Servo.h>

//define pin, variable, constraint
#define ESP8266_I2C_ADDRESS 8

  //load cell
const int LOADCELL_DOUT_PIN = A0;
const int LOADCELL_SCK_PIN = A1;
HX711 scale;
const float CALIBRATING = 412.234;// this value is obtained by calibrating the scale with known weights
const int weightHighestVal = 300;  //300g
float weightCurrentVal = 0;
float weightFoodSpout = 22;

  //food door: servo motor
Servo foodDoor;
const int foodDoorPin = 9;
bool isDoorOpen = false;
int closedAngle = 0;
int openedAngle = 130;
bool isTimeReached;

  //led
const int ledPin =  4;
int ledState = LOW;

  //button
const int buttonPin =  7;
int buttonState = LOW;

  //message between esp and arduino
char message[100];

//--------------------SET UP----------------------------

void setup() {
  //button 
  pinMode(buttonPin, INPUT);  

  //led
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  //servo set up
  foodDoor.attach(foodDoorPin);
  foodDoor.write(closedAngle); 

  Serial.println("Initializing the scale");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(CALIBRATING); // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();                      // reset the scale to 0

  Serial.begin(9600);
  // espSerial.begin(9600);
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

  // Read load sensor value
  readScale();
  readButton();
  // if(buttonState == HIGH){
  //   Serial.println("Button pressed!");
  // }

  //check button state and feeding time to dispense food
  if ( (buttonState == HIGH) || isTimeReached) {
    do {
      turnLedOn();
      openFoodDoor();
      readScale();
      delay(800);
    } while (!checkEnoughFood());
    isTimeReached = false;
  }
  turnLedOff();
  closeFoodDoor();
  delay(800);
}