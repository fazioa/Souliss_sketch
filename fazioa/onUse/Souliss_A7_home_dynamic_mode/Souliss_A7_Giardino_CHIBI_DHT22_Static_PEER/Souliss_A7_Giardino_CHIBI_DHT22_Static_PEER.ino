//DA compilare selezionando la scheda ARDUINO PRO OR PRO MINI  e processore ATMEGA328 3.3V 8 MHZ

/**************************************************************************
Souliss Bridge

Setting on QuickConf file:
QC_BOARDTYPE			0x01
QC_GATEWAYTYPE			0x00
DHT_SENSOR				0x01
DHT on PIN 2
***************************************************************************/
// Configure the framework
#include "bconf/Chibiduino_v1.h"			// Use a Chibiduino 2.4 GHz wireless board

#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>
#include "DHT.h"


// Define the network configuration according to your router settingsuration according to your router settings
#define	Gateway_address	0x6511				// The Gateway node has two address, one on the Ethernet side 69				// The Gateway node has two address, one on the Ethernet side
// and the other on the wireless oneless one
#define	peer_address	0x6513
#define	myvNet_subnet	0xFF00
#define	myvNet_supern	Gateway_address

#define PIN_DHT      2

#define DHTTYPE DHT22   // DHT 22  

#define TEMPERATURE				0			// This is the memory slot used for the execution of the logic in network_address1
#define HUMIDITY				2			// This is the memory slot used for the execution of the logic
#define DEADBAND				0.01		// Deadband value 1%  

// Initialize DHT sensor for normal 8mhz Arduino
DHT dht(PIN_DHT, DHTTYPE, 2);

void setup()
{
  //Serial.begin(9600);

  Initialize();

  // Setup the network configuration
  Souliss_SetAddress(peer_address, myvNet_subnet, myvNet_supern);					// Address on the wireless interface

  // Set the typical to use
  Set_T52(TEMPERATURE);
  Set_T53(HUMIDITY);
  pinMode(PIN_DHT, INPUT);

  dht.begin();
}

void loop()
{

  EXECUTEFAST() {
    UPDATEFAST();

    // Process every 510ms the logic that control the curtain
    FAST_510ms() {
      // Compare the acquired input with the stored one, send the new value to the
      // user interface if the difference is greater than the deadband
      Logic_T52(TEMPERATURE);
      Logic_T53(HUMIDITY);
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

