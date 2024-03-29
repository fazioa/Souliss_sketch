/**************************************************************************
  Sketch: POWER SOCKET - VER.1 - Souliss - Static Configuration
  Author: Tonino Fazio

  ESP Core 2.3.0
  This example is only supported on ESP8266.
  Programm it with "Generic ESP8266 Module with 4M (1M SPIFFS)
***************************************************************************/
#include "SoulissFramework.h"
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>

#include <ArduinoOTA.h>

#include <EEPROM.h>
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/IPBroadcast.h"

// **** Define the WiFi name and password ****
#define WIFICONF_INSKETCH
#define WiFi_SSID               "asterix"
#define WiFi_Password           "ttony2013"
// Include framework code and libraries

#include "Souliss.h"



//*************************************************************************
// Define the network configuration according to your router settingsuration according to your router settings
// and the other on the wireless oneless one
#define peer_address  0xAB15
#define myvNet_subnet 0xFF00
#define myvNet_supern 0xAB10
//*************************************************************************

#define SLOT_POWERSOCKET 0
#define PIN_POWERSOCKET 12
#define PIN_LED 13
#define PIN_BUTTON 0

boolean bLedState = false;
void setup()
{
  // Define output pins
  pinMode(PIN_POWERSOCKET, OUTPUT);    // Relè
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUTTON, INPUT);

  digitalWrite(PIN_LED, LOW);

  //delay 10 seconds
  delay(10000);
  Initialize();
  GetIPAddress();

  SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  //*************************************************************************
  //*************************************************************************
  Set_SimpleLight(SLOT_POWERSOCKET);


  // Init the OTA
  ArduinoOTA.setHostname("souliss-SONOFF-powerswitch");
  ArduinoOTA.begin();
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_50ms() {
      Logic_SimpleLight(SLOT_POWERSOCKET);
      DigOut(PIN_POWERSOCKET, Souliss_T1n_Coil, SLOT_POWERSOCKET);
    }
    FAST_510ms() {
      if (!digitalRead(PIN_BUTTON)) {
        mInput(SLOT_POWERSOCKET)= Souliss_T1n_ToggleCmd;
      }
    }


    FAST_1110ms() {
      bLedState = !bLedState;
      digitalWrite(PIN_LED, bLedState);
    }

    FAST_PeerComms();
  }

  EXECUTESLOW() {
    UPDATESLOW();
    SLOW_10s() {  // Process the timer every 10 seconds
      Timer_SimpleLight(SLOT_POWERSOCKET);
    }
  }

  // Look for a new sketch to update over the air
  ArduinoOTA.handle();
}


