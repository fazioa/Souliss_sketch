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

//Adafruit MQTT Library
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "mqtt_conf.h"

#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/IPBroadcast.h"

#include "credenziali.h"
// **** creare un file di testo chiamato credenziali.h con il seguente contenuto personalizzato ****
//#define WIFICONF_INSKETCH
//#define WiFi_SSID               "SSID"
//#define WiFi_Password           "PWD"

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Include framework code and libraries
#include "Homie.h"
#include "Souliss.h"

//*************************************************************************
// Define the network configuration according to your router settingsuration according to your router settings
// and the other on the wireless oneless one
#define peer_address  0xAB20
#define myvNet_subnet 0xFF00
#define myvNet_supern 0xAB10
//*************************************************************************

#define SLOT_POWERSOCKET1 0
#define SLOT_POWERSOCKET2 1
#define SLOT_POWERSOCKET3 2
#define SLOT_POWERSOCKET4 3

#define PIN_POWERSOCKET 12
#define PIN_BUTTON_0 0
#define PIN_BUTTON_14 14

//Variable to Handle WiFio Signal
long rssi = 0;
int bars = 0;
#define T_WIFI_STRDB  4 //It takes 2 slots
#define T_WIFI_STR    6 //It takes 2 slots

boolean bLedState = false;
void setup()
{

#ifdef SERIAL_DEBUG
  Serial.begin(115200);
  Serial.println("\nNode Starting");
#endif

  // Define output pins
  pinMode(PIN_POWERSOCKET, OUTPUT);    // Relè
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_BUTTON_14, INPUT_PULLUP);

  digitalWrite(LED_BUILTIN, HIGH);

  GetIPAddress();
  Initialize();

  digitalWrite(LED_BUILTIN, LOW);
  SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  mOutput(SLOT_POWERSOCKET1) = Souliss_T1n_OnCoil;

  Set_T51(T_WIFI_STRDB); //Imposto il tipico per contenere il segnale del Wifi in decibel
  Set_T51(T_WIFI_STR); //Imposto il tipico per contenere il segnale del Wifi in barre da 1 a 5

#ifdef SERIAL_DEBUG
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("IP:  ");
  Serial.println(WiFi.localIP());
  Serial.print("Subnet:  ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.println("Node Initialized");
#endif


  // Setup MQTT subscription for onoff feed.
  // DA VEDERE MEGLIO
  //mqtt.subscribe(&MQTTrelay0_Read);

  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)


}

int i = 0;
void loop()
{



  EXECUTEFAST() {
    UPDATEFAST();

    FAST_2110ms() {
      //Processa le logiche per il segnale WiFi
      Read_T51(T_WIFI_STRDB);
      Read_T51(T_WIFI_STR);
    }



    FAST_50ms() {
      DigIn2State(PIN_BUTTON_14, Souliss_T1n_ToggleCmd, Souliss_T1n_ToggleCmd, SLOT_POWERSOCKET1);
      Logic_SimpleLight(SLOT_POWERSOCKET1);
      DigOut(PIN_POWERSOCKET, Souliss_T1n_Coil, SLOT_POWERSOCKET1);
    }

    FAST_1110ms() {
      bLedState = !bLedState;
      digitalWrite(LED_BUILTIN, bLedState);
    }

    FAST_2110ms() {
      //Processa le logiche per il segnale WiFi
      Read_T51(T_WIFI_STRDB);
      Read_T51(T_WIFI_STR);

    }

    FAST_91110ms() {
      //verifica ogni 90 sec (fast 91110) che la ESP sia collegata alla rete Wifi (5 tentativi al 6^fa hard reset)
      int tent = 0;

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
      Timer_SimpleLight(SLOT_POWERSOCKET1);
    }
    SLOW_70s() {
      Homie_init();
      //*************************************************************************
      // MQTT HOMIE
      Homie_declare_nodes("lucepianoterra,lucepianoprimo,Piantana1,Piantana2"); //ELENCO DEI TIPICI SEPARATI DA VIRGOLA, SENZA SPAZI
      Set_SimpleLight_mqtt_homie(SLOT_POWERSOCKET1, "lucepianoterra", "Terra");
      Set_SimpleLight_mqtt_homie(SLOT_POWERSOCKET2, "lucepianoprimo", "Primo");
      Set_SimpleLight_mqtt_homie(SLOT_POWERSOCKET3, "Piantana1", "Piantana Soggiorno");
      Set_SimpleLight_mqtt_homie(SLOT_POWERSOCKET4, "Piantana2", "Piantana Camera");
      //PUBLISH MESSAGE MQTT - DEVICE READY
      Homie_device_ready();
    }
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
  Serial.print("wifi rssi: ");
  Serial.println(f_rssi);
  Serial.print("wifi bars: ");
  Serial.println(f_bars);
#endif
}
