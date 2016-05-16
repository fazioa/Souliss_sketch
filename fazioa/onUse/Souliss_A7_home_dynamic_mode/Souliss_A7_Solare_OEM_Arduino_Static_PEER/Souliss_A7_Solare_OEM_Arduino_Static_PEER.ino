// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"

// Configure the framework
#include "bconf/Chibiduino_v1.h"      // Use a Chibiduino 2.4 GHz wireless board
#include "Souliss.h"
#include "Typicals.h"
#include "topics.h"

#include <SPI.h>
#include "EmonLib.h"             // Include Emon Library
EnergyMonitor emon1;             // Create an instance

//**********************************************
//****************** PID ***********************
//**********************************************
#include "PID_v1_mod.h"
//Define Variables we'll be connecting to
double Setpoint, Input, Output, powerOutRate;
//Setpoint: è il bilanciamento tra produzione solare e consumo. Si può mettere a zero o poco più, per evitare l'immissione in rete
//Input: è il consumo di casa. E' già uguale alla differenza tra la produzione ed il fabbisogno dell'abitazione
//Output: è la spinta da dare ai pannelli solari. Es: Se il prelievo dalla rete è 500W allora i pannelli devono essere al massimo, per produrre di più ed abbassare la quantità di energia prelevata

//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint, 1, 0.01, 0.1, REVERSE);

//*********************************************************
//***********  DEFINE  ************************************
//*********************************************************
#define SETPOINT 30 //Watt

// Define the network configuration according to your router settingsuration according to your router settings
#define  Gateway_address 0x6511        // The Gateway node has two address, one on the Ethernet side 69        // The Gateway node has two address, one on the Ethernet side
// and the other on the wireless oneless one
#define peer_address  0x6514
#define myvNet_subnet 0xFF00
#define myvNet_supern Gateway_address

#define     SLOT_Watt                  0
#define     SLOT_Voltage               2
#define     SLOT_Current               4
#define     SLOT_RELE_GROUP_ONE_PERCENT   6
#define     SLOT_RELE_GROUP_TWO_PERCENT   8

#define     PIN_VOLTAGE             A1 //15
#define     PIN_CURRENT             A0 //14
#define     PIN_RELE_GROUP_1             9 //9 AND 10, THAT NOT cause the delay() and millis() functions to stop working
#define     PIN_RELE_GROUP_2             3 //in chibiduino avaiable only 3 and 9 for PWM 31Hz

//#define SIZE 2
//float fPowerValues[SIZE];
//int i = 0;
float fMedia = 0;

float fV = 0;
float fI = 0;

float fPannelliGruppo1_percento = 100;
float fPannelliGruppo2_percento = 100;

float iPWM_Val_1 = 255;
float iPWM_Val_2 = 255;


float fTopic_HomePower;
uint8_t mypayload_len = 0;
uint8_t mypayload[2];
int actual_SolarProduction,start_SolarProduction,iStart_HomePower;
void setup()
{
  // Init Serial
//  Serial.begin(9600);
//  Serial.println("POWER METER - VER.2 - Souliss");

  Initialize();
  // Set network parameters
  Souliss_SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  Set_Power(SLOT_Watt);
  Set_Voltage(SLOT_Voltage);
  Set_Current(SLOT_Current);
  Set_T51(SLOT_RELE_GROUP_ONE_PERCENT);
  Set_T51(SLOT_RELE_GROUP_TWO_PERCENT);

  pinMode(PIN_RELE_GROUP_1, OUTPUT);
  pinMode(PIN_RELE_GROUP_2, OUTPUT);
  setPwmFrequency(PIN_RELE_GROUP_1, 1024);
  setPwmFrequency(PIN_RELE_GROUP_2, 1024);

  pinMode(PIN_VOLTAGE, INPUT);
  pinMode(PIN_CURRENT, INPUT);
  emon1.voltage(PIN_VOLTAGE, 204, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current(PIN_CURRENT, 22);       // Current: input pin, calibration.


  //**********************************************
  //****************** PID ***********************
  //**********************************************
  Setpoint = SETPOINT; //Il setpoint è il bilanciamento tra produzione solare e consumo
  //turn the PID on
  myPID.SetMode(AUTOMATIC);
}
float fVal;

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    //acquisizione valori
    SHIFT_1110ms(0) {
     // if (i < SIZE) {
        emon1.calcVI(20, 200);  //esegue il campionamento // Calculate all. No.of wavelengths, time-out
        fVal = emon1.realPower;
        fV = emon1.Vrms;
        fV  = emon1.Irms;

    //    fPowerValues[i++] = fVal;
   //   } else {
        //calcola media ed acquisisce il valore
  //      for (int j = 0; j < SIZE; j++) {
 //         fMedia += fPowerValues[j];
  //      }
 //       fMedia = round(fMedia / SIZE);
        fMedia = round(fVal);

        //se il consumo rilevato è <1 (oppure anche inferiore a zero) allora viene posto a zero, e di conseguenza non dovrebbero esserci aggiornamenti sul bus
        if (fMedia < 1) {
          fMedia = 0;
          fV = 0;
          fI = 0;
        }

        ImportAnalog(SLOT_Watt, &fMedia);
        Logic_Power(SLOT_Watt);

        ImportAnalog(SLOT_Voltage, &fV);
        Logic_Voltage(SLOT_Voltage);
        //        Serial.print("emon: "); Serial.print(i); Serial.print(" - ");
        //        Serial.print(fV); LOG.print(" VOLTAGE - ");
        ImportAnalog(SLOT_Current, &fI);
        Logic_Current(SLOT_Current);
        //        Serial.print(fI); LOG.println(" CURRENT");
      //  i = 0;
        fMedia = 0;

        ImportAnalog(SLOT_RELE_GROUP_ONE_PERCENT, &fPannelliGruppo1_percento);
        Read_T51(SLOT_RELE_GROUP_ONE_PERCENT);

        ImportAnalog(SLOT_RELE_GROUP_TWO_PERCENT, &fPannelliGruppo2_percento);
        Read_T51(SLOT_RELE_GROUP_TWO_PERCENT);

  //    }
      actual_SolarProduction = fMedia;
    }

    SHIFT_1110ms(1) {
      subscribeTopics();
    }

    SHIFT_1110ms(2) {
      //Livellamento
      //Il valore dell'input è determinato tenendo conto del valore del consumo di casa ricevuto via rete Souliss l'ultima volta
      //l'indice è aggiornato in continuazione anche sulla base della produzione solare
      //uso questo metodo perchè i valori trasmessi con publish&subscribe non sembre hanno la costanza di 1 secondo. Spesso mancano per 3 o 4 secondi
      //e ciò causa l'intervento incisivo di adattamenti PID non desiderati, dovuti alla mancanza di feedback
      Input = iStart_HomePower - actual_SolarProduction + start_SolarProduction;

      //qui gestisco
      // fPannelliGruppo1_percento
      // fPannelliGruppo2_percento
      myPID.Compute();
      //a questo punto in Output ho il valore, compreso tra 0-254 con il quale regolare la produzione
      //la scala viene ampliata 0-511. Il primo PWM ha priorità e va presto a 255, il secondo solo quando serve, ed è il primo a cui viene tolta potenza. In questo modo è più probabile che venga modulato in PWM soltanto una parte dei relè, e non tutti.
//      Serial.print("Input: "); Serial.println(Input);
//      Serial.print("Output: "); Serial.println(Output);
      powerOutRate = (Output / 255) * 511; //trasforma la scala a 256*2 valori
//      Serial.print("Output ampliato: "); Serial.println(powerOutRate);
      iPWM_Val_1 = powerOutRate;
      if (iPWM_Val_1 > 255) iPWM_Val_1 = 255; //il massimo valore puà essere 255
      fPannelliGruppo1_percento = (int) ((iPWM_Val_1 / 255) * 100); //trasformo il valore in percentuale per passarlo al tipico di Souliss. E' più semplice rappresentare il dato in percentuale

      iPWM_Val_2 = powerOutRate - 256;
      if (iPWM_Val_2 < 0) iPWM_Val_2 = 0; //il minimo valore può essere 0
      fPannelliGruppo2_percento = (int) ((iPWM_Val_2 / 255) * 100);

//      Serial.print("fPannelliGruppo1_percento: "); Serial.println(fPannelliGruppo1_percento);
//      Serial.print("fPannelliGruppo2_percento: "); Serial.println(fPannelliGruppo2_percento);


    }

    FAST_110ms() {
      //qui devo inviare il valore PWM al pin
      analogWrite(PIN_RELE_GROUP_1, iPWM_Val_1);
      analogWrite(PIN_RELE_GROUP_2, iPWM_Val_2);
    }

    // Process the communication
    FAST_PeerComms();
  }

  EXECUTESLOW() {
    UPDATESLOW();
    SLOW_10s() {
    }
  }
}

void subscribeTopics() {
  if (subscribedata(ENERGY_TOPIC, mypayload, &mypayload_len)) {
    float32((uint16_t*) mypayload,  &fTopic_HomePower);
//    Serial.print("ENERGY_TOPIC: "); Serial.println(fTopic_HomePower);

    //Se esiste comunicazione del consumo di casa allora aggiorno le variabili usate per la rideterminazione dei rate dei gruppi di pannelli, che viene eseguito ogni secondo
    //tengo traccia della produzione solare e del consumo attuale al momento dell'acquisizione del consumo di casa (che è già al netto della produzione solare)
    iStart_HomePower = fTopic_HomePower;
    //start_SolarProduction impostata sopra
    start_SolarProduction = actual_SolarProduction;
  }
}


/**
   NOTE ABOUT PWM
   Divides a given PWM pin frequency by a divisor.

   The resulting frequency is equal to the base frequency divided by
   the given divisor:
     - Base frequencies:
        o The base frequency for pins 3, 9, 10, and 11 is 31250 Hz.
        o The base frequency for pins 5 and 6 is 62500 Hz.
     - Divisors:
        o The divisors available on pins 5, 6, 9 and 10 are: 1, 8, 64,
          256, and 1024.
        o The divisors available on pins 3 and 11 are: 1, 8, 32, 64,
          128, 256, and 1024.

   PWM frequencies are tied together in pairs of pins. If one in a
   pair is changed, the other is also changed to match:
     - Pins 5 and 6 are paired on timer0
     - Pins 9 and 10 are paired on timer1
     - Pins 3 and 11 are paired on timer2

   Note that this function will have side effects on anything else
   that uses timers:
     - Changes on pins 3, 5, 6, or 11 may cause the delay() and
       millis() functions to stop working. Other timing-related
       functions may also be affected.
     - Changes on pins 9 or 10 will cause the Servo library to function
       incorrectly.

   Thanks to macegr of the Arduino forums for his documentation of the
   PWM frequency divisors. His post can be viewed at:
     http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1235060559/0#4
*/
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if (pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch (divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if (pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if (pin == 3 || pin == 11) {
    switch (divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}
