/**************************************************************************
   Souliss - LYT8266 WiFi RGBW LED Bulb

    This is the basic sketch for an LYT8266 a WiFi LED Bulb based on ESP8266,
    at first boot it starts as access point with a basic web interface to
    allow WiFi and Souliss configuration.
    Info are stored in the module and used at next boot.

    Configure one module as Gateway for your network and all the other as
    Peer. Before load this sketch, run the e02_LYT8266_WiFi_Erase to erase
    the memory.

    Once configured, can be accessed via SoulissApp or openHAB.

***************************************************************************/

// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"

// **** Define the WiFi name and password ****
#define WIFICONF_INSKETCH
#define WiFi_SSID               "souliss"
#define WiFi_Password           "souliss01"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Configure the Souliss framework
#include "bconf/LYT8266_LEDBulb.h"          // Load the code directly on the ESP8266
#include "conf/IPBroadcast.h"

//#include "conf/RuntimeGateway.h"            // This node is a Peer and can became a Gateway at runtime
//#include "conf/DynamicAddressing.h"         // Use dynamically assigned addresses
//#include "conf/WEBCONFinterface.h"          // Enable the WebConfig interface



/*** All configuration includes should be above this line ***/
#include "Souliss.h"

//*************************************************************************
// Define the network configuration according to your router settingsuration according to your router settings
// and the other on the wireless oneless one
#define peer_wifi_address_LYT  0xAB13 //LYT
//#define peer_wifi_address_LYT  0xAB14 //LYT
#define myvNet_subnet 0xFF00
#define wifi_bridge_address    0xAB01 //gateway
#define myvNet_supern wifi_bridge_address
//*************************************************************************



// Define logic slots, multicolor lights use four slots
#define LYTLIGHT1           0

#define RED_STARTUP         0x50
#define GREEN_STARTUP       0x50
#define BLUE_STARTUP        0x50

#define WHITE_STARTUP        0xff
void setup()
{
  // Init the network stack and the bulb, turn on with a warm amber
  Initialize();
  InitLYT();

  /****
      Generally set a PWM output before the connection will lead the
      ESP8266 to reboot for a conflict on the FLASH write access.

      Here we do the configuration during the WebConfig and so we don't
      need to write anything in the FLASH, and the module can connect
      to the last used network.

      If you don't use the WebConfig use a dummy sketch that connects to
      your WiFi and then use this sketch
  ****/
  //SetColor(LYTLIGHT1, RED_STARTUP, GREEN_STARTUP, BLUE_STARTUP);
  SetWhite(LYTLIGHT1, WHITE_STARTUP);
  // Read the IP configuration from the EEPROM, if not available start
  // the node as access point.
  //
  // If you want to force the device in WebConfiguration mode, power OFF
  // your router and power OFF and then ON the bulb, you will see an access
  // point called Souliss.

  GetIPAddress();
  SetAddress(peer_wifi_address_LYT, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  // Define a logic to handle the bulb
  SetLYTLamps(LYTLIGHT1);

  // Init the OTA
  ArduinoOTA.setHostname("souliss-LYT");
  ArduinoOTA.begin();
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    // Is an unusual approach, but to get fast response to color change we run the LYT logic and
    // basic communication processing at maximum speed.
    LogicLYTLamps(LYTLIGHT1);
    ProcessCommunication();

    FAST_PeerComms();
  }

  EXECUTESLOW() {
    UPDATESLOW();

    // Slowly shut down the lamp
    SLOW_10s() {
      LYTSleepTimer(LYTLIGHT1);
    }

    SLOW_PeerJoin();
  }

  // Look for a new sketch to update over the air
  ArduinoOTA.handle();
}
