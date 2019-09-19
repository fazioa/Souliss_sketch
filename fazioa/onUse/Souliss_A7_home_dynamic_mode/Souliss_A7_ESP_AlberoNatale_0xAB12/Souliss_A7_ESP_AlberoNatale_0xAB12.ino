#define HOST_NAME_INSKETCH
#define HOST_NAME "Souliss-Power_Socket-v1-dhcp-staticVNETaddress"
//#define SERIAL_DEBUG
/**************************************************************************
  Sketch: POWER SOCKET - VER.2 - Souliss - Web Configuration
  Author: Tonino Fazio
  
 
  ESP Core 2.4.2
  This example is only supported on ESP8266.

  Arduino IDE 1.8.8

Compile:
Generic ESP8266 Module
CPU Frequency: 80MHz
Crystal Frequency: 26MHz
Flash Size: 1Mb (no SPIFFS) 
Flash Mode: DIO
Flash Frequency 40 MHz
Other: Default
***************************************************************************/

// RESET OGNI 20 MIN SE NON E' COLLEGATO AL GATEWAY
#define  VNET_RESETTIME_INSKETCH
#define VNET_RESETTIME      0x00042F7 // ((20 Min*60)*1000)/70ms = 17143 => 42F7
#define VNET_HARDRESET      ESP.reset()

#include "SoulissFramework.h"
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
//#include <ArduinoOTA.h>

#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#define HOSTNAME "souliss-ESP-Albero-di-Natale"
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

#include <UniversalTelegramBot.h>
#include "Souliss.h"


//*************************************************************************
// Define the network configuration according to your router settingsuration according to your router settings
// and the other on the wireless oneless one
#define peer_address  0xAB12
#define myvNet_subnet 0xFF00
#define myvNet_supern 0xAB10
//*************************************************************************

#define SLOT_POWERSOCKET 0
#define PIN_POWERSOCKET 14

//Variable to Handle WiFio Signal
long rssi = 0;
int bars = 0;
#define T_WIFI_STRDB  1 //It takes 2 slots
#define T_WIFI_STR    3 //It takes 2 slots

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
void setup()
{
  #ifdef SERIAL_DEBUG
  Serial.begin(115200);
  Serial.println("Node Starting");
#endif
  //delay 10 seconds
  delay(10000);
  Initialize();
  GetIPAddress();

  SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  //*************************************************************************
  //*************************************************************************
  Set_SimpleLight(SLOT_POWERSOCKET);
Set_T51(T_WIFI_STRDB); //Imposto il tipico per contenere il segnale del Wifi in decibel
  Set_T51(T_WIFI_STR); //Imposto il tipico per contenere il segnale del Wifi in barre da 1 a 5

  // Define output pins
  pinMode(PIN_POWERSOCKET, OUTPUT);    // Relè

  // Init the OTA
//  ArduinoOTA.setHostname(HOSTNAME);
 // ArduinoOTA.begin();
    NotificaTelegram();

      MDNS.begin(HOSTNAME);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
}

void loop()
{
    httpServer.handleClient();
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_50ms() {
      Logic_SimpleLight(SLOT_POWERSOCKET);
      DigOut(PIN_POWERSOCKET, Souliss_T1n_Coil, SLOT_POWERSOCKET);
    }

       FAST_11110ms() {
      //Processa le logiche per il segnale WiFi
      Read_T51(T_WIFI_STRDB);
      Read_T51(T_WIFI_STR);
    }
    
    FAST_PeerComms();
  }

  EXECUTESLOW() {
    UPDATESLOW();

     SLOW_50s() {  // Process the timer every 10 seconds
      Timer_SimpleLight(SLOT_POWERSOCKET);
      check_wifi_signal();
    }
       SLOW_110s() {
      //verifica ogni 90 sec (fast 91110) che la ESP sia collegata alla rete Wifi (5 tentativi al 6^fa hard reset)
      int tent = 0;
#ifdef SERIAL_DEBUG
      Serial.println("Verifico connessione");
#endif
      while ((WiFi.status() != WL_CONNECTED) && tent < 4)
      {
        WiFi.disconnect();
        WiFi.mode(WIFI_STA);
        WiFi.begin(WiFi_SSID , WiFi_Password);
        int ritardo = 0;
        while ((WiFi.status() != WL_CONNECTED) && ritardo < 20)
        {
          delay(500);
          ritardo += 1;
#ifdef SERIAL_DEBUG
          Serial.println(ritardo);
#endif
        }

        if (WiFi.status() != WL_CONNECTED )
          delay(2000);
        tent += 1;
      }
#ifdef SERIAL_DEBUG
      if (tent > 4)Serial.println("tentativo non riuscito");
#endif
    }
  }

  // Look for a new sketch to update over the air
//  ArduinoOTA.handle();
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
  Serial.print("wifi rssi: ");
  Serial.println(f_rssi);
  Serial.print("wifi bars: ");
  Serial.println(f_bars);
#endif
}

//telegram
void sendToTelegram(String choose, String text ) {
  WiFiClientSecure botclient;
  UniversalTelegramBot bot(BOTTOKEN, botclient);
  if (bot.sendMessage(choose, text, "")) {
#ifdef SERIAL_DEBUG
    Serial.println("TELEGRAM Successfully sent");
#endif
  }
  botclient.stop();
}

void NotificaTelegram() {
#ifdef SERIAL_DEBUG
  Serial.println("Invio messaggio su Telegram");
#endif
  sendToTelegram(CHAT_ID, "Nodo \"" + (String) HOSTNAME + "\" avviato" + " - IP: " + WiFi.localIP().toString());
#ifdef SERIAL_DEBUG
  Serial.print(" ...ok");
#endif
}
//end telegra
