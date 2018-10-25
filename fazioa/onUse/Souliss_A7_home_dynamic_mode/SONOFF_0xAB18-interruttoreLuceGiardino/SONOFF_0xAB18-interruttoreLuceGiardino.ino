/**************************************************************************
  Interruttore luce giardino

  Sketch: POWER SOCKET - VER.2 - Souliss - Static Configuration
  Author: Tonino Fazio

  ESP Core 2.3.0
  This example for Geekcreit 2 Channel
  PSF-B compatible ESP8266 built in


  parametri upload Arduino IDE:
  – ESP8266 Generic
  – Flash Mode: DOUT
  – Crystal Frequency: 26 MHz (non presente su IDE 1.6.12)
  – Flash Frequency 80 MHz
  – CPU frequency 80 MHz
  – Flash Size 1 MB (256K SPIFFS)

      This sketch require libreries:
  - UniversalTelegramBot (stable release)
  - ArduinoJson
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

#define HOSTNAME "souliss-GEEKCREIT_2CH-forno-giardino"

#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/IPBroadcast.h"

#include <UniversalTelegramBot.h>

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
#include "topics.h"
uint8_t mypayload_len = 0;
U8 mypayload;

//*************************************************************************
// Define the network configuration according to your router settingsuration according to your router settings
// and the other on the wireless oneless one
#define peer_address  0xAB18
#define myvNet_subnet 0xFF00
#define myvNet_supern 0xAB10
//*************************************************************************

#define SLOT_RELE1 0
#define SLOT_RELE2 1
#define T_WIFI_STRDB  2 //It takes 2 slots
#define T_WIFI_STR    4 //It takes 2 slots

//#define PIN_POWERSOCKET 12
#define PIN_RELE1 12
#define PIN_RELE2 14
#define PIN_LED 13

#define PIN_LEFT_BUTTON_0 0
#define PIN_MIDDLE_BUTTON_9 9

//Variable to Handle WiFio Signal
long rssi = 0;
int bars = 0;


int iPIN = 1;

boolean bLedState = false;
void setup()
{
#ifdef SERIAL_DEBUG
  Serial.begin(9600);
  Serial.println("Node Starting");
#endif

  // Define output pins
  pinMode(PIN_RELE1, OUTPUT);    // Relè
  pinMode(PIN_RELE2, OUTPUT);    // Relè
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_LEFT_BUTTON_0, INPUT_PULLUP);
  pinMode(PIN_MIDDLE_BUTTON_9, INPUT_PULLUP);


  digitalWrite(PIN_LED, HIGH);

  //delay(30000); // Ritardo di setup per permettere al router di effettuare il boot

  Initialize();
  GetIPAddress();
  digitalWrite(PIN_LED, LOW);
  SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  //*************************************************************************
  //*************************************************************************
  Set_SimpleLight(SLOT_RELE1);
  Set_SimpleLight(SLOT_RELE2);

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

  NotificaTelegram();

  mOutput(SLOT_RELE1) = Souliss_T1n_OnCoil;
  mOutput(SLOT_RELE2) = Souliss_T1n_OnCoil;
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_50ms() {
      DigIn(PIN_LEFT_BUTTON_0, Souliss_T1n_ToggleCmd, SLOT_RELE1);
      Logic_SimpleLight(SLOT_RELE1);
      DigOut(PIN_RELE1, Souliss_T1n_Coil, SLOT_RELE1);

      DigIn(PIN_MIDDLE_BUTTON_9, Souliss_T1n_ToggleCmd, SLOT_RELE2);
      Logic_SimpleLight(SLOT_RELE2);
      DigOut(PIN_RELE2, Souliss_T1n_Coil, SLOT_RELE2);
    }

    FAST_90ms() {
      subcription_ON_OFF();
    }

    FAST_1110ms() {
      bLedState = !bLedState;
      digitalWrite(PIN_LED, bLedState);
    }

    FAST_2110ms() {
      //Processa le logiche per il segnale WiFi
      Read_T51(T_WIFI_STRDB);
      Read_T51(T_WIFI_STR);
    }

    FAST_PeerComms();
  }

  EXECUTESLOW() {
    UPDATESLOW();
    SLOW_10s() {  // Process the timer every 10 seconds
      Timer_SimpleLight(SLOT_RELE1);
      Timer_SimpleLight(SLOT_RELE2);
      check_wifi_signal();
    }

    SLOW_50s() {
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
    Serial.println("TELEGRAM Successfully sent");
  }
  botclient.stop();
}

void NotificaTelegram() {
  Serial.println("Invio messaggio su Telegram");
  sendToTelegram(CHAT_ID, "Nodo \"" + (String) HOSTNAME + "\" avviato" + " - IP: " + WiFi.localIP().toString());
  Serial.print(" ...ok");
}
//end telegram


void subcription_ON_OFF() {
  if (sbscrbdata(GIARDINO_ONOFF, &mypayload, &mypayload_len)) {
    if (mypayload == LIGHT_ON)
      mOutput(SLOT_RELE1) = Souliss_T1n_OnCoil;
    else if (mypayload == LIGHT_OFF)
      mOutput(SLOT_RELE1) = Souliss_T1n_OffCoil;
  }

  if (sbscrbdata(FORNO_ONOFF, &mypayload, &mypayload_len)) {
    if (mypayload == LIGHT_ON)
      mOutput(SLOT_RELE2) = Souliss_T1n_OnCoil;
    else if (mypayload == LIGHT_OFF)
      mOutput(SLOT_RELE2) = Souliss_T1n_OffCoil;
  }

}
