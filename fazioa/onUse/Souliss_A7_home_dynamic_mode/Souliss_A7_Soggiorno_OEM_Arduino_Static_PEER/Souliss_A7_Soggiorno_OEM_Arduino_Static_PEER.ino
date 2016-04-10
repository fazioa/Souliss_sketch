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


// Define the network configuration according to your router settingsuration according to your router settings
#define  Gateway_address 0x6511        // The Gateway node has two address, one on the Ethernet side 69        // The Gateway node has two address, one on the Ethernet side
// and the other on the wireless oneless one
#define peer_address  0x0010
#define myvNet_subnet 0xFF00
#define myvNet_supern Gateway_address

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

#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define DEADBAND				0.01		// Deadband value 1%  

// Initialize DHT sensor for normal 8mhz Arduino
DHT dht(PIN_DHT, DHTTYPE, 2);

#define SIZE 6
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
  Souliss_SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  // Set the typical to use
  Set_T52(TEMPERATURE);
  Set_T53(HUMIDITY);
  Set_T57(SLOT_Watt);
  Set_Voltage(SLOT_Voltage); //T55
  Set_Current(SLOT_Current); //T56

  pinMode(PIN_VOLTAGE, INPUT);
  pinMode(PIN_CURRENT, INPUT);
  emon1.voltage(PIN_VOLTAGE, 221, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current(PIN_CURRENT, 5);       // Current: input pin, calibration.

  Set_PulseOutput(SLOT_REMOTE_CONTROLLER);
  Set_PulseOutput(SLOT_APRIPORTA);

  pinMode(PIN_DHT, INPUT);

  dht.begin();
}


void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_510ms() {
      // Compare the acquired input with the stored one, send the new value to the
      // user interface if the difference is greater than the deadband
      Logic_T52(TEMPERATURE);
      Logic_T53(HUMIDITY);
    }

    FAST_710ms() {
      Logic_PulseOutput(SLOT_REMOTE_CONTROLLER);
      LowDigOut(PIN_OUTPUT_REMOTE_CONTROLLER, Souliss_T1n_Coil, SLOT_REMOTE_CONTROLLER);
      Logic_PulseOutput(SLOT_APRIPORTA);
      LowDigOut(PIN_OUTPUT_APRIPORTA, Souliss_T1n_Coil, SLOT_APRIPORTA);
    }


    //acquisizione valori
    FAST_1110ms() {
      if (i < SIZE) {
        emon1.calcVI(20, 200);  //esegue il campionamento // Calculate all. No.of wavelengths, time-out
        fVal = emon1.realPower;
        fPowerValues[i++] = fVal;
      } else {
        //calcola media ed acquisisce il valore
        for (int j = 0; j < SIZE; j++) {
          iMedia += fPowerValues[j];
        }
        iMedia = round(iMedia / SIZE);
        ImportAnalog(SLOT_Watt, &iMedia);
        Logic_Power(SLOT_Watt);

        //scelgo di fare pubblicare il valore direttamente dal nodo invece che dal GW
        float16(&output16, &iMedia);
        valByteArray[0] = C16TO8L(output16);
        valByteArray[1] = C16TO8H(output16);
        publishdata(ENERGY_TOPIC, valByteArray, 2);

        i = 0;
        iMedia = 0;
      }
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
    SLOW_10s() {

      // Read temperature value from DHT sensor and convert from single-precision to half-precision
      float temperature = dht.readTemperature();
      ImportAnalog(TEMPERATURE, &temperature);
      // Read humidity value from DHT sensor and convert from single-precision to half-precision
      float humidity = dht.readHumidity();
      ImportAnalog(HUMIDITY, &humidity);
    }
  }
}
