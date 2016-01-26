#define HOST_NAME_INSKETCH
#define HOST_NAME "ESP8266-WiFi-Relay-V3"

/**************************************************************************
Sketch: POWER SOCKET - VER.1 - Souliss - Web Configuration
Author: Tonino Fazio

ESP Core 1.6.5 Staging 1.6.5-1160-gef26c5f
 This example is only supported on ESP8266.
***************************************************************************/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <WiFiUdp.h>

// Configure the Souliss framework
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/RuntimeGateway.h"            // This node is a Peer and can became a Gateway at runtime
#include "conf/DynamicAddressing.h"         // Use dynamically assigned addresses
#include "conf/WEBCONFinterface.h"          // Enable the WebConfig interface

#include "Souliss.h"
#include "DHT.h"
//*************************************************************************
//*************************************************************************

#define SLOT_RELAY_0 0
#define SLOT_TEMPERATURE        1     // This is the memory slot used for the execution of the logic in network_address1
#define SLOT_HUMIDITY        3     // This is the memory slot used for the execution of the logic
//#define SLOT_RELAY_1 1
//#define SLOT_RELAY_2 2
//#define SLOT_RELAY_3 3
//#define SLOT_RELAY_4 4
//#define SLOT_RELAY_5 5
//#define SLOT_RELAY_6 6
//#define SLOT_RELAY_7 7
//#define SLOT_RELAY_8 8
//#define SLOT_RELAY_9 9


//#define PIN_0 0
//#define PIN_2 2
//#define PIN_4 4
//#define PIN_5 5
#define PIN_14 14
//#define PIN_16 16
//
//#define PIN_1 1
#define PIN_12 12
#define PIN_13 13
//#define PIN_15 15

#define PIN_DHT      16
#define DHTTYPE DHT22   // DHT 22 
#define DEADBAND        0.01    // Deadband value 1% 
// Initialize DHT sensor for normal 8mhz Arduino
DHT dht(PIN_DHT, DHTTYPE, 2);

// Setup the libraries for Over The Air Update
OTA_Setup();
void setup()
{
  //delay 30 seconds
  delay(15000);
  Initialize();

  // Read the IP configuration from the EEPROM, if not available start
  // the node as access point
  if (!ReadIPConfiguration())
  {
    // Start the node as access point with a configuration WebServer
    SetAccessPoint();
    startWebServer();
    // We have nothing more than the WebServer for the configuration
    // to run, once configured the node will quit this.
    while (1)
    {
      yield();
      runWebServer();
    }

  }

  if (IsRuntimeGateway())
  {
    // Connect to the WiFi network and get an address from DHCP
    SetAsGateway(myvNet_dhcp);       // Set this node as gateway for SoulissApp
    SetAddressingServer();
  }
  else
  {
    // This board request an address to the gateway at runtime, no need
    // to configure any parameter here.
    SetDynamicAddressing();
    GetAddress();
  }

  //*************************************************************************
  //*************************************************************************
  Set_SimpleLight(SLOT_RELAY_0);
  Set_Temperature(SLOT_TEMPERATURE);
  Set_Humidity(SLOT_HUMIDITY);
  pinMode(PIN_DHT, INPUT);
  dht.begin();
  // Set_SimpleLight(SLOT_RELAY_1);
  // Set_SimpleLight(SLOT_RELAY_2);
  // Set_SimpleLight(SLOT_RELAY_3);
  //  Set_SimpleLight(SLOT_RELAY_4);
  //  Set_SimpleLight(SLOT_RELAY_5);

  //  Set_SimpleLight(SLOT_RELAY_6);
  // Set_SimpleLight(SLOT_RELAY_7);
  // Set_SimpleLight(SLOT_RELAY_8);
  //  Set_SimpleLight(SLOT_RELAY_9);

  // Define output pins
  //  pinMode(PIN_0, OUTPUT);    // Relè
  //  pinMode(PIN_2, OUTPUT);    // Relè
  //  pinMode(PIN_4, OUTPUT);    // Relè
  //  pinMode(PIN_5, OUTPUT);    // Relè
  pinMode(PIN_14, INPUT_PULLUP);    // Relè

  //  pinMode(PIN_16, OUTPUT);    // Relè
  //
  //  pinMode(PIN_1, OUTPUT);    // Relè

  digitalWrite(PIN_12, LOW);
  pinMode(PIN_12, OUTPUT);    // Relè
  digitalWrite(PIN_13, LOW);
  pinMode(PIN_13, OUTPUT);    // Relè
  //  pinMode(PIN_15, OUTPUT);    // Relè


  // Init the OTA
  OTA_Init();
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_50ms() {
      DigIn2State(PIN_14, Souliss_T1n_ToggleCmd, Souliss_T1n_ToggleCmd, SLOT_RELAY_0);
      Logic_SimpleLight(SLOT_RELAY_0);
      PulseDigOut(PIN_12, Souliss_T1n_OnCoil, SLOT_RELAY_0);
      PulseDigOut(PIN_13, Souliss_T1n_OffCoil, SLOT_RELAY_0);

      //
      //      Logic_SimpleLight(SLOT_RELAY_1);
      //      DigOut(PIN_2, Souliss_T1n_Coil, SLOT_RELAY_1);
      //
      //      Logic_SimpleLight(SLOT_RELAY_2);
      //      DigOut(PIN_4, Souliss_T1n_Coil, SLOT_RELAY_2);
      //
      //      Logic_SimpleLight(SLOT_RELAY_3);
      //      DigOut(PIN_5, Souliss_T1n_Coil, SLOT_RELAY_3);
      //
      //      Logic_SimpleLight(SLOT_RELAY_4);
      //      DigOut(PIN_14, Souliss_T1n_Coil, SLOT_RELAY_4);
      //
      //      Logic_SimpleLight(SLOT_RELAY_5);
      //      DigOut(PIN_16, Souliss_T1n_Coil, SLOT_RELAY_5);
      //
      //      Logic_SimpleLight(SLOT_RELAY_6);
      //      DigOut(PIN_1, Souliss_T1n_Coil, SLOT_RELAY_6);
      //
      //      Logic_SimpleLight(SLOT_RELAY_7);
      //      DigOut(PIN_12, Souliss_T1n_Coil, SLOT_RELAY_7);
      //
      //      Logic_SimpleLight(SLOT_RELAY_8);
      //      DigOut(PIN_13, Souliss_T1n_Coil, SLOT_RELAY_8);

      //      Logic_SimpleLight(SLOT_RELAY_9);
      //      DigOut(PIN_15, Souliss_T1n_Coil, SLOT_RELAY_9);

    }

    FAST_510ms() {
      Logic_Temperature(SLOT_TEMPERATURE);
      Logic_Humidity(SLOT_HUMIDITY);
    }


    // Run communication as Gateway or Peer
    if (IsRuntimeGateway())
      FAST_GatewayComms();
    else
      FAST_PeerComms();
  }

  EXECUTESLOW() {
    UPDATESLOW();
    SLOW_10s() {  // Process the timer every 10 seconds
      Timer_SimpleLight(SLOT_RELAY_0);
      // Read temperature value from DHT sensor and convert from single-precision to half-precision
      float temperature = dht.readTemperature();
      ImportAnalog(SLOT_TEMPERATURE, &temperature);

      // Read humidity value from DHT sensor and convert from single-precision to half-precision
      float humidity = dht.readHumidity();
      ImportAnalog(SLOT_HUMIDITY, &humidity);

    }
    // If running as Peer
    if (!IsRuntimeGateway())
      SLOW_PeerJoin();
  }
  // Look for a new sketch to update over the air
  OTA_Process();
}


