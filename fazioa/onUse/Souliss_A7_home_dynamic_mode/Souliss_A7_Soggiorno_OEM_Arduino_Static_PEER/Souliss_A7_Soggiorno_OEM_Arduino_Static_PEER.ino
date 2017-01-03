// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"

// Configure the framework
#include "bconf/StandardArduino.h"          // Use a standard Arduino
#include "conf/ethW5100.h"                  // Ethernet through Wiznet W5100

#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>
#include "DHT.h"
#include "topics.h"

#include "EmonLib.h"             // Include Emon Library
EnergyMonitor emon1;             // Create an instance

// Define the network configuration according to your router settings
uint8_t ip_address[4]  = {192, 168, 1, 105}; //IP GATEWAY

#define Gateway_address 0x69              // IP dec  105

// and the other on the wireless oneless one
#define peer_address  0x0010 //192.168.1.16 IP Fisso
#define myvNet_subnet 0xFF00

#define TEMPERATURE				0			// This is the memory slot used for the execution of the logic in network_address1
#define HUMIDITY				2			// This is the memory slot used for the execution of the logic
#define SLOT_Watt                               4
#define     SLOT_Voltage               8
#define     SLOT_Current               11

#define SLOT_REMOTE_CONTROLLER                  6
#define SLOT_APRIPORTA                          7

#define     PIN_DHT                     2
#define     PIN_OUTPUT_REMOTE_CONTROLLER     3
#define     PIN_OUTPUT_APRIPORTA             5

#define     PIN_VOLTAGE             15
#define     PIN_CURRENT             14

#define DEADBAND				0.01		// Deadband value 1%  
#define SIZE 2
float fPowerValues[SIZE];
uint8_t valByteArray[2];
float fVal;
uint16_t output16;
int i = 0;
float iMedia = 0;

void setup()
{
  // Serial.begin(9600);

  digitalWrite(PIN_OUTPUT_REMOTE_CONTROLLER, HIGH);
  digitalWrite(PIN_OUTPUT_APRIPORTA, HIGH);
  pinMode(PIN_OUTPUT_REMOTE_CONTROLLER, OUTPUT); //PIN FOR PULSE REMOTE CONTROLLER
  pinMode(PIN_OUTPUT_APRIPORTA, OUTPUT); //PIN FOR PULSE SLOT_APRIPORTA

  Initialize();
  // Set network parameters
  Souliss_SetAddress(peer_address, myvNet_subnet, Gateway_address);          // Address on the wireless interface

  Set_T57(SLOT_Watt);
  Set_Voltage(SLOT_Voltage); //T55
  Set_Current(SLOT_Current); //T56

  pinMode(PIN_VOLTAGE, INPUT);
  pinMode(PIN_CURRENT, INPUT);
  emon1.voltage(PIN_VOLTAGE, 221, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current(PIN_CURRENT, 5);       // Current: input pin, calibration.

  Set_PulseOutput(SLOT_REMOTE_CONTROLLER);
  Set_PulseOutput(SLOT_APRIPORTA);

}


void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_710ms() {
      Logic_PulseOutput(SLOT_REMOTE_CONTROLLER);
      LowDigOut(PIN_OUTPUT_REMOTE_CONTROLLER, Souliss_T1n_Coil, SLOT_REMOTE_CONTROLLER);
      Logic_PulseOutput(SLOT_APRIPORTA);
      LowDigOut(PIN_OUTPUT_APRIPORTA, Souliss_T1n_Coil, SLOT_APRIPORTA);
    }


    //acquisizione valori
    FAST_1110ms() {
      emon1.calcVI(30, 300);  //esegue il campionamento // Calculate all. No.of wavelengths, time-out
      fVal = emon1.realPower;

      iMedia = round(fVal);
      ImportAnalog(SLOT_Watt, &iMedia);
      Logic_Power(SLOT_Watt);

      //scelgo di fare pubblicare il valore direttamente dal nodo invece che dal GW
      float16(&output16, &iMedia);
      valByteArray[0] = C16TO8L(output16);
      valByteArray[1] = C16TO8H(output16);
      pblshdata(ENERGY_TOPIC, valByteArray, 2);

      fVal = emon1.Vrms;
      ImportAnalog(SLOT_Voltage, &fVal);
      Logic_Voltage(SLOT_Voltage);

      fVal = emon1.Irms;
      ImportAnalog(SLOT_Current, &fVal);
      Logic_Current(SLOT_Current);

    }

    // Process the communication
    FAST_PeerComms();
  }

  EXECUTESLOW() {
    UPDATESLOW();
  }
}
