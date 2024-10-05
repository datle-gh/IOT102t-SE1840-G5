//Library
#include <Arduino.h>
#include <HX711.h>
#include <Servo.h>
#include <SoftwareSerial.h>

//define pin, variable, constraint

//Setup pin to communicate with esp by UART
#define RX_PIN 0 // RX pin for Software Serial
#define TX_PIN 1 // TX pin for Software Serial
SoftwareSerial espSerial(RX_PIN, TX_PIN); // RX = 0, TX = 1

  //load cell
const int LOADCELL_DOUT_PIN = A0;
const int LOADCELL_SCK_PIN = A1;
HX711 scale;

  //food door: servo motor
Servo foodDoor;  //servo object
const int foodDoorPin = 9;
const int weightLowestVal = 100;   //100g
const int weightHighestVal = 300;  //300g
float weightCurentVal = 0;
bool isDoorOpen = false;
char weightLevelVal[50];

  //led
const int ledPin =  4;
int ledState = LOW;

  //button
const int buttonPin =  7;
int buttonState = 0;

  //time set
int feedingHour = 0;
int feedingMinute = 0;

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

  Serial.println("Initializing the scale");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(-533.6609523810);  // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();                      // reset the scale to 0

  Serial.begin(9600);
  espSerial.begin(9600);
}

//-------------------------METHOD------------------------

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
    foodDoor.write(100);  // Rotate to 100 degree
    isDoorOpen = true;
  }
}

//rotate servo to close
void closeFoodDoor() {
  if(isDoorOpen == true){
    foodDoor.write(0);  // Rotate to 0 degree
    isDoorOpen = false;
  }
}

//check if the food amount has reached the maximum weight
bool checkEnoughFood(){
  bool isEnough = false;
  if(weightCurentVal >= weightHighestVal)
    isEnough = true;
  return isEnough;
}

//check if the feeding time has been set
bool checkTimeSet(){
  bool isTimeSet = false;
  if (espSerial.available()) {
    String command = espSerial.readStringUntil('\n');

    if (command == "FEED") {
      Serial.println("Feeding the pet...");
      isTimeSet = true;
    }
  }
  return isTimeSet;
}

//read data from load cell sensor
void readScale() {
  Serial.println("After setting up the scale:");
  weightCurentVal = scale.get_units();
}

//read the button state
void readButton(){
  buttonState = digitalRead(buttonPin);
}

//send data to esp
void sendESP(){
  dtostrf((float)33.3333, 10, 9, weightLevelVal);  // (float value, min width, precision, buffer)
  // Send data to ESP
  message[0] = '\0';
  strcat(message, weightLevelVal);
  espSerial.print(message); /*send string on request */
}

//-----------------------------LOOP-------------------------------------

void loop() {
  //Receive data from esp: time set
  bool isTimeReached = checkTimeSet();

  // Read load sensor value
  readScale();  // Read the weight value
  Serial.print("Food weight (g): ");
  Serial.println(weightCurentVal);  // Print the food weight value to Serial Monitor
  // Read button state
  readButton();

  // send data to esp
  sendESP();

  //check button state and feeding time to dispense food
  if ( buttonState || isTimeReached) {
    do {
      turnLedOn();
      openFoodDoor();
      readScale();
    } while (checkEnoughFood());
  }
  turnLedOff();
  closeFoodDoor();
  sendESP();

  //delay 1 minute
  delay(1000);
}
