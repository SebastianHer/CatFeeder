/*
Automatic Cat Feeder
*/


/* Step 1 : Define the libraries used, the toolboxes!*/
#include <LiquidCrystal.h>                                       //We'll need the LCD toolbox
#include <Servo.h>                                               //We'll need the Servo Toolbox
#include <DS3231.h>                                              // Real Time Clock toolbox
#include "pitches.h"                                             //To control the beeper so the cats know when it's feeding time

//-----------------------------------------------------------------------------------------------
/* Step 2: Define the components */
DS3231  rtc(SDA, SCL);                                           //We connect the RTC to the SDA and SCL
const int RS = 12, RW = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;      //We write down the pins for the LCD
LiquidCrystal lcd(RS, RW, d4, d5, d6, d7);

//Input & Button Logic
const int numOfInputs = 4;                                       // Define how many buttons
const int inputPins[numOfInputs] = {6,7,8,9};                    // define the pins for the buttons
int inputState[numOfInputs];                                     // Create an array for storing each button
int lastInputState[numOfInputs] = {LOW,LOW,LOW,LOW};             // Set all the buttons to Low to start
bool inputFlags[numOfInputs] = {LOW,LOW,LOW,LOW};                // Set all the flags to Low
long lastDebounceTime[numOfInputs] = {0,0,0,0};                  // 
long debounceDelay = 5;                                          // 
const int numOfScreens = 10;                                     // Pick here How many menus you want in our case 
int currentScreen = 0;                                           // First Menu will be stored in the screen 0 
String screens[numOfScreens][2] = {
  {},                                                            // Menu 0, is setup in the 
  {}, 
  {}, 
  {"Overload Temp.","degC"},
  {"Accel Time", "Secs"},
  {"Restart Time","Mins"},
  {"Analog Out. Curr.","mA"},
  {"Input Temp.","degC"},
  {"Run Time", "Hours"},
  {"Start Times","times"}};
int parameters[numOfScreens];

//----------------------------------------------------------------------------------------------
//Step 3 : Have some customization fun

//Special characters
 byte Cat[8]={B01001,B01111,B10111,B11101,B01110,B11101,B11100,B10011};
 byte Bell[8]={B00000,B00100,B01110,B01110,B01110,B01110,B11111,B00100};
 byte Clock[8]={B00100,B01010,B10001,B10011,B10101,B10101,B01110,B00100};

//Tone melody setup for the cats 
 int melody[]={NOTE_C4,NOTE_G3,NOTE_G3,NOTE_A3,NOTE_G3,0,NOTE_B3,NOTE_C4};             // notes in the melody:
 int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4};                                       // note durations: 4 = quarter note, 8 = eighth note, etc.:

//---------------------------------------------------------------------------------------------

void setup() {
 rtc.begin();                                                                          // Initialize the Real Time Clock
 //rtc.setTime(18,40,00);                                                              //Remove comments to set up time, then put back in comment and upload code again
 //rtc.setDate(13,6,2020);                                                             //Remove comments to set up date, then put back in comment and upload code again
 Serial.begin(9600);                                                                   // Set up the Baud rate for the serial for debugging if needed
 lcd.begin(16,2);                                                                      // Initialize the LCD 16 characters row, and 2 rows
 lcd.createChar(1, Cat);                                                               // Link the special characters created previously in the LCD
 lcd.createChar(2, Bell);
 lcd.createChar(3, Clock);
    for(int i = 0; i < numOfInputs; i++) {                                             // Initialize each button
    pinMode(inputPins[i], INPUT);                                                      // Set each pin as an inputs
    digitalWrite(inputPins[i], HIGH);                                                  // Set each input using the internal pull-up
    }    
    for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];                                  // to calculate the note duration, take one second divided by the note type.
    tone(8, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;                                        // to distinguish the notes, set a minimum time between them.the note's duration + 30% seems to work well:
    delay(pauseBetweenNotes);                                                           // add the delay previously setup
    noTone(8);                                                                          // stop the tone playing:
  }
}

void loop() {
  setInputFlags();
  resolveInputFlags();
}

void setInputFlags() {
  for(int i = 0; i < numOfInputs; i++) {                                                // Checking for the status of each button
    int reading = digitalRead(inputPins[i]);                                            // Checking on each individual button
    if (reading != lastInputState[i]) {                                                 // If the reading has changed
      lastDebounceTime[i] = millis();                                                   // Store Number of milliseconds passed since the program started
    }
    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (reading != inputState[i]) {
        inputState[i] = reading;
        if (inputState[i] == HIGH) {
          inputFlags[i] = HIGH;
        }
      }
    }
    lastInputState[i] = reading;
  }
}

void resolveInputFlags() {
  for(int i = 0; i < numOfInputs; i++) {
    if(inputFlags[i] == HIGH) {
      inputAction(i);
      inputFlags[i] = LOW;
      printScreen();
    }
  }
}

void inputAction(int input) {
  if(input == 0) {
    if (currentScreen == 0) {
      currentScreen = numOfScreens-1;
    }else{
      currentScreen--;
    }
  }else if(input == 1) {
    if (currentScreen == numOfScreens-1) {
      currentScreen = 0;
    }else{
      currentScreen++;
    }
  }else if(input == 2) {
    parameterChange(0);
  }else if(input == 3) {
    parameterChange(1);
  }
}

void parameterChange(int key) {
  if(key == 0) {
    parameters[currentScreen]++;
  }else if(key == 1) {
    parameters[currentScreen]--;
  }
}

void printScreen() {
  
  lcd.clear();
  if(currentScreen == 0){
  lcd.print("Cat Feeder");
  lcd.setCursor(0,1);
  lcd.print("Version 1.0");
  delay(3000);
  currentScreen++;
  }else if(currentScreen ==1){
   lcd.clear();
   lcd.setCursor(0,0);
   lcd.print("Time:   ");
   lcd.print(rtc.getTimeStr(FORMAT_SHORT));
   lcd.setCursor(0,1);
   lcd.print("Date: ");
   lcd.print(rtc.getDateStr(FORMAT_SHORT,FORMAT_BIGENDIAN));
   delay(1000); 
  }else {
  lcd.print(screens[currentScreen][0]);
  lcd.setCursor(0,1);
  lcd.print(parameters[currentScreen]);
  lcd.print(" ");
  lcd.print(screens[currentScreen][1]);
  }
}
