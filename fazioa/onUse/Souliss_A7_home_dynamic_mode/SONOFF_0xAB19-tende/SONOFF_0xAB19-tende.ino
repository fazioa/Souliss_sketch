/**************************************************************************
  Sketch: Tende - Souliss - Static Configuration
  Author: Tonino Fazio

  ESP Core 2.3.0
  Funziona su Sonoff 4CH R2

  parametri upload Arduino IDE:
  – ESP8285 Generic
  – Flash Size: 1M (no spiffs)
  – Crystal Frequency: 26 MHz (non presente su IDE 1.6.12)
  – CPU frequency 80 MHz
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

#define HOSTNAME "souliss-SONOFF-4CHR2-tende"

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
#define peer_address  0xAB19
#define myvNet_subnet 0xFF00
#define myvNet_supern 0xAB10
//*************************************************************************

#define SLOT_TENDA1 0
#define SLOT_TENDA2 1
#define SLOT_WIFI_STRDB  2 //It takes 2 slots
#define SLOT_WIFI_STR    4 //It takes 2 slots

#define PIN_BUTTON_L1 0
#define PIN_BUTTON_L2 9
#define PIN_BUTTON_L3 10
#define PIN_BUTTON_L4 14

#define PIN_LED_L1 12
#define PIN_LED_L2 5
#define PIN_LED_L3 4
#define PIN_LED_L4 15
#define PIN_LED_WIFI 13

#define Souliss_T2n_Timer_Val=Souliss_T2n_Timer_Off+0x14;

//Variable to Handle WiFio Signal
long rssi = 0;
int bars = 0;

boolean bLedState = false;
void setup()
{
#ifdef SERIAL_DEBUG
  Serial.begin(115200);
  Serial.println("Node Starting");
#endif

  // Definisco i pin di input
  pinMode(PIN_BUTTON_L1, INPUT_PULLUP);
  pinMode(PIN_BUTTON_L2, INPUT_PULLUP);
  pinMode(PIN_BUTTON_L3, INPUT_PULLUP);
  pinMode(PIN_BUTTON_L4, INPUT_PULLUP);

  pinMode(PIN_LED_L1, OUTPUT);
  pinMode(PIN_LED_L2, OUTPUT);
  pinMode(PIN_LED_L3, OUTPUT);
  pinMode(PIN_LED_L4, OUTPUT);
  pinMode(PIN_LED_WIFI, OUTPUT);

  delay(5000); // Ritardo di setup per permettere al router di effettuare il boot

  Initialize();
  GetIPAddress();
  digitalWrite(PIN_LED_WIFI, LOW);
  SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  //*************************************************************************
  //*************************************************************************
  Set_T22(SLOT_TENDA1);
  Set_T22(SLOT_TENDA2);

 mOutput(SLOT_TENDA1) = Souliss_T2n_StopCmd;
 mOutput(SLOT_TENDA2) = Souliss_T2n_StopCmd;
 
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

    FAST_50ms() {
      DigIn(PIN_BUTTON_L1, Souliss_T2n_OpenCmd_Local ,  SLOT_TENDA1);
      DigIn(PIN_BUTTON_L2, Souliss_T2n_CloseCmd_Local,  SLOT_TENDA1);
      DigIn(PIN_BUTTON_L3, Souliss_T2n_OpenCmd_Local,  SLOT_TENDA2);
      DigIn(PIN_BUTTON_L4, Souliss_T2n_CloseCmd_Local ,  SLOT_TENDA2);

      Souliss_Logic_T22(memory_map, SLOT_TENDA1, &data_changed, Souliss_T2n_Timer_Off+0x32);
      Souliss_Logic_T22(memory_map, SLOT_TENDA2, &data_changed, Souliss_T2n_Timer_Off+0x10);
      //Logic_Windows(SLOT_TENDA2);


      DigOut(PIN_LED_L1, Souliss_T2n_Coil_Open, SLOT_TENDA1);
      DigOut(PIN_LED_L2, Souliss_T2n_Coil_Close, SLOT_TENDA1);
      DigOut(PIN_LED_L3, Souliss_T2n_Coil_Open, SLOT_TENDA2);
      DigOut(PIN_LED_L4, Souliss_T2n_Coil_Close, SLOT_TENDA2);
    }

    FAST_1110ms() {
      bLedState = !bLedState;
      digitalWrite(PIN_LED_WIFI, bLedState);
    }

        // Define the hold time of the outputs, by default the timer hold the relays for 16 times, so:
        // 221x10x16ms that is about 35 seconds. Change the parameter inside FAST_x10ms() to change this time.
        FAST_x10ms(40) {                 
            Timer_T22(SLOT_TENDA1);
             Timer_T22(SLOT_TENDA2);
        }
    

    FAST_21110ms() {
      //Processa le logiche per il segnale WiFi
      Read_T51(SLOT_WIFI_STRDB);
      Read_T51(SLOT_WIFI_STR);
    }

    FAST_91110ms() {
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
