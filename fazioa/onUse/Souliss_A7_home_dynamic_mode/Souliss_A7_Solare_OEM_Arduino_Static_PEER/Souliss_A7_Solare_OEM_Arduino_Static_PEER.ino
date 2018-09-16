//  COMPILA CON
//Scheda: Arduino Pro or Pro Mini
//Processore: ATmega 328P (3.3V 8MHz)

// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"

// Configure the framework
#include "bconf/Chibiduino_v1.h"      // Use a Chibiduino 2.4 GHz wireless board
#include "Souliss.h"
#include "Typicals.h"
#include "topics.h"

#include <SPI.h>
#include <OneWire.h>
#include "DallasTemperature.h"
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
//double consKp=0.5, consKi=0.5, consKd=0;  OLD

//Define the aggressive and conservative Tuning Parameters
//double aggKp = 0.5, aggKi = 0.2, aggKd = 0.1;
double consKp = 0, consKi = 0.25, consKd = 0;

PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, REVERSE);

//*********************************************************
//***********  DEFINE  ************************************
//*********************************************************
#define SETPOINT 10 //Watt
//#define W_PASSAGGIO_AGGRESSIVE_TUNING 150 //Watt

#define PIN_ONEWIRE_SENSORS 5

float temperature_rele_1;
float temperature_rele_2;

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
#define SLOT_TEMPERATURE_ONE        10     // This is the memory slot used for the execution of the logic in network_address1
#define SLOT_TEMPERATURE_TWO     12


#define     PIN_VOLTAGE             A1 //15
#define     PIN_CURRENT             A0 //14
#define     PIN_RELE_GROUP_1             9 //9 AND 10, THAT NOT cause the delay() and millis() functions to stop working
#define     PIN_RELE_GROUP_2             3 //in chibiduino avaiable only 3 and 9 for PWM 31Hz

float fV = 0;
float fI = 0;

float fPannelliGruppo1_percento = 100;
float fPannelliGruppo2_percento = 100;

float iPWM_Val_1 = 255;
float iPWM_Val_2 = 255;


float fTopic_HomePower;
uint8_t mypayload_len = 0;
uint8_t mypayload[2];
int actual_SolarProduction, start_SolarProduction, iStart_HomePower;

// Initialize sensors
OneWire oneWire(PIN_ONEWIRE_SENSORS);
DallasTemperature sensors(&oneWire);

void setup()
{
  // Init Serial
  Serial.begin(9600);
  Serial.println("POWER METER - VER.3 - Souliss");

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

  Set_Temperature(SLOT_TEMPERATURE_ONE);
  Set_Temperature(SLOT_TEMPERATURE_TWO);
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12.

}
float fVal;

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();


    FAST_210ms() {
      //acquisizione valori
      //********************************
      //********************************
      emon1.calcVI(20, 200);  //esegue il campionamento // Calculate all. No.of wavelengths, time-out
      fVal = emon1.realPower;
      fV = emon1.Vrms;
      fI  = emon1.Irms;

      //se la produzione rilevata è <5 (oppure anche inferiore a zero) allora viene posto a zero, e di conseguenza non dovrebbero esserci aggiornamenti sul bus
      fVal = round(fVal);
      if (fVal <= 5) {
        fVal = 0;
        fI = 0;
      }

      actual_SolarProduction = fVal;
      //********************************
      //********************************


      //acquisizione comsuno casa
      //********************************
      //********************************
      subscribeTopics();
      //********************************
      //********************************


      //Livellamento
      //********************************
      //********************************
      //Il valore dell'input è determinato tenendo conto del valore del consumo di casa ricevuto via rete Souliss
      //l'indice è aggiornato in continuazione anche sulla base della produzione solare
      //uso questo metodo perchè i valori ricevuti con publish&subscribe non sempre vengono trasmessi con costanza. Spesso mancano per 3 o 4 secondi
      //e ciò causa l'intervento incisivo di adattamenti PID non desiderati dovuti alla mancanza di feedback
      Input = iStart_HomePower - actual_SolarProduction + start_SolarProduction;

      // Stabilisce i parametri del PID, conservativi o aggressivi
      double gap = Input - Setpoint; //distance away from setpoint
      //se avviene immissione di corrente in rete allora vengono impostati dei parametri del PID leggermente più aggressivi
      //  if (gap < W_PASSAGGIO_AGGRESSIVE_TUNING) //se la potenza immessa supera i 50W allora il PID viene impostato in modalità più aggressiva
      //  {
      //we're far from setpoint, use aggressive tuning parameters
      //    myPID.SetTunings(aggKp, aggKi, aggKd);
      //    }
      //    else
      //     {
      //we're close to setpoint, use conservative tuning parameters
      myPID.SetTunings(consKp, consKi, consKd);
      //     }


      //qui gestisco
      // fPannelliGruppo1_percento
      // fPannelliGruppo2_percento

      myPID.Compute(); // By default this range is 0-255: the arduino PWM range.
      //a questo punto in Output ho il valore, compreso tra 0-254 con il quale regolare la produzione
      //la scala viene ampliata 0-511. Il primo PWM ha priorità e va presto a 255, il secondo solo quando serve, ed è il primo a cui viene tolta potenza. In questo modo è più probabile che venga modulato in PWM soltanto una parte dei relè, e non tutti.
      //      Serial.print("Input: "); Serial.println(Input);
      //      Serial.print("Output: "); Serial.println(Output);
      //********************************
      //********************************
      //usndo il codice seguente i due relè vengono comandati in modo graduale. Se è necessario diminuire la produzione ed il secondo relè è già a 0% allora comincio a diminuire anche il primo relè
      /*

        powerOutRate = (Output / 255) * 511; //trasforma la scala a 256*2 valori
        //      Serial.print("Output ampliato: "); Serial.println(powerOutRate);
        iPWM_Val_1 = powerOutRate;
        if (iPWM_Val_1 > 255) iPWM_Val_1 = 255; //il massimo valore può essere 255
        fPannelliGruppo1_percento = (int) ((iPWM_Val_1 / 255) * 100); //trasformo il valore in percentuale per passarlo al tipico di Souliss. E' più semplice rappresentare il dato in percentuale

        iPWM_Val_2 = powerOutRate - 256;
        if (iPWM_Val_2 < 0) iPWM_Val_2 = 0; //il minimo valore può essere 0
        fPannelliGruppo2_percento = (int) ((iPWM_Val_2 / 255) * 100);
      */

      //ho notato che uno dei due relè viene sempre caricato di più lavoro rispetto all'altro.
      //non noto peggioramenti nel funzionamento facendo lavorare i due relè in modo simmetrico
      iPWM_Val_1 = Output ;
      iPWM_Val_2 = Output ;

      //invio valore PWM al pin
      //********************************
      //********************************
      //se la temperatura supera i 55 gradi la percentuale viene abbassata
      if (temperature_rele_1 > 65) {
        iPWM_Val_1 /= 3;
      } else if (temperature_rele_1 > 55){
         iPWM_Val_1 /= 2;
      }
       if (temperature_rele_2 > 65) {
        iPWM_Val_2 /= 3;
      } else if (temperature_rele_2 > 55){
         iPWM_Val_2 /= 2;
      }
     

      analogWrite(PIN_RELE_GROUP_1, iPWM_Val_1);
      analogWrite(PIN_RELE_GROUP_2, iPWM_Val_2);
      fPannelliGruppo1_percento = (int) ((iPWM_Val_1 / 255) * 100); //trasformo il valore in percentuale per passarlo al tipico di Souliss. E' più semplice rappresentare il dato in percentuale
      fPannelliGruppo2_percento = (int) ((iPWM_Val_2 / 255) * 100);
      
      //      Serial.print("iPWM_Val_1: "); Serial.println(iPWM_Val_1);
      //      Serial.print("iPWM_Val_2: "); Serial.println(iPWM_Val_2);
      //      Serial.print("fPannelliGruppo1_percento: "); Serial.println(fPannelliGruppo1_percento);
      //      Serial.print("fPannelliGruppo2_percento: "); Serial.println(fPannelliGruppo2_percento);

    }




    FAST_1110ms() {
      //invio valori a Souliss
      //********************************
      //********************************
      ImportAnalog(SLOT_Watt, &fVal);
      //Logic_Power(SLOT_Watt);
      Souliss_Logic_T57(memory_map, SLOT_Watt, 5, &data_changed);


      ImportAnalog(SLOT_Voltage, &fV);
      //Logic_Voltage(SLOT_Voltage);
      Souliss_Logic_T55(memory_map, SLOT_Voltage, 1, &data_changed);

      ImportAnalog(SLOT_Current, &fI);
      // Logic_Current(SLOT_Current);
      Souliss_Logic_T56(memory_map, SLOT_Current, 0.3, &data_changed);


      ImportAnalog(SLOT_RELE_GROUP_ONE_PERCENT, &fPannelliGruppo1_percento);
      Read_T51(SLOT_RELE_GROUP_ONE_PERCENT);

      ImportAnalog(SLOT_RELE_GROUP_TWO_PERCENT, &fPannelliGruppo2_percento);
      Read_T51(SLOT_RELE_GROUP_TWO_PERCENT);

    }

    SHIFT_11110ms(0) {
      sensors.requestTemperatures(); // Send the command to get temperatures

      temperature_rele_1 = sensors.getTempCByIndex(0);
      Serial.print("Temp 1: ");
      Serial.println(temperature_rele_1);
      ImportAnalog(SLOT_TEMPERATURE_ONE, &temperature_rele_1);
      Logic_Temperature(SLOT_TEMPERATURE_ONE);
    }

    SHIFT_11110ms(100) {
      temperature_rele_2 = sensors.getTempCByIndex(1);
      Serial.print("Temp 2: ");
      Serial.println(temperature_rele_2);
      ImportAnalog(SLOT_TEMPERATURE_TWO, &temperature_rele_2);
      Logic_Temperature(SLOT_TEMPERATURE_TWO);
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
  if (sbscrbdata(ENERGY_TOPIC, mypayload, &mypayload_len)) {
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
