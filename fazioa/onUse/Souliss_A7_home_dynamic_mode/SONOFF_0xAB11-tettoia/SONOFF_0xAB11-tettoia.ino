/**************************************************************************
  Interruttore luce tettoia
  Invia messaggi di accensione e spegnimento al luce tettoia e luce muro e forno
  Funziona con switch collegato al pin 14
  Singolo click comanda rele onboard
  Dobbio click invia messaggio accensiona a REMOTE_ADDRESS 0xAB17 SLOT 0
  Triplo click invia messaggio accensiona a REMOTE_ADDRESS 0xAB17 SLOT 0
  Pressione lunga invia entrambi i messaggi e spegne/accende rele onboard

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
//#define SERIAL_DEBUG

// RESET OGNI 20 MIN SE NON E' COLLEGATO AL GATEWAY
#define  VNET_RESETTIME_INSKETCH
#define VNET_RESETTIME      0x00042F7 // ((20 Min*60)*1000)/70ms = 17143 => 42F7
#define VNET_HARDRESET      ESP.reset()


// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
//#include <ArduinoOTA.h>
#include <ESP8266HTTPUpdateServer.h>

#define HOSTNAME "souliss-SONOFF-tettoia-cmd-forno-muro"

// Configure the framework
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

#include "ClickButton.h"
#include <UniversalTelegramBot.h>
#include "Souliss.h"

//*************************************************************************
// Define the network configuration according to your router settingsuration according to your router settings
// and the other on the wireless oneless one
#define peer_address  0xAB11
#define myvNet_subnet 0xFF00
#define myvNet_supern    0xAB10 //gateway
//*************************************************************************

#define SLOT_RELAY 0
#define SLOT_LOCALE_X_RELAY_REMOTO_0  1
#define SLOT_LOCALE_X_RELAY_REMOTO_1  2

#define SLOT_RELAY_REMOTO_0  0
#define SLOT_RELAY_REMOTO_1  1

#define REMOTE_ADDRESS 0xAB17

#define PIN_RELAY 12
#define PIN_LED 13
#define PIN_BUTTON_0 0
#define PIN_BUTTON_14 14

ClickButton button1(PIN_BUTTON_14, LOW, CLICKBTN_PULLUP);


//Variable to Handle WiFio Signal
long rssi = 0;
int bars = 0;
#define T_WIFI_STRDB  1 //It takes 2 slots
#define T_WIFI_STR    3 //It takes 2 slots

boolean bLedState = false;

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

void setup()
{
#ifdef SERIAL_DEBUG
  Serial.begin(115200);
  Serial.println("Node Starting");
#endif

  // Define output pins
  pinMode(PIN_RELAY, OUTPUT);    // Relè
 // pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUTTON_14, INPUT_PULLUP);
  digitalWrite(PIN_LED, HIGH);

  //delay 11 seconds
  delay(11000);
  digitalWrite(PIN_LED, LOW);
  Initialize();

  digitalWrite(PIN_LED, HIGH);
  // Connect to the WiFi network and get an address from DHCP
  GetIPAddress();
  digitalWrite(PIN_LED, LOW);
  // This is the vNet address for this node, used to communicate with other
  // nodes in your Souliss network
  SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  Set_SimpleLight(SLOT_RELAY);
  Set_SimpleLight(SLOT_LOCALE_X_RELAY_REMOTO_0);
  Set_SimpleLight(SLOT_LOCALE_X_RELAY_REMOTO_1);

  Set_T51(T_WIFI_STRDB); //Imposto il tipico per contenere il segnale del Wifi in decibel
  Set_T51(T_WIFI_STR); //Imposto il tipico per contenere il segnale del Wifi in barre da 1 a 5

  // Init the OTA
  // ArduinoOTA.setHostname(HOSTNAME);
  // ArduinoOTA.begin();

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

  MDNS.begin(HOSTNAME);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  
 button1.debounceTime   = 20;   // Debounce timer in ms
  button1.multiclickTime = 250;  // Time limit for multi clicks
  button1.longClickTime  = 1000; // time until "held-down clicks" register
}

void loop()
{
  httpServer.handleClient();

  // Update button state
  button1.Update();

if(button1.clicks == 1) buttonActions(1);
else if(button1.clicks == 2) buttonActions(2);
else if(button1.clicks == 3) buttonActions(3);
//long press
else if(button1.clicks <= -1) buttonActions(-1);


  EXECUTEFAST() {
    UPDATEFAST();

    FAST_1110ms() {
      //led attività
      bLedState = !bLedState;
      digitalWrite(PIN_LED, bLedState);
    }

    SHIFT_11110ms(10) {
      //recupera i valore su REMOTE_ADDRESS e li copia sugli slot locali
      PullData(REMOTE_ADDRESS, SLOT_LOCALE_X_RELAY_REMOTO_1, SLOT_RELAY_REMOTO_1, 1);
    }
    SHIFT_11110ms(100) {
      PullData(REMOTE_ADDRESS, SLOT_LOCALE_X_RELAY_REMOTO_0, SLOT_RELAY_REMOTO_0, 1);
    }

    FAST_21110ms() {
      //Processa le logiche per il segnale WiFi
      Read_T51(T_WIFI_STRDB);
      Read_T51(T_WIFI_STR);
    }

    FAST_50ms() {
      //il pulsante di ingresso è gestito dalla libreria multipress, non da Souliss, quindi elimino la riga DigIn
      //DigIn2State(PIN_SWITCH, Souliss_T1n_ToggleCmd, Souliss_T1n_ToggleCmd, SLOT_RELAY);
     // DigIn(PIN_BUTTON_14, Souliss_T1n_ToggleCmd, SLOT_RELAY);

      Logic_SimpleLight(SLOT_RELAY);
      DigOut(PIN_RELAY, Souliss_T1n_Coil, SLOT_RELAY);
    }

    FAST_PeerComms();
  }


  EXECUTESLOW() {
    UPDATESLOW();
    SLOW_50s() {  // Process the timer every 10 seconds
      Timer_SimpleLight(SLOT_RELAY);
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
//end telegram

void buttonActions(const int value)  // example of registering Multi-Presses
{
#ifdef SERIAL_DEBUG
  Serial.print(F("Button:\t"));
#endif
  switch (value)
  {
    case -1:
      //Long Press
      longPress();
      break;
    case 1:
      //One Press
      click();
      break;
    case 2:
      //Two Presses
      doubleClick();
      break;
    case 3:
      //Three Presses
      tripleClick();
      break;
    default:
#ifdef SERIAL_DEBUG
      Serial.println(F("Whole Lotta Presses"));
#endif
      break;
  }
}

//========================= S I N G O L O   C L I C K ============================================
void click() {
#ifdef SERIAL_DEBUG
  Serial.println("Single click" );
#endif
  mInput(SLOT_RELAY) = Souliss_T1n_ToggleCmd;
} // click

//========================= D O P P I O   C L I C K ============================================
void doubleClick() {
#ifdef SERIAL_DEBUG
  Serial.print("Doppio click - " );
  Serial.println("Muro");
#endif
  Send(REMOTE_ADDRESS, SLOT_RELAY_REMOTO_0, Souliss_T1n_ToggleCmd);

} // doubleclick

//========================= T R I P L O   C L I C K ============================================
// this function will be called when the button was pressed 3 times in a short timeframe.
void tripleClick() {
#ifdef SERIAL_DEBUG
  Serial.print("Triplo click - " );
  Serial.println("Forno");
#endif
  Send(REMOTE_ADDRESS, SLOT_RELAY_REMOTO_1, Souliss_T1n_ToggleCmd);
} // tripleclick

//========================= L O N G   C L I C K ============================================
void longPress() {

  //se almeno una delle luci è accesa allora spongo tutto, altrimenti accendo tutto
  if ((mInput(SLOT_LOCALE_X_RELAY_REMOTO_0) == Souliss_T1n_Coil) || (mInput(SLOT_LOCALE_X_RELAY_REMOTO_1) == Souliss_T1n_Coil) || (mOutput(SLOT_RELAY) == Souliss_T1n_OnCoil)) {
#ifdef SERIAL_DEBUG
    Serial.println("LongPress - Qualche luce è accesa, dunque spengo tutto" );
#endif
    Send(REMOTE_ADDRESS, SLOT_RELAY_REMOTO_0, Souliss_T1n_OffCmd);
    Send(REMOTE_ADDRESS, SLOT_RELAY_REMOTO_1, Souliss_T1n_OffCmd);
    mOutput(SLOT_RELAY) = Souliss_T1n_OffCoil;
  } else {
#ifdef SERIAL_DEBUG
    Serial.println("LongPress - Tutte le luci sono spente, dunque accendo tutto" );
#endif
    Send(REMOTE_ADDRESS, SLOT_RELAY_REMOTO_0, Souliss_T1n_OnCmd);
    Send(REMOTE_ADDRESS, SLOT_RELAY_REMOTO_1, Souliss_T1n_OnCmd);
    mOutput(SLOT_RELAY) = Souliss_T1n_OnCoil ;
  }
} // tripleclick
