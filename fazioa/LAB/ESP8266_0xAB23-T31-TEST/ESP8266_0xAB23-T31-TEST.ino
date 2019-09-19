/**************************************************************************
  Sketch: Tende - Souliss - Static Configuration
  Author: Tonino Fazio

  ESP Core 2.3.0

  parametri upload Arduino IDE:
  – ESP8285 Generic
  – Flash Size: 1M (no spiffs)
  – Crystal Frequency: 26 MHz (non presente su IDE 1.6.12)
  – CPU frequency 80 MHz
 ***************************************************************************/
#define SERIAL_DEBUG

// RESET OGNI 20 MIN SE NON E' COLLEGATO AL GATEWAY
#define  VNET_RESETTIME_INSKETCH
#define VNET_RESETTIME      0x00042F7 // ((20 Min*60)*1000)/70ms = 17143 => 42F7
#define VNET_HARDRESET      ESP.reset()

#include "SoulissFramework.h"
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define HOSTNAME "souliss-ESP8266-T31-TEST"

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
#define peer_address  0xAB23
#define myvNet_subnet 0xFF00
#define myvNet_supern 0xAB10
//*************************************************************************

#define SLOT_T31 0
#define SLOT_WIFI_STRDB  5 //It takes 2 slots
#define SLOT_WIFI_STR    7 //It takes 2 slots


float temp_setpoint, temp_ambiente;
byte stato;
#define PIN_LED_WIFI 13

//Variable to Handle WiFio Signal
long rssi = 0;
int bars = 0;
int T_WIFI_STRDB = 0;  //It takes 2 slots
int T_WIFI_STR  =  0 ; //It takes 2 slots

boolean bLedState = false;
void setup()
{
#ifdef SERIAL_DEBUG
  Serial.begin(115200);
  Serial.println("Node Starting");
#endif

  // Definisco la funzione dei pin
  pinMode(PIN_LED_WIFI, OUTPUT);

  delay(1000); // Ritardo di setup per permettere al router di effettuare il boot

  Initialize();
  GetIPAddress();
  digitalWrite(PIN_LED_WIFI, LOW);
  SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  //*************************************************************************
  //*************************************************************************
  Set_Thermostat(SLOT_T31);

  Set_T51(SLOT_WIFI_STRDB); //Imposto il tipico per contenere il segnale del Wifi in decibel
  Set_T51(SLOT_WIFI_STR); //Imposto il tipico per contenere il segnale del Wifi in barre da 1 a 5

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
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_110ms() {
      Logic_Thermostat(SLOT_T31);
    }



    FAST_1110ms() {
      bLedState = !bLedState;
      digitalWrite(PIN_LED_WIFI, bLedState);
    }

    FAST_2110ms()  {
//stato sistema
      stato = mOutput(SLOT_T31);
            
      //temperatura rilevata
      temp_ambiente = mOutputAsFloat(SLOT_T31+1);
            
      //temperatura setpoint
      temp_setpoint = mOutputAsFloat(SLOT_T31+3);
    //  ImportAnalog(SLOT_T31, &temp_ambiente);

      Serial.print("STATO: ");
      Serial.print(stato);
      Serial.print(" - ");
      Serial.print("TEMP: ");
      Serial.print(temp_ambiente);
      Serial.print(" - ");
      Serial.print("SETPOINT: ");
      Serial.println(temp_setpoint);
    }


    FAST_21110ms() {
      //Processa le logiche per il segnale WiFi
      Read_T51(T_WIFI_STRDB);
      Read_T51(T_WIFI_STR);
    }
    
    FAST_PeerComms();
  }

  EXECUTESLOW() {
    UPDATESLOW();
    SLOW_10s() {  // Process the timer every 10 seconds

    }
    SLOW_50s() {
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
  ImportAnalog(SLOT_WIFI_STRDB, &f_rssi);
  ImportAnalog(SLOT_WIFI_STR, &f_bars);

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
