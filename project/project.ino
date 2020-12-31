/* project global includes */
#include <inttypes.h>           // standard library
#include "macros.h"             // self implemented
#include "Event.h"              // locally included
#include "Timer.h"              // locally included
#include <LiquidCrystal.h>      // system library
#include <Keypad.h>        // GDY120705

/* initialize the library by associating any needed LCD interface pin
 * with the arduino pin number it is connected to */
const int rs = 13, en = 12, d4 = A2, d5 = A3, d6 = A4, d7 = A5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'7','8','9'},
  {'4','5','6'},
  {'1','2','3'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {5, 4, 3, 10}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {8, 7, 6}; //connect to the column pinouts of the keypad
  
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

String inputString;
long inputInt;
float flowRate; /* ml/hr */
const int maxFlowRateApplicable = 150; /* ul/hr */
float remainingVolume; /* ml */
const int syringeSize = 10; /* ml */
boolean pauseFlag = false;

void pauseToRefill(void){
  pauseFlag = !pauseFlag;
  lcd.clear();
  lcd.print("paused");
  while(digitalRead(2) == LOW){}
}

int LevelSensorPin = A0;
int motorPin = 9;
int pauseButton = 2;
int buzzerAlarmPin = 11;

void setup() {
  pinMode(motorPin, OUTPUT);
  pinMode(pauseButton, INPUT_PULLUP);
  pinMode(buzzerAlarmPin, OUTPUT);

  /* set system's external interrupts 0 to pause system ISR in pin 2 */
  attachInterrupt(0, pauseToRefill, LOW);
  
  Serial.begin(9600);
  lcd.begin(16, 4);
  lcd.setCursor(0,0);
  pinMode(9, OUTPUT);
  analogWrite(9, 0);


}

void loop() {

  /************input volume task ****************************/

  boolean applicableVol = false;
  while(applicableVol == false)
  {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Volume(ul): ");
  lcd.setCursor(2, 1);
  /* inputing some Multy digit value -*/
  inputString = "";               // clear input
  char key;
  do{
    key = keypad.getKey();
  //  Serial.println(key);
    if (key >= '0' && key <= '9') {     // only act on numeric keys
      inputString += key;               // append new character to input string
      lcd.print(key);
    }
  }while(key != '#'); 
  if (inputString.length() > 0) 
  {
    inputInt = inputString.toInt(); // YOU GOT AN INTEGER NUMBER
  }
  remainingVolume = inputInt/1000; /* ml */
  if(remainingVolume > syringeSize * 1000)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("error");
    lcd.setCursor(0,1);
    lcd.print("too much volume");
    delay(2000);
    applicableVol = false;
  }
  else
  {
    applicableVol = true;
  }
  }

    /************input Rate task ****************************/  
  boolean applicableRate = false;
  while(applicableRate == false)
  {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Rate(ul/hr): ");
  lcd.setCursor(2, 1);
  /* inputing some Multy digit value -*/
  inputString = "";               // clear input
  char key;
  do{
  key = keypad.getKey();
//  Serial.println(key);
  if (key >= '0' && key <= '9') {     // only act on numeric keys
    inputString += key;               // append new character to input string
    lcd.print(key);
  }
  }while(key != '#'); 
  if (inputString.length() > 0) 
  {
    inputInt = inputString.toInt(); // YOU GOT AN INTEGER NUMBER
    flowRate = inputInt/1000.0; /* ml/hr */
  }
  
  if(flowRate > maxFlowRateApplicable/1000.0)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("error");
    lcd.setCursor(0,1);
    lcd.print("high rate");
    delay(2000);
    applicableRate = false;
  }
  else
  {
    applicableRate = true;
  }
  }

  while(true)
  {
    if(pauseFlag == false)
    {
    boolean stableFlow = true;
  
    while(stableFlow)
    {
      /* calculate the syringe fluid percentage */
      int SensorValue = analogRead(LevelSensorPin);
//      int CurrentInsulinVolume = SensorValue * 100/1019; /* percentage % */
    
      if(SensorValue < 30)
      {
        digitalWrite(buzzerAlarmPin, HIGH);
        stableFlow = false;
      }
      else
      {
        digitalWrite(buzzerAlarmPin, LOW);
        stableFlow = true;
      }
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Rate:");
      lcd.print(flowRate);
      lcd.print(" ml/hr");
//      delay(2000);
    //  lcd.setCursor(0,1);
    //  lcd.print("Rate:")
    //  lcd.print(flowRate);
    //  lcd.print(" ml/hr");  
      lcd.setCursor(0,2);
      lcd.print("syringe:");
      lcd.print(SensorValue);
//      lcd.print("%");
//      delay(2000);
      lcd.setCursor(0,3);
      
      lcd.print("stable flow...");
      delay(1000);
      int motorSpeed = (255/150.0)*flowRate*1000;
      analogWrite(motorPin,motorSpeed);
      }
  }
  else
  {
    lcd.clear();
    analogWrite(motorPin,0);
    digitalWrite(buzzerAlarmPin, LOW);
    lcd.setCursor(0,0);
    lcd.print("paused!");
  }
  }


}
