#define HOST_NAME_INSKETCH
#define HOST_NAME "Souliss-TEST-1"

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

//*************************************************************************
//*************************************************************************

#define SLOT_POWERSOCKET 0
#define PIN_POWERSOCKET 14

// Setup the libraries for Over The Air Update
OTA_Setup();
void setup()
{
  //delay 30 seconds
  delay(30000);
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
  Set_SimpleLight(SLOT_POWERSOCKET);

  // Define output pins
  pinMode(PIN_POWERSOCKET, OUTPUT);    // Relè

  // Init the OTA
  OTA_Init();
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_50ms() {
      Logic_SimpleLight(SLOT_POWERSOCKET);
      DigOut(PIN_POWERSOCKET, Souliss_T1n_Coil, SLOT_POWERSOCKET);
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
      Timer_SimpleLight(SLOT_POWERSOCKET);
    }
    // If running as Peer
    if (!IsRuntimeGateway())
      SLOW_PeerJoin();
  }
  // Look for a new sketch to update over the air
  OTA_Process();
}


