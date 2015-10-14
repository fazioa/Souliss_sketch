#include "constants.h"
#include "encoder.h"
#include <MenuSystem.h>
#include "menu.h"
float encoderValue_prec = 0;
#include <Arduino.h>


MenuSystem myMenu;
void setup() {
  Serial.begin(9600);
  //ENCODER
  /////////////////////////////////////////////////////////////////////////////////////////////////////////
  pinMode (ENCODER_PIN_A, INPUT_PULLUP);
  pinMode (ENCODER_PIN_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), encoder, CHANGE);

  pinMode(ENCODER_SWITCH, INPUT);
  digitalWrite(ENCODER_SWITCH, HIGH);
  encoderValue_prec = getEncoderValue();

  //setEncoderValue(setpoint);
  //setpoint = getEncoderValue();
  //MENU
  /////////////////////////////////////////////////////////////////////////////////////////////////////////
  initMenu();
  myMenu = getMenu();
 
}

void loop() {
  delay(250);
  //SWITCH ENCODER
  if (!digitalRead(ENCODER_SWITCH)) {
    Serial.println("pulsante premuto");
    myMenu.select(false);
    printMenu(myMenu);
    Serial.println(); Serial.println();
  }

  if (getEncoderValue() > encoderValue_prec) {
    encoderValue_prec = getEncoderValue();
    myMenu.prev();
    printMenu(myMenu);
    Serial.println(); Serial.println();
  } else if (getEncoderValue() < encoderValue_prec) {
    encoderValue_prec = getEncoderValue();
    myMenu.next();
    printMenu(myMenu);
    Serial.println(); Serial.println();
  }


}

