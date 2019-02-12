/*
 This is a sample sketch to show how to use the OneButtonLibrary
 to detect double-click events on a button. 
 The library internals are explained at
 http://www.mathertel.de/Arduino/OneButtonLibrary.aspx
 
 Setup a test circuit:
 * Connect a pushbutton to pin A1 (ButtonPin) and ground.
 * The pin 13 (StatusPin) is used for output attach a led and resistor to ground
 or see the built-in led on the standard arduino board.
 
 The Sketch shows how to setup the library and bind a special function to the doubleclick event.
 In the loop function the button.tick function has to be called as often as you like.
 */

// 03.03.2011 created by Matthias Hertel
// 01.12.2011 extension changed to work with the Arduino 1.0 environment
// 02.11.2012 Modified by Tonino Fazio. Added support to triple click
//------------------------
// Sketch modificato da Tonino Fazio (www.xorse.it). La modifica consiste nell'introduzione del supporto al triplo click. 
//------------------------

#include "OneButton.h"

const int DOWNPin = 3;
const int UPPin = 4;
const int UPPinButton = A1;
const int DOWNPinButton = A2;

boolean pressed =false;
boolean doubleClickFlag =false;

unsigned long _clickTicksDoubleClick=5000; //millisecondi
unsigned long _clickTicksToSTOP=40000;  //millisecondi

unsigned long _startTimeDoubleClick; // current (relative) time in msecs.
unsigned long _startTimeF;
//0:STOP, 1: UP, 2: DOWN
const int STOP = 0;
const int UP = 1;
const int DOWN = 2;

int movimentoTenda=STOP;
//================================
// Setup a new OneButton on pin A1.  
OneButton buttonUP(UPPinButton, true);
OneButton buttonDOWN(DOWNPinButton, true);


// setup code here, to run once:
void setup() {
  // enable the standard led on pin 13.
  Serial.begin(9600);
  pinMode(DOWNPin, OUTPUT);
  pinMode(UPPin, OUTPUT);

  // link the doubleclick function to be called on a doubleclick event.   
  buttonUP.attachDoubleClick(doubleclickUP);
  buttonDOWN.attachDoubleClick(doubleclickDOWN);
  buttonUP.attachTripleClick(tripleclickUP);
  buttonDOWN.attachTripleClick(tripleclickDOWN);

  buttonDOWN.attachClick(click);
  buttonUP.attachClick(click);
  buttonDOWN.attachPress(pressDOWN);
  buttonUP.attachPress(pressUP);

} // setup


// main code here, to run repeatedly: 
void loop() {
  // keep watching the push button:
  buttonUP.tick();
  buttonDOWN.tick();

 //tempo attuale
 unsigned long _now = millis(); // current (relative) time in msecs.
  //se è attiva la funziona press (singola pressione. No doppio o triplo click o pressione prolungata) allora controllo lo stato dei pulsanti. Se nessun pulsante è premuto allora STOP
    if (pressed) {
    if ( leggeStatoPressioneTasti()==STOP) {
      movimentoTenda=STOP;
    }} 
    //in ogni caso STOP se supero un tempo limite: _clickTicksToSTOP
    else if (_now > _startTimeF + _clickTicksToSTOP) {
           movimentoTenda=STOP;
    }  
  
  //in caso di doppio click
  if (doubleClickFlag) {
    //calcolo il tempo trascorso dall'inizio del doppio click. Fermo il movimento dopo un tempo impostato con costante, sopra
    if (_now > _startTimeDoubleClick + _clickTicksDoubleClick) {
        //passato il tempo impostato (_clickTicksDoubleClick) STOP
      movimentoTenda=STOP ;
    }
  }

  //movimentoTenda -> 0:STOP, 1: UP, 2: DOWN
  switch (movimentoTenda) {
  case STOP:    // Tenda Ferma
    digitalWrite(UPPin, LOW);
    digitalWrite(DOWNPin, LOW);
    pressed=false;
    doubleClickFlag=false;
    _startTimeDoubleClick=0;
    _startTimeF=0;
    break;
  case UP:    
     digitalWrite(UPPin, HIGH);
    digitalWrite(DOWNPin, LOW);
    break;
  case DOWN:    
      digitalWrite(UPPin, LOW);
    digitalWrite(DOWNPin, HIGH);
    break;
  } 
  // You can implement other code in here or just wait a while 
  delay(10);
} // loop

// this function will be called when the button was pressed 2 times in a short timeframe.
void doubleclickUP() {
  Serial.println("Doppio click UP" );
  movimentoTenda=UP;
  doubleClickFlag=true;
  //fissa il tempo di inizio del doppio click
  _startTimeDoubleClick=millis();
  _startTimeF=millis();
} // doubleclick

void doubleclickDOWN() {
  Serial.println("Doppio click DOWN" );
  movimentoTenda=DOWN;
  doubleClickFlag=true;
  //fissa il tempo di inizio del doppio click
  _startTimeDoubleClick=millis();
  _startTimeF=millis();
} // doubleclick

//========================= T R I P L O   C L I C K ============================================
// this function will be called when the button was pressed 3 times in a short timeframe.
void tripleclickUP() {
  Serial.println("Triplo click UP" );
  movimentoTenda=UP;
  doubleClickFlag=false;
  _startTimeF=millis();
} // tripleclick

void tripleclickDOWN() {
  Serial.println("Triplo click DOWN" );
  movimentoTenda=DOWN;
  doubleClickFlag=false;
  _startTimeF=millis();
} // tripleclick
//===================================================================================


void click() {
  movimentoTenda= STOP;
  doubleClickFlag=false;
  _startTimeF=millis();
} // click

void pressUP() {
  movimentoTenda= UP;
  pressed=true;
} // press
void pressDOWN() {
  movimentoTenda= DOWN;
  pressed=true;
} // press


int leggeStatoPressioneTasti() {
  //legge lo stato di entrambi i pulsanti
  int readingUPPinButton = digitalRead(UPPinButton);
  int readingDOWNPinButton = digitalRead(DOWNPinButton);
  //se il pulsante UP è stato premuto pone il movimento a UP, altrimenti se è stato premuto DOWN lo pone a DOWN, altrimenti STOP
  if (readingUPPinButton ==LOW) {
    return UP;
  } 
  else if (readingDOWNPinButton==LOW) {
    return DOWN;
  }
  return STOP;
} // leggeStatoPressioneTasti
// End










