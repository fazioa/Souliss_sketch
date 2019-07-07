/**************************************************************************
  Interruttore Portoncino e Cancello e lettura dati misuratore energia SDM220

  Sketch: Porta - Cancello - GEEKCREIT 2 Channel modificato - VER.2 - Souliss - Static Configuration
  Author: Tonino Fazio

  ESP Core 2.4.2
  This example for Geekcreit 2 Channel
  PSF-B compatible ESP8266 built in


  parametri upload Arduino IDE 1.8.19.0:
  – ESP8266 Generic
  – Flash Mode: DOUT
  – Crystal Frequency: 26 MHz (non presente su IDE 1.6.12)
  – Flash Frequency 80 MHz
  – CPU frequency 160 MHz
  – Flash Size 1 MB (256K SPIFFS)
  - lwIP Variant: v1.4 Higher Bandwidth

Upload OTA ok il 04/03/2019


      This sketch require libreries:
  - UniversalTelegramBot (stable release)
  - ArduinoJson
***************************************************************************/
//#define SERIAL_DEBUG

// RESET OGNI 20 MIN SE NON E' COLLEGATO AL GATEWAY
#define  VNET_RESETTIME_INSKETCH
#define VNET_RESETTIME      0x00042F7 // ((20 Min*60)*1000)/70ms = 17143 => 42F7
#define VNET_HARDRESET      ESP.reset()

#define HOSTNAME "souliss-porta-cancello"

#include "SoulissFramework.h"
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>



#include <SoftwareSerial.h>                                                     //import SoftwareSerial library
#include "SDM.h"                                                                //import SDM library

#define SDM_UART_BAUD                     9600


SoftwareSerial swSerSDM(1, 3);                                               //hardware TX - RX, pins 1, 3

SDM sdm(swSerSDM, SDM_UART_BAUD, NOT_A_PIN);                                             //config SDM


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
#include <UniversalTelegramBot.h>  //

//*************************************************************************
// Define the network configuration according to your router settingsuration according to your router settings
// and the other on the wireless oneless one
#define peer_address  0xAB18
#define myvNet_subnet 0xFF00
#define myvNet_supern 0xAB10
//*************************************************************************

#define SLOT_REMOTE_CONTROLLER                          0
#define SLOT_APRIPORTA                  1
#define SLOT_POWER  2
#define SLOT_TOTAL_IMPORTED_ENERGY  4
#define SLOT_TOTAL_EXPORTED_ENERGY 6
#define T_WIFI_STRDB  8 //It takes 2 slots
#define T_WIFI_STR    10 //It takes 2 slots

#define PIN_OUTPUT_REMOTE_CONTROLLER_RELE1 12
#define PIN_OUTPUT_APRIPORTA_RELE2 14
#define PIN_LED 13

#define PIN_LEFT_BUTTON_0 0
#define PIN_MIDDLE_BUTTON_9 9

//Variable to Handle WiFio Signal
long rssi = 0;
int bars = 0;

float v;

boolean bLedState = false;
void setup()
{

  sdm.begin();

#ifdef SERIAL_DEBUG
  Serial.begin(9600);
  Serial.println("Node Starting");
#endif

#ifdef SERIAL_DEBUG
  Serial.println("Define Outputs Pins");
#endif
  // Define output pins
  pinMode(PIN_OUTPUT_REMOTE_CONTROLLER_RELE1, OUTPUT);    // Relè
  pinMode(PIN_OUTPUT_APRIPORTA_RELE2, OUTPUT);    // Relè
  pinMode(PIN_LED, OUTPUT);

#ifdef SERIAL_DEBUG
  Serial.println("Define Inputs Pins");
#endif

  pinMode(PIN_LEFT_BUTTON_0, INPUT_PULLUP);
  pinMode(PIN_MIDDLE_BUTTON_9, INPUT_PULLUP);

#ifdef SERIAL_DEBUG
  Serial.println("PIN_LED HIGH");
#endif
  digitalWrite(PIN_LED, HIGH);

  delay(5000); // Ritardo di setup per permettere al router di effettuare il boot

#ifdef SERIAL_DEBUG
  Serial.println("Initialize");
#endif
  Initialize();

#ifdef SERIAL_DEBUG
  Serial.println("GetIPAddress");
#endif
  GetIPAddress();

#ifdef SERIAL_DEBUG
  Serial.println("Ok");
#endif

#ifdef SERIAL_DEBUG
  Serial.println("PIN_LED LOW");
#endif
  digitalWrite(PIN_LED, LOW);
  SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  //*************************************************************************
  //*************************************************************************
  Set_PulseOutput(SLOT_REMOTE_CONTROLLER);
  Set_PulseOutput(SLOT_APRIPORTA);

  Set_Power(SLOT_POWER);
  Set_Power(SLOT_TOTAL_IMPORTED_ENERGY);
  Set_Power(SLOT_TOTAL_EXPORTED_ENERGY);

  Set_T51(T_WIFI_STRDB); //Imposto il tipico per contenere il segnale del Wifi in decibel
  Set_T51(T_WIFI_STR); //Imposto il tipico per contenere il segnale del Wifi in barre da 1 a 5



#ifdef SERIAL_DEBUG
  Serial.println("Arduino OTA Begin");
#endif
  // Init the OTA
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.begin();

#ifdef SERIAL_DEBUG
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("IP:  ");
  Serial.println(WiFi.localIP());
  Serial.println("Node Initialized");

#endif

  delay(1000);
  NotificaTelegram();

  //mOutput(SLOT_REMOTE_CONTROLLER) = Souliss_T1n_OnCoil;
  // mOutput(SLOT_APRIPORTA) = Souliss_T1n_OnCoil;
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_50ms() {
      LowDigIn(PIN_LEFT_BUTTON_0, Souliss_T1n_OnCmd, SLOT_REMOTE_CONTROLLER);
      LowDigIn(PIN_MIDDLE_BUTTON_9, Souliss_T1n_OnCmd, SLOT_APRIPORTA);
      
      Souliss_Logic_T57(memory_map, SLOT_POWER, 0.02, &data_changed);
      Souliss_Logic_T57(memory_map, SLOT_TOTAL_IMPORTED_ENERGY, 0.02, &data_changed);
      Souliss_Logic_T57(memory_map, SLOT_TOTAL_EXPORTED_ENERGY, 0.02, &data_changed);
    }

    FAST_710ms() {
      Logic_PulseOutput(SLOT_REMOTE_CONTROLLER);
      DigOut(PIN_OUTPUT_REMOTE_CONTROLLER_RELE1, Souliss_T1n_Coil, SLOT_REMOTE_CONTROLLER);


      Logic_PulseOutput(SLOT_APRIPORTA);
      DigOut(PIN_OUTPUT_APRIPORTA_RELE2, Souliss_T1n_Coil, SLOT_APRIPORTA);
    }

    FAST_1110ms() {
      bLedState = !bLedState;
      digitalWrite(PIN_LED, bLedState);
    }


    FAST_7110ms() {
      v = sdm.readVal(SDM220T_POWER);
#ifdef SERIAL_DEBUG
      Serial.print("Power: ");
      Serial.println(v);
#endif
      ImportAnalog(SLOT_POWER, &v);

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
      check_wifi_signal();
    }


    SLOW_110s() {
      v = sdm.readVal(SDM220T_IMPORT_ACTIVE_ENERGY);
#ifdef SERIAL_DEBUG
      Serial.print("Total Imported: ");
      Serial.println(v);
#endif
      ImportAnalog(SLOT_TOTAL_IMPORTED_ENERGY, &v);

      v = sdm.readVal(SDM220T_EXPORT_ACTIVE_ENERGY);
#ifdef SERIAL_DEBUG
      Serial.print("Total Exported: ");
      Serial.println(v);
#endif
      ImportAnalog(SLOT_TOTAL_EXPORTED_ENERGY, &v);
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
#ifdef SERIAL_DEBUG
  Serial.print("Try to send: ");
  Serial.println(text);

#endif
  WiFiClientSecure botclient;
  UniversalTelegramBot bot(BOTTOKEN, botclient);
  if (bot.sendMessage(choose, text, "")) {
    Serial.println("TELEGRAM Successfully sent");
  } else {
    Serial.println("TELEGRAM NOT Successfully sent");
  }

  botclient.stop();
}

void NotificaTelegram() {
  Serial.println("Invio messaggio su Telegram");
  sendToTelegram(CHAT_ID, "Nodo \"" + (String) HOSTNAME + "\" avviato" + " - IP: " + WiFi.localIP().toString());
}
//end telegram
