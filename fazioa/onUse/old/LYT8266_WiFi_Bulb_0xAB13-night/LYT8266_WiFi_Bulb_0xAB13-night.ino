/**************************************************************************
   Souliss - LYT8266 WiFi RGBW LED Bulb
  Upload:
  Generic ESP8266
  Flash 4Mb (1Mb SPIFFS)

***************************************************************************/

// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"

// **** Define the WiFi name and password ****
#define WIFICONF_INSKETCH
#define WiFi_SSID               "asterix"
#define WiFi_Password           "ttony2013"

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
#define wifi_bridge_address    0xAB10 //gateway
#define myvNet_supern wifi_bridge_address
//*************************************************************************



// Define logic slots, multicolor lights use four slots
#define AUTOLIGHT1  0
#define LYTLIGHT1           1

#define RED_STARTUP         0x20
#define GREEN_STARTUP       0x00
#define BLUE_STARTUP        0x00

U8 RED_VAL = RED_STARTUP;
U8 GREEN_VAL = GREEN_STARTUP;
U8 BLUE_VAL = GREEN_STARTUP;
U8 brightness = 100;
void setup()
{
  Serial.begin(57600);
  // Init the network stack and the bulb, turn on with a warm amber
  Initialize();
  InitLYT();
SetColor(LYTLIGHT1, RED_STARTUP, GREEN_STARTUP, BLUE_STARTUP);

  GetIPAddress();
  SetAddress(peer_wifi_address_LYT, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  // Define a logic to handle the bulb
  SetLYTLamps(LYTLIGHT1);
  

  Set_SimpleLight(AUTOLIGHT1);
  // Init the OTA
  ArduinoOTA.setHostname("souliss-LYT");
  ArduinoOTA.begin();
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();
    ProcessCommunication();
    Logic_SimpleLight(AUTOLIGHT1);
    //la lampada RGB funziona solo se lo switch allo slot 0 è ON, altrimenti funziona con l'automatismo notte
    if (mOutput(AUTOLIGHT1) == Souliss_T1n_OnCoil)
    {
      LogicLYTLamps(LYTLIGHT1);
    } else {

      FAST_2110ms() {
        if (RED_VAL < 225) RED_VAL = mOutput(LYTLIGHT1 + 1) + 1;
        if (RED_VAL >= 220) {
          if (GREEN_VAL < 225) GREEN_VAL = mOutput(LYTLIGHT1 + 2) + 2;
          if (BLUE_VAL < 225) BLUE_VAL = mOutput(LYTLIGHT1 + 3) + 1;
        }

        if (BLUE_VAL >= 225 && GREEN_VAL >= 225 && RED_VAL >= 225) {
          if (brightness < 255) ++brightness;
          Souliss_SetWhite(memory_map, LYTLIGHT1, &data_changed, brightness);
        } else
        {
          SetColor(LYTLIGHT1, RED_VAL, GREEN_VAL, BLUE_VAL);
        }
        Serial.print("Red: ");
        Serial.println(RED_VAL);

        Serial.print("Rreen: ");
        Serial.println(GREEN_VAL);
        Serial.print("Blue: ");
        Serial.println(BLUE_VAL);
        Serial.print("Brightness: ");
        Serial.println(brightness);
      }
    }

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
