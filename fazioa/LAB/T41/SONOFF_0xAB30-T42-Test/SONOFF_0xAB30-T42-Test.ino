/**************************************************************************
  Interruttore luce cucina
  Funziona con switch collegato al pin 14

  Sketch: POWER SOCKET - VER.2 - Souliss - Static Configuration
  Author: Tonino Fazio

  ESP Core 2.3.0
  This example is only supported on ESP8266.

  parametri upload Arduino IDE:
  – ESP8266 Generic
  – Flash Mode: DIO
  – Crystal Frequency: 26 MHz (non presente su IDE 1.6.12)
  – Flash Frequency 80 MHz
  – CPU frequency 160 MHz
  – Flash Size 1 MB (256K SPIFFS)
***************************************************************************/
//#define SERIAL_DEBUG

// RESET OGNI 20 MIN SE NON E' COLLEGATO AL GATEWAY
#define  VNET_RESETTIME_INSKETCH
#define VNET_RESETTIME      0x00042F7 // ((20 Min*60)*1000)/70ms = 17143 => 42F7
#define VNET_HARDRESET      ESP.reset()

#include "SoulissFramework.h"
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "conf/Gateway.h"

#define HOSTNAME "souliss-SONOFF-GW-TEST"

#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/IPBroadcast.h"

#include "credenziali.h"
// **** creare un file di testo chiamato credenziali.h con il seguente contenuto personalizzato ****
//#define WIFICONF_INSKETCH
//#define WiFi_SSID               "SSID"
//#define WiFi_Password           "PWD"

// **** Define Telegram parameters ****
//#define   BOTTOKEN "XXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"  // your Bot Token (Get from Botfather)
//#define  CHAT_ID "chat ID number"

// Include framework code and libraries
#include "Souliss.h"
#include <UniversalTelegramBot.h>


//*************************************************************************
// Define the network configuration according to your router settingsuration according to your router settings
// and the other on the wireless oneless one
#define peer_address2  0xAB31
#define myvNet_subnet 0xFF00
#define Gateway_address 0xAB30
uint8_t ip_gateway[4]  = {192, 168, 1, 1};
//*************************************************************************

#define ANTITHEFT               0           // This is the memory slot used for the execution of the anti-theft
#define WATCHDOG                1           // This is the memory slot used for the execution of the watchdog

void setup()
{
  Initialize();
  GetIPAddress();
  SetAsGateway(myvNet_subnet);
  Souliss_SetAddress(Gateway_address, myvNet_subnet, Gateway_address);
  SetAsPeerNode(peer_address2, 1);

  // Setup the anti-theft
  Set_T41(ANTITHEFT);

  // Define inputs, outputs pins and pullup
  pinMode(2, INPUT_PULLUP);  // Hardware pullup required
  pinMode(16, OUTPUT);
  pinMode(3, OUTPUT);
}

void loop()
{
  // Here we start to play
  EXECUTEFAST() {
    UPDATEFAST();

    // Process every 510ms the logic that control the curtain
    FAST_510ms() {

      // Input from anti-theft sensor
      LowDigIn(2, Souliss_T4n_Alarm, ANTITHEFT);

      // Execute the anti-theft logic and report to the main node
      Logic_T41(ANTITHEFT);

      // Set the Pin if the anti-theft is activated
      nDigOut(3, Souliss_T4n_Antitheft, ANTITHEFT);

      // Set the Pin if the alarm is raised
      DigOut(16, Souliss_T4n_InAlarm, ANTITHEFT);
    }

    // Process the communication
    FAST_GatewayComms();

    // Execute the code every 2110ms
    FAST_2110ms()   {

      // Build a watchdog chain to monitor the nodes
      mInput(ANTITHEFT) = Watchdog(peer_address2, WATCHDOG, Souliss_T4n_Alarm);
    }
  }
}
