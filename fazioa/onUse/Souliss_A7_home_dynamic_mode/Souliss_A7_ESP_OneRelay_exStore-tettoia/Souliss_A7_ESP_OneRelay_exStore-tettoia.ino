/**************************************************************************
Sketch: ESP8266 WiFi Relay V3 - Souliss - Web Configuration
Author: Tonino Fazio

This example is only supported on ESP8266.

 //Used pins
// pin 12: onboad relay ON
// pin 13: onboad relay OFF
// pin 14: switch
***************************************************************************/
//#define SERIAL_DEBUG

// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define HOSTNAME "souliss-EXSTORE-tettoia"

// Configure the framework
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/IPBroadcast.h"

// **** Define the WiFi name and password ****
#define WIFICONF_INSKETCH
#define WiFi_SSID               "asterix"
#define WiFi_Password           "ttony2013"

#include "Souliss.h"
//*************************************************************************
// Define the network configuration according to your router settingsuration according to your router settings
// and the other on the wireless oneless one
#define peer_address  0xAB11
#define myvNet_subnet 0xFF00
#define myvNet_supern    0xAB10 //gateway
//*************************************************************************

#define SLOT_RELAY_0 0

// Example for other optional relay
//#define SLOT_RELAY_1 1
//#define PIN_RELAY_1 16

#define PIN_SWITCH 14
#define PIN_RELAY_ON 12
#define PIN_RELAY_OFF 13
#define PIN_LED 2

//Variable to Handle WiFio Signal
long rssi = 0;
int bars = 0;
#define T_WIFI_STRDB  1 //It takes 2 slots
#define T_WIFI_STR    3 //It takes 2 slots

void setup()
{
#ifdef SERIAL_DEBUG
  Serial.begin(115200);
  Serial.println("Node Starting");
#endif

  //delay 11 seconds
  delay(11000);
  
  Initialize();
 
   // Connect to the WiFi network and get an address from DHCP
  GetIPAddress();
  // This is the vNet address for this node, used to communicate with other
  // nodes in your Souliss network
  SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface



  //*************************************************************************
  //*************************************************************************

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

  Set_SimpleLight(SLOT_RELAY_0);
  mOutput(SLOT_RELAY_0) = Souliss_T1n_OnCoil; //Set output to ON, then first execution of DigIn2State cause a change state to OFF.

  Set_T51(T_WIFI_STRDB); //Imposto il tipico per contenere il segnale del Wifi in decibel
  Set_T51(T_WIFI_STR); //Imposto il tipico per contenere il segnale del Wifi in barre da 1 a 5

  // Init the OTA
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.begin();
  
  //End setup
  digitalWrite(PIN_LED, HIGH);
  
#ifdef SERIAL_DEBUG
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("IP:  ");
  Serial.println(WiFi.localIP());
  Serial.print("Subnet: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println("Node Initialized");
#endif
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_2110ms() {
      activity();
    }
 FAST_2110ms() {
      //Processa le logiche per il segnale WiFi
      Read_T51(T_WIFI_STRDB);
      Read_T51(T_WIFI_STR);
    }
    
    FAST_50ms() {
      DigIn2State(PIN_SWITCH, Souliss_T1n_ToggleCmd, Souliss_T1n_ToggleCmd, SLOT_RELAY_0);
      Logic_SimpleLight(SLOT_RELAY_0);
      PulseLowDigOut(PIN_RELAY_ON, Souliss_T1n_OnCoil, SLOT_RELAY_0);
      PulseLowDigOut(PIN_RELAY_OFF, Souliss_T1n_OffCoil, SLOT_RELAY_0);

      // Example for other optional relay
      // Logic_SimpleLight(SLOT_RELAY_1);
      // DigOut(PIN_RELAY_1, Souliss_T1n_Coil, SLOT_RELAY_1);

    }
    FAST_PeerComms();
  }
    // Look for a new sketch to update over the air
  ArduinoOTA.handle();
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

void check_wifi_signal() {
  long rssi = WiFi.RSSI();
  int bars = 0;

  if (rssi > -55) {
    bars = 5;
  }
  else if (rssi < -55 & rssi > -65) {
    bars = 4;
  }
  else if (rssi < -65 & rssi > -70) {
    bars = 3;
  }
  else if (rssi < -70 & rssi > -78) {
    bars = 2;
  }
  else if (rssi < -78 & rssi > -82) {
    bars = 1;
  }
  else {
    bars = 0;
  }
  float f_rssi = (float)rssi;
  float f_bars = (float)bars;
  ImportAnalog(T_WIFI_STRDB, &f_rssi);
  ImportAnalog(T_WIFI_STR, &f_bars);

#ifdef SERIAL_DEBUG
  Serial.print("wifi rssi:");
  Serial.println(f_rssi);
  Serial.print("wifi bars:");
  Serial.println(f_bars);
#endif
}
