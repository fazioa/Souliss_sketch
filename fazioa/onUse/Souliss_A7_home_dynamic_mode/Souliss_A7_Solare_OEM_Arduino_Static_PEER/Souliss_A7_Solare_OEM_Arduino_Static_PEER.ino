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


// Define the network configuration according to your router settingsuration according to your router settings
#define  Gateway_address 0x6511        // The Gateway node has two address, one on the Ethernet side 69        // The Gateway node has two address, one on the Ethernet side
// and the other on the wireless oneless one
#define peer_address  0x6514
#define myvNet_subnet 0xFF00
#define myvNet_supern Gateway_address

#define     SLOT_Watt                  0
#define     SLOT_Voltage               2
#define     SLOT_Current               4


#define     PIN_VOLTAGE             A1 //15
#define     PIN_CURRENT             A0 //14
#define     PIN_RELE_GROUP_1             9 //9 AND 10, THAT NOT cause the delay() and millis() functions to stop working
#define     PIN_RELE_GROUP_2             10 

#define SIZE 6
float fPowerValues[SIZE];
int i = 0;
float fMedia = 0;
int iProduzioneSolare = 0;
int iLimiteInferiorePrelievoGrid = 30;
int iDeadBandPrelievoGrid = 20;
int iWattSogliaCUT = 300;

float fV = 0;
float fI = 0;

float fPannelliGruppo1_percento = 100;
float fPannelliGruppo2_percento = 100;
float fPrelievoGrid = 0;
float fStep_percento = 5;

int iPWM_Val_1 = 255;
int iPWM_Val_2 = 255;

float fTopic_HomePower;
uint8_t mypayload_len = 0;
uint8_t mypayload[2];



void setup()
{
  // Init Serial
  Serial.begin(9600);
  Serial.println("POWER METER - VER.2 - Souliss");
  
  Initialize();
  // Set network parameters
  Souliss_SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  Set_Power(SLOT_Watt);
  Set_Voltage(SLOT_Voltage);
  Set_Current(SLOT_Current);

   pinMode(PIN_RELE_GROUP_1, OUTPUT);
   pinMode(PIN_RELE_GROUP_2, OUTPUT);
   setPwmFrequency(PIN_RELE_GROUP_1, 1024);
   setPwmFrequency(PIN_RELE_GROUP_2, 1024);

  pinMode(PIN_VOLTAGE, INPUT);
  pinMode(PIN_CURRENT, INPUT);
  emon1.voltage(PIN_VOLTAGE, 204, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current(PIN_CURRENT, 22);       // Current: input pin, calibration.
}

float fVal;

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    //acquisizione valori
    SHIFT_1110ms(0) {
      if (i < SIZE) {
        emon1.calcVI(20, 200);  //esegue il campionamento // Calculate all. No.of wavelengths, time-out
        fVal = emon1.realPower;
        float fV = emon1.Vrms;
        float fI  = emon1.Irms;

        fPowerValues[i++] = fVal;
      } else {
        //calcola media ed acquisisce il valore
        for (int j = 0; j < SIZE; j++) {
          fMedia += fPowerValues[j];
        }
        fMedia = round(fMedia / SIZE);
        ImportAnalog(SLOT_Watt, &fMedia);

        //se il consumo rilevato è <1 (oppure anche inferiore a zero) allora viene posto a zero, e di conseguenza non dovrebbero esserci aggiornamenti sul bus
        if (fMedia < 1) {
          fMedia = 0;
        }

        Logic_Power(SLOT_Watt);

        ImportAnalog(SLOT_Voltage, &fV);
        Logic_Voltage(SLOT_Voltage);
        //        Serial.print("emon: "); Serial.print(i); Serial.print(" - ");
        //        Serial.print(fV); LOG.print(" VOLTAGE - ");
        ImportAnalog(SLOT_Current, &fI);
        Logic_Current(SLOT_Current);
        //        Serial.print(fI); LOG.println(" CURRENT");


        i = 0;
        fMedia = 0;
      }
    }

    SHIFT_1110ms(1) {
      subscribeTopics();
    }

    SHIFT_1110ms(2) {
      //Livellamento
      iProduzioneSolare = fMedia;
      fPrelievoGrid = fTopic_HomePower;

      //qui gestisco
      // fPannelliGruppo1_percento
      // fPannelliGruppo2_percento

      //quin forse conviene calcolare il differenziale ed adeguare lo step

      if (fPrelievoGrid <= iLimiteInferiorePrelievoGrid) {
        //diminuisco la produzione
        if (fPannelliGruppo2_percento > 0) {
          //se il primo gruppo produce allora diminuisco quello
          gruppoPannelli_removePower(fPannelliGruppo2_percento, 5);
        } else {
          //altrimenti, se il primo gruppo è già a zero, diminuisco il primo gruppo
          gruppoPannelli_removePower(fPannelliGruppo1_percento, 5);
        }

      } else if (fPrelievoGrid > iLimiteInferiorePrelievoGrid + iDeadBandPrelievoGrid) {
        //aumento la produzione
        if (fPannelliGruppo1_percento >= 100) {
          //se il primo gruppo produce allora diminuisco quello
          gruppoPannelli_addPower(fPannelliGruppo2_percento, fStep_percento);
        } else {
          //altrimenti, se il primo gruppo è già a zero, diminuisco il primo gruppo
          gruppoPannelli_addPower(fPannelliGruppo1_percento, fStep_percento);
        }

      }
    }

    FAST_110ms() {
      //CUT
      //Se viene rilevata una produzione troppo elevata rispetto al bisogno, allora interrompo immediatamento la produzione
      //anche qui forse conviene calcolare il differenziale ed adeguare lo step
      

      //qui devo inviare il valore PWM al pin
      iPWM_Val_1 = (int) fPannelliGruppo1_percento / 100 * 254;
      iPWM_Val_2 = (int) fPannelliGruppo2_percento / 100 * 254;
      analogWrite(PIN_RELE_GROUP_1,iPWM_Val_1);
      analogWrite(PIN_RELE_GROUP_2,iPWM_Val_2);
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
    Serial.print("ENERGY_TOPIC: "); Serial.println(fTopic_HomePower);
  }
}

int gruppoPannelli_addPower(float iValoreGruppo, float iPercento) {
  iValoreGruppo += iValoreGruppo * iPercento / 100;
  if (iValoreGruppo <= 0) {
    iValoreGruppo = 0;
  }
  return  iValoreGruppo;
}

int gruppoPannelli_removePower(float iValoreGruppo, float iPercento) {
  iValoreGruppo -= iValoreGruppo * iPercento / 100;
  if (iValoreGruppo <= 0) {
    iValoreGruppo = 0;
  }
  return  iValoreGruppo;
}


//N OTE ABOUT PWM
/**
 * Divides a given PWM pin frequency by a divisor.
 * 
 * The resulting frequency is equal to the base frequency divided by
 * the given divisor:
 *   - Base frequencies:
 *      o The base frequency for pins 3, 9, 10, and 11 is 31250 Hz.
 *      o The base frequency for pins 5 and 6 is 62500 Hz.
 *   - Divisors:
 *      o The divisors available on pins 5, 6, 9 and 10 are: 1, 8, 64,
 *        256, and 1024.
 *      o The divisors available on pins 3 and 11 are: 1, 8, 32, 64,
 *        128, 256, and 1024.
 * 
 * PWM frequencies are tied together in pairs of pins. If one in a
 * pair is changed, the other is also changed to match:
 *   - Pins 5 and 6 are paired on timer0
 *   - Pins 9 and 10 are paired on timer1
 *   - Pins 3 and 11 are paired on timer2
 * 
 * Note that this function will have side effects on anything else
 * that uses timers:
 *   - Changes on pins 3, 5, 6, or 11 may cause the delay() and
 *     millis() functions to stop working. Other timing-related
 *     functions may also be affected.
 *   - Changes on pins 9 or 10 will cause the Servo library to function
 *     incorrectly.
 * 
 * Thanks to macegr of the Arduino forums for his documentation of the
 * PWM frequency divisors. His post can be viewed at:
 *   http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1235060559/0#4
 */
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
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

