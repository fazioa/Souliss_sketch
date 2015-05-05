/**************************************************************************
	Souliss - Thermostat

	Control the heating or cooling of using a temperature measure and a
	setpoint, based on the percentage distance from the setpoint runs the
	fans.

	The temperature measure is acquired throught Souliss and available at the
	user interfaces as half-precision floating point. All calculation shall be
	performed as standard floating point and at the end converted in half
	precision using :
		void Souliss_AnalogIn(U8 pin, U8 *memory_map, U8 slot,
								float scaling, float bias);

	Is suggested a scaling to the vRef of the AVR's ADC, using an external
	amplification circuit, in order to use the whole resolution.

	The Device has
		- An analog sensor or device on PIN A0

***************************************************************************/
#include "bconf/Chibiduino_v1.h"			// Use a Chibiduino 2.4 GHz wireless board

#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>
#include <OneWire.h>
#include "DallasTemperature.h"

#define SlotThermostat				0			// This is the memory slot used for the execution of the logic in network_address1
#define DSPIN       14

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(DSPIN);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

float temperature;

// Define the network configuration according to your router settingsuration according to your router settings
#define	Gateway_address	0x6501				// The Gateway node has two address, one on the Ethernet side69				// The Gateway node has two address, one on the Ethernet side
											// and the other on the wireless oneless one
#define	Peer_address	0x6502
#define	myvNet_subnet	0xFF00
#define	myvNet_supern	Gateway_address

void setup()
{
  Serial.begin(9600);

  Initialize();

  // Setup the network configuration
  Souliss_SetAddress(Peer_address, myvNet_subnet, myvNet_supern);					// Address on the wireless interface


  // Set the typical to use in slot 0
  Set_Thermostat(SlotThermostat);

  // Define output pins
  pinMode(2, OUTPUT);		// Heater
  pinMode(3, OUTPUT);		// Fan 1
  pinMode(4, OUTPUT);		// Fan 2
  pinMode(5, OUTPUT);		// Fan 3

  // Start up the library
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12.
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    // Process every 510ms the logic that control the curtain
    FAST_510ms() {


      Logic_Thermostat(SlotThermostat);
      
      // Start the heater and the fans
      nLowDigOut(2, Souliss_T3n_HeatingOn, SlotThermostat);    // Heater
      nLowDigOut(3, Souliss_T3n_FanOn1,  SlotThermostat);    // Fan1
      nLowDigOut(4, Souliss_T3n_FanOn2, SlotThermostat);    // Fan2
      nLowDigOut(5, Souliss_T3n_FanOn3, SlotThermostat);    // Fan3
      
      // Process the communication
      FAST_PeerComms();

      FAST_1110ms() {

        sensors.requestTemperatures(); // Send the command to get temperatures
        // SENSORE 1
        temperature = sensors.getTempCByIndex(0);
        ImportAnalog(SlotThermostat + 1, &temperature);

      }

      FAST_510ms() {
        Serial.println(temperature);
        Serial.print("Set Point ");
        float m_in;
        float32((U16*)(memory_map + MaCaco_IN_s + SlotThermostat+3), &m_in);
        Serial.println(m_in);
      }
    }
  }
}
