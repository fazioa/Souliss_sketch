//#define VNET_DEBUG_INSKETCH
//#define VNET_DEBUG  1
//#define  SOULISS_DEBUG_INSKETCH
//#define SOULISS_DEBUG      1
//#define  MaCaco_DEBUG_INSKETCH
//#define MaCaco_DEBUG      1
/**************************************************************************
  Sketch: ESP8266 WiFi Relay V3 - Souliss
  Author: Tonino Fazio

  This example is only supported on ESP8266.

  //Used pins
  // pin 12: onboad relay ON
  // pin 13: onboad relay OFF
  // pin 14: switch
***************************************************************************/
#include "SoulissFramework.h"
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/IPBroadcast.h"

// **** Define the WiFi name and password ****
#define WIFICONF_INSKETCH
#define WiFi_SSID               "Souliss"
#define WiFi_Password           ""


// Include framework code and libraries
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "Souliss.h"

//*************************************************************************
// Define the network configuration according to your router settingsuration according to your router settings
// and the other on the wireless oneless one
#define peer_address  0xAB02
#define myvNet_subnet 0xFF00
#define myvNet_supern 0xAB01

//*************************************************************************

#define SLOT_RELAY_0 0
#define SLOT_RELAY_1 1
#define PIN_SWITCH 14
#define PIN_RELAY_ON 12
#define PIN_RELAY_OFF 13
#define PIN_LED 2

void setup()
{
  Serial.begin(9600);
  Initialize();
   GetIPAddress();
  
  SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface
  
  //*************************************************************************
  //*************************************************************************

  pinMode(PIN_SWITCH, INPUT_PULLUP);    // Switch

  digitalWrite(PIN_RELAY_ON, LOW);
  pinMode(PIN_RELAY_ON, OUTPUT);    // Relay ON

  digitalWrite(PIN_RELAY_OFF, LOW);
  pinMode(PIN_RELAY_OFF, OUTPUT);    // Relay OFF
  pinMode(PIN_LED, OUTPUT);

  Set_SimpleLight(SLOT_RELAY_0);



  mOutput(SLOT_RELAY_0) = Souliss_T1n_OnCoil; //Set output to ON, then first execution of DigIn2State cause a change state to OFF.

}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_2110ms() {
    //  activity();
    }

    FAST_50ms() {
      DigIn2State(PIN_SWITCH, Souliss_T1n_ToggleCmd, Souliss_T1n_ToggleCmd, SLOT_RELAY_0);
      Logic_SimpleLight(SLOT_RELAY_0);
      Logic_SimpleLight(SLOT_RELAY_1);
      Logic_SimpleLight(SLOT_RELAY_1);
      PulseLowDigOut(PIN_RELAY_ON, Souliss_T1n_OnCoil, SLOT_RELAY_0);
      PulseLowDigOut(PIN_RELAY_OFF, Souliss_T1n_OffCoil, SLOT_RELAY_0);

      // Example for other optional relay
      // Logic_SimpleLight(SLOT_RELAY_1);
      // DigOut(PIN_RELAY_1, Souliss_T1n_Coil, SLOT_RELAY_1);
    }
    FAST_PeerComms();
  }
}

U8 activity_led_status = 0;
void activity() {
  if (activity_led_status == 0) {
    digitalWrite(PIN_LED, HIGH);
    activity_led_status = 1;
  } else {
    digitalWrite(PIN_LED, LOW);
    activity_led_status = 0;
  }
}
