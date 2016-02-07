#define HOST_NAME_INSKETCH
#define HOST_NAME "ESP8266-WiFi-Relay-V3"

/**************************************************************************
Sketch: ESP8266 WiFi Relay V3 - Souliss - Web Configuration
Author: Tonino Fazio

This example is only supported on ESP8266.

 //Used pins
// pin 12: onboad relay ON
// pin 13: onboad relay OFF
// pin 14: switch
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

#define SLOT_RELAY_0 0

// Example for other optional relay
//#define SLOT_RELAY_1 1
//#define PIN_RELAY_1 16

#define PIN_SWITCH 14
#define PIN_RELAY_ON 12
#define PIN_RELAY_OFF 13
#define PIN_LED 2

// Setup the libraries for Over The Air Update
OTA_Setup();
void setup()
{
  //delay 15 seconds
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

  // Example for other optional relay
  // Set_SimpleLight(SLOT_RELAY_1);
  //digitalWrite(PIN_RELAY_1, LOW);
  //pinMode(PIN_RELAY_1, OUTPUT);    // Relay 1

  pinMode(PIN_SWITCH, INPUT_PULLUP);    // Switch

  digitalWrite(PIN_RELAY_ON, LOW);
  pinMode(PIN_RELAY_ON, OUTPUT);    // Relay ON

  digitalWrite(PIN_RELAY_OFF, LOW);
  pinMode(PIN_RELAY_OFF, OUTPUT);    // Relay OFF

  pinMode(PIN_LED, OUTPUT);

  // Init the OTA
  OTA_Init();
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();
    FAST_210ms() {
      check_if_joined();
    }


    FAST_50ms() {
      DigIn2State(PIN_SWITCH, Souliss_T1n_ToggleCmd, Souliss_T1n_ToggleCmd, SLOT_RELAY_0);
      Logic_SimpleLight(SLOT_RELAY_0);
      PulseDigOut(PIN_RELAY_ON, Souliss_T1n_OnCoil, SLOT_RELAY_0);
      PulseDigOut(PIN_RELAY_OFF, Souliss_T1n_OffCoil, SLOT_RELAY_0);

      // Example for other optional relay
      // Logic_SimpleLight(SLOT_RELAY_1);
      // DigOut(PIN_RELAY_1, Souliss_T1n_Coil, SLOT_RELAY_1);

    }

    // Run communication as Gateway or Peer
    if (IsRuntimeGateway())
      FAST_GatewayComms();
    else
      FAST_PeerComms();
  }

  EXECUTESLOW() {
    UPDATESLOW();
    // If running as Peer
    if (!IsRuntimeGateway())
      SLOW_PeerJoin();
  }
  // Look for a new sketch to update over the air
  OTA_Process();
}

U8 led_status = 0;
U8 joined = 0;
void check_if_joined() {
  if (JoinInProgress() && joined == 0) {
    joined = 0;
    if (led_status == 0) {
      digitalWrite(PIN_LED, HIGH);
      led_status = 1;
    } else {
      digitalWrite(PIN_LED, LOW);
      led_status = 0;
    }
  } else {
    joined = 1;
    digitalWrite(PIN_LED, HIGH);
  }
}
