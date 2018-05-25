//**************************************************************************
//  Sketch: POWER SOCKET - VER.2 - Souliss - Static Configuration
//  Author: Tonino Fazio
//
//  ESP Core 2.3.0
//  This example is only supported on ESP8266.
//  Programm it with "Generic ESP8266 Module with 4M (1M SPIFFS)
//***************************************************************************/
#define SERIAL_DEBUG

#define  VNET_RESETTIME_INSKETCH
#define VNET_RESETTIME      0x00042F7 // ((20 Min*60)*1000)/70ms = 17143 => 42F7
//#define VNET_RESETTIME      0x0000359 // 857 -> x359 1 minuto
#define VNET_HARDRESET      ESP.reset()

#define HOSTNAME "souliss-SONOFF-divano-TEST"

#include "SoulissFramework.h"
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>



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
#define peer_address  0xAB14
#define myvNet_subnet 0xFF00
#define myvNet_supern 0xAB10
//*************************************************************************

#define SLOT_POWERSOCKET 0
#define PIN_POWERSOCKET 12
#define PIN_LED 13
#define PIN_BUTTON_0 0
#define PIN_BUTTON_14 14

//Variable to Handle WiFio Signal
long rssi = 0;
int bars = 0;
#define T_WIFI_STRDB  3 //It takes 2 slots
#define T_WIFI_STR    5 //It takes 2 slots

boolean bLedState = false;
void setup()
{
#ifdef SERIAL_DEBUG
    Serial.begin(115200);
    Serial.println("Node Starting");
  #endif
  
  // Define output pins
  pinMode(PIN_POWERSOCKET, OUTPUT);    // Relè
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUTTON_14, INPUT_PULLUP);

  digitalWrite(PIN_LED, LOW);


  //delay 10 seconds
  // delay(10000);

  Initialize();
  GetIPAddress();

  SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  //*************************************************************************
  //*************************************************************************
  Set_SimpleLight(SLOT_POWERSOCKET);
  mOutput(SLOT_POWERSOCKET) = Souliss_T1n_OnCoil;

  Set_T51(T_WIFI_STRDB); //Imposto il tipico per contenere il segnale del Wifi in decibel
  Set_T51(T_WIFI_STR); //Imposto il tipico per contenere il segnale del Wifi in barre da 1 a 5

  // Init the OTA
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.begin();
  
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

    FAST_50ms() {
      DigIn2State(PIN_BUTTON_14, Souliss_T1n_ToggleCmd, Souliss_T1n_ToggleCmd, SLOT_POWERSOCKET);
      Logic_SimpleLight(SLOT_POWERSOCKET);
      DigOut(PIN_POWERSOCKET, Souliss_T1n_Coil, SLOT_POWERSOCKET);
    }

    FAST_2110ms() {
      //Processa le logiche per il segnale WiFi
      Read_T51(T_WIFI_STRDB);
      Read_T51(T_WIFI_STR);
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
      check_wifi_signal();
    }
  }
 
 // Look for a new sketch to update over the air
 ArduinoOTA.handle();
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
