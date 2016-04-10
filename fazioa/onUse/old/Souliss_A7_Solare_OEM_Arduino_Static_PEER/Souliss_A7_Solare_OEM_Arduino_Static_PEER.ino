// Configure the framework
#include "bconf/Chibiduino_v1.h"      // Use a Chibiduino 2.4 GHz wireless board
#include "Souliss.h"
#include "Typicals.h"
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

#define SIZE 6
float fPowerValues[SIZE];
int i = 0;
float iMedia = 0;
void setup()
{
  // Init Serial
  Serial.begin(9600);
  Serial.println("POWER METER - VER.1 - Souliss");
  Initialize();
  // Set network parameters
  Souliss_SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  Set_Power(SLOT_Watt);
  Set_Voltage(SLOT_Voltage);
  Set_Current(SLOT_Current);

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
    FAST_1110ms() {
      if (i < SIZE) {
        emon1.calcVI(20, 200);  //esegue il campionamento // Calculate all. No.of wavelengths, time-out
        fVal = emon1.realPower;
        float fV = emon1.Vrms;
        float fI  = emon1.Irms;

        fPowerValues[i++] = fVal;
        ImportAnalog(SLOT_Voltage, &fV);
        Logic_Voltage(SLOT_Voltage);
        Serial.print("emon: "); Serial.print(i); Serial.print(" - ");
        Serial.print(fV); LOG.print(" VOLTAGE - ");


        ImportAnalog(SLOT_Current, &fI);
        Logic_Current(SLOT_Current);

        Serial.print(fI); LOG.println(" CURRENT");

      } else {
        //calcola media ed acquisisce il valore
        for (int j = 0; j < SIZE; j++) {
          iMedia += fPowerValues[j];
        }
        iMedia = round(iMedia / SIZE);
        ImportAnalog(SLOT_Watt, &iMedia);
        Logic_T57(SLOT_Watt);
        i = 0;
        iMedia = 0;
      }
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
