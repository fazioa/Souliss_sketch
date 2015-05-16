#define  BOARDTYPE_INSKETCH
#define  QC_BOARDTYPE  0x01      /** Freaklabs Chibiduino */

#define  GATEWAYTYPE_INSKETCH
#define  QC_GATEWAYTYPE  0x00    // NO Define board as gateway

// FROM
// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

#include "bconf/Chibiduino_v1.h"			// Use a Chibiduino 2.4 GHz wireless board

#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>
#include <OneWire.h>
#include "DallasTemperature.h"

#define TEMPERATURE				0
// Define the network configuration according to your router settingsuration according to your router settings
#define	Gateway_address	0x6501				// The Gateway node has two address, one on the Ethernet side69				// The Gateway node has two address, one on the Ethernet side
											// and the other on the wireless oneless one
#define	Peer_address	0x6502
#define	myvNet_subnet	0xFF00
#define	myvNet_supern	Gateway_address

#define DSPIN       14

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(DSPIN);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

void setup()
{
  Serial.begin(9600);

  Initialize();
  // Setup the network configuration
  Souliss_SetAddress(Peer_address, myvNet_subnet, myvNet_supern);					// Address on the wireless interface
  // Set the typical to use
  Set_T52(TEMPERATURE);
  // Start up the library
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12.
}

void loop()
{
   EXECUTEFAST() {
    UPDATEFAST();
    FAST_50ms() {
      Logic_T52(TEMPERATURE);

      // Process the communication
      FAST_PeerComms();
    }
  }
  
FAST_2110ms()	{
      float temperature;
      sensors.requestTemperatures(); // Send the command to get temperatures
      // SENSORE 1
      temperature = sensors.getTempCByIndex(0);
      Souliss_ImportAnalog(memory_map, TEMPERATURE, &temperature);
    }
  }
