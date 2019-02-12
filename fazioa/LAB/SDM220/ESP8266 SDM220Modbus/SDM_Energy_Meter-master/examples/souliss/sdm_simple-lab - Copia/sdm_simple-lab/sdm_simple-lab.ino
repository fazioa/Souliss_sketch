/**************************************************************************
  Sketch: SDM220 mODBUS - VER.1 - Souliss - Static Configuration
  Author: Tonino Fazio

  ESP Core 2.3.0
  This example is only supported on ESP8266.

  parametri upload Arduino IDE:
  – ESP8266 Generic
  – Flash Mode: DOUT
  - Debug Port: disabled
  – Crystal Frequency: 26 MHz (non presente su IDE 1.6.12)
  – Flash Frequency 80 MHz
  – CPU frequency 160 MHz
  – Flash Size 1 MB (256K SPIFFS)

  Fonte: https://github.com/reaper7/SDM_Energy_Meter
***************************************************************************/

// RESET OGNI 20 MIN SE NON E' COLLEGATO AL GATEWAY
#define  VNET_RESETTIME_INSKETCH
#define VNET_RESETTIME      0x00042F7 // ((20 Min*60)*1000)/70ms = 17143 => 42F7
#define VNET_HARDRESET      ESP.reset()

#include "SoulissFramework.h"
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define HOSTNAME "souliss-SDM220-consumo"

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

//*************************************************************************
// Define the network configuration according to your router settingsuration according to your router settings
// and the other on the wireless oneless one
#define peer_address  0xAB19
#define myvNet_subnet 0xFF00
#define myvNet_supern 0xAB10
//*************************************************************************
#define SLOT_Watt     1
#define SLOT_Voltage  3
#define SLOT_Current  5

#define PIN_LED 13
boolean bLedState = false;

#include "SDM.h"                                                                //import SDM library

SDM sdm(Serial);

float iSDM220T_VOLTAGE = 0;
float iSDM220T_CURRENT = 0;
float iSDM220T_POWER = 0;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  //ACCENDE LED
  digitalWrite(PIN_LED, HIGH);
  delay(5000); // Ritardo di setup per permettere al router di effettuare il boot

  sdm.begin();                                                                  //initialize SDM communication
  
  Initialize();
  GetIPAddress();
  // Set network parameters
  SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  //SPEGNE LED
  digitalWrite(PIN_LED, LOW);

   Set_T57(SLOT_Watt);
  Set_Voltage(SLOT_Voltage); //T55
  Set_Current(SLOT_Current); //T56

  // Init the OTA
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.begin();

 
  NotificaTelegram();
}


void loop() {

  EXECUTEFAST() {
    UPDATEFAST();
    SHIFT_210ms(1) {
      iSDM220T_VOLTAGE = sdm.readVal(SDM220T_VOLTAGE);
    }
    SHIFT_210ms(1) {
      iSDM220T_CURRENT = sdm.readVal(SDM220T_CURRENT);
    }
    SHIFT_210ms(1) {
      iSDM220T_POWER = sdm.readVal(SDM220T_POWER);
    }
    
    FAST_1110ms() {
      ImportAnalog(SLOT_Watt, &iSDM220T_POWER);
      Logic_Power(SLOT_Watt);

      //scelgo di fare pubblicare il valore direttamente dal nodo invece che dal GW
      // float16(&output16, &iMedia);
      //  valByteArray[0] = C16TO8L(output16);
      //   valByteArray[1] = C16TO8H(output16);
      //  pblshdata(ENERGY_TOPIC, valByteArray, 2);

      ImportAnalog(SLOT_Voltage, &iSDM220T_VOLTAGE);
      Logic_Voltage(SLOT_Voltage);

      ImportAnalog(SLOT_Current, &iSDM220T_CURRENT);
      Logic_Current(SLOT_Current);


      bLedState = !bLedState;
      digitalWrite(PIN_LED, bLedState);
    }

    // Process the communication
    FAST_PeerComms();
  }

  EXECUTESLOW() {
    UPDATESLOW();
  }

  // Look for a new sketch to update over the air
  ArduinoOTA.handle();
}


//telegram
void sendToTelegram(String choose, String text ) {
  WiFiClientSecure botclient;
  UniversalTelegramBot bot(BOTTOKEN, botclient);
 // if (bot.sendMessage(choose, text, "")) {
  //  Serial.println("TELEGRAM Successfully sent");
 // }
  botclient.stop();
}

void NotificaTelegram() {
 // Serial.println("Invio messaggio su Telegram");
  sendToTelegram(CHAT_ID, "Nodo \"" + (String) HOSTNAME + "\" avviato" + " - IP: " + WiFi.localIP().toString());
}
//end telegram
