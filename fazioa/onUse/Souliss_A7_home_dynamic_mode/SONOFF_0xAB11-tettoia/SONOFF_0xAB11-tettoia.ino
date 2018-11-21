/**************************************************************************
  Interruttore luce tettoia
  Invia messaggi di accensione e spegnimento al nodo forno giardino
  Funziona con switch collegato al pin 14

  Sketch: POWER SOCKET - VER.2 - Souliss - Static Configuration
  Author: Tonino Fazio

  ESP Core 2.4.2
  This example is only supported on ESP8266.

  parametri upload Arduino IDE:
  – ESP8266 Generic
  – Flash Mode: DIO
  – Crystal Frequency: 26 MHz (non presente su IDE 1.6.12)
  – Flash Frequency 80 MHz
  – CPU frequency 80 MHz
  – Flash Size 1 MB (256K SPIFFS)
***************************************************************************/
#define SERIAL_DEBUG

// RESET OGNI 20 MIN SE NON E' COLLEGATO AL GATEWAY
#define  VNET_RESETTIME_INSKETCH
#define VNET_RESETTIME      0x00042F7 // ((20 Min*60)*1000)/70ms = 17143 => 42F7
#define VNET_HARDRESET      ESP.reset()


// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define HOSTNAME "souliss-SONOFF-tettoia (msg forno-giardino)"

// Configure the framework
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

#include "MultiPress.h"
#include "Souliss.h"
#include "topics.h"
uint8_t mypayload_len = 0;
U8 mypayload;

//*************************************************************************
// Define the network configuration according to your router settingsuration according to your router settings
// and the other on the wireless oneless one
#define peer_address  0xAB12
#define myvNet_subnet 0xFF00
#define myvNet_supern    0xAB10 //gateway
//*************************************************************************

#define SLOT_RELAY 0

#define SLOT_LOCALE_X_RELAY_REMOTO_0  1
#define SLOT_RELAY_REMOTO_0  0
#define SLOT_LOCALE_X_RELAY_REMOTO_1  2
#define SLOT_RELAY_REMOTO_1  1
#define REMOTE_ADDRESS 0xAB17

#define PIN_RELAY 12
#define PIN_LED 13
#define PIN_BUTTON_0 0
#define PIN_BUTTON_14 14
#define BUTTON_TEST 3
#define LEDPIN 13
//Variable to Handle WiFio Signal
long rssi = 0;
int bars = 0;
#define T_WIFI_STRDB  1 //It takes 2 slots
#define T_WIFI_STR    3 //It takes 2 slots

const boolean ON = true;
const boolean OFF = false;
boolean bLedState = false;

void B_ButtonActions(const int value);
SimplePress pushButtonSwitches[] = {
  {PIN_BUTTON_14, 700, B_ButtonActions}
};

class CoilState {
  public:
    boolean toDoCmd = true;
    U8 stato;
};

CoilState coilTettoia ;
CoilState coilForno;
CoilState coilMuro ;
CoilState coilsState;

void setup()
{
#ifdef SERIAL_DEBUG
  Serial.begin(115200);
  Serial.println("Node Starting");
#endif

  Serial.println(SimplePress::getCount());
  SimplePress::beginAll();
  SimplePress::setDebounceAll(200);

  // Define output pins
  pinMode(PIN_RELAY, OUTPUT);    // Relè
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUTTON_14, INPUT_PULLUP);
  pinMode(BUTTON_TEST, INPUT_PULLUP);

  //delay 11 seconds
  // delay(11000);

  Initialize();

  // Connect to the WiFi network and get an address from DHCP
  GetIPAddress();
  digitalWrite(PIN_LED, LOW);
  // This is the vNet address for this node, used to communicate with other
  // nodes in your Souliss network
  SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  Set_SimpleLight(SLOT_RELAY);
  mOutput(SLOT_RELAY) = Souliss_T1n_OnCoil;


  Set_SimpleLight(SLOT_LOCALE_X_RELAY_REMOTO_0);
  Set_SimpleLight(SLOT_LOCALE_X_RELAY_REMOTO_1);

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


}

void loop()
{
  SimplePress::update();

  EXECUTEFAST() {
    UPDATEFAST();

    FAST_1110ms() {
      //led attività
      bLedState = !bLedState;
      digitalWrite(PIN_LED, bLedState);
    }

    SHIFT_110ms(0) {
      //    if (coilMuro) {      //invia al nodo forno-giardino il messaggio di accensione

      //  publishMuroState_ON_OFF(ON);
      //    } else {
      //invia al nodo forno-giardino il messaggio di spegnimento
      // publishMuroState_ON_OFF(OFF);
      //     }
    }
    SHIFT_110ms(1) {
      //  if (coilForno) {      //invia al nodo forno-giardino il messaggio di accensione
      // publishFornoState_ON_OFF(ON);
      //  } else {
      //invia al nodo forno-giardino il messaggio di spegnimento
      //publishFornoState_ON_OFF(OFF);
      //  }
    }


    FAST_11110ms() {
      //Processa le logiche per il segnale WiFi
      Read_T51(T_WIFI_STRDB);
      Read_T51(T_WIFI_STR);


    }

    FAST_50ms() {
      //DigIn2State(PIN_SWITCH, Souliss_T1n_ToggleCmd, Souliss_T1n_ToggleCmd, SLOT_RELAY);
      // DigIn(PIN_BUTTON_14, Souliss_T1n_ToggleCmd, SLOT_RELAY);
      Logic_SimpleLight(SLOT_RELAY);
      // DigOut(coilTettoia, Souliss_T1n_Coil, SLOT_RELAY);

      //RemoteDigIn(BUTTON_TEST, Souliss_T1n_ToggleCmd, 0xAB17, 0);
      // Send(0xAB17, 0, Souliss_T1n_OnCmd);
      //      - leggere lo stato del nodo remoto
      //      - confronto se comando locale sia diverso (ignora se è indeterminato)
      //      - se è diverso invio comando remoto
      //      - cambio stato comando locale


      //recupera i valore su REMOTE_ADDRESS e li copia sugli slot locali
      PullData(REMOTE_ADDRESS, SLOT_LOCALE_X_RELAY_REMOTO_1, SLOT_RELAY_REMOTO_1, 1);
      PullData(REMOTE_ADDRESS, SLOT_LOCALE_X_RELAY_REMOTO_0, SLOT_RELAY_REMOTO_0, 1);

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
      Timer_SimpleLight(SLOT_RELAY);
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



boolean bGiardinoState = OFF;
//void publishMuroState_ON_OFF(boolean _bState) {
//  if (_bState != bGiardinoState)
//  { if (_bState)
//      pblshdata(GIARDINO_ONOFF, &LIGHT_ON, 1);
//    else
//      pblshdata(GIARDINO_ONOFF, &LIGHT_OFF, 1);
//
//    bGiardinoState = _bState;
//  }
//}
//
//boolean bFornoState = OFF;
//void publishFornoState_ON_OFF(boolean _bState) {
//  if (_bState != bFornoState)
//  { if (_bState)
//      pblshdata(FORNO_ONOFF, &LIGHT_ON, 1);
//    else
//      pblshdata(FORNO_ONOFF, &LIGHT_OFF, 1);
//
//    bFornoState = _bState;
//  }
//}

void B_ButtonActions(const int value)  // example of registering Multi-Presses
{
  Serial.print(F("Button:\t"));
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
      Serial.println(F("Whole Lotta Presses"));
      break;
  }
}

#define ON "Acceso"
#define OFF "Spento"
//========================= S I N G O L O   C L I C K ============================================
void click() {
  if (coilTettoia.stato != Souliss_T1n_OnCoil ) {
    coilTettoia.stato = Souliss_T1n_OnCoil;
  } else {
    coilTettoia.stato = Souliss_T1n_OffCoil;
  }

#ifdef SERIAL_DEBUG
  Serial.println("Single click" );
  Serial.print("Tettoia: ");
  Serial.println(coilTettoia.stato);

#endif

} // click

//========================= D O P P I O   C L I C K ============================================
void doubleClick() {
//  if (coilMuro.stato != Souliss_T1n_OnCoil) {
//    coilMuro.stato = Souliss_T1n_OnCoil;
//  } else {
//    coilMuro.stato = Souliss_T1n_OffCoil;
//  }

#ifdef SERIAL_DEBUG
  Serial.print("Doppio click - " );
  Serial.println("Muro");
 // Serial.println(coilMuro.stato);
#endif

//  if (mInput(SLOT_LOCALE_X_RELAY_REMOTO_0) != coilMuro.stato ) {
    //se lo stato remoto è diverso allora invio lo stato impostato localmente
  //   Serial.println("Muro - Invio comando");
    Send(REMOTE_ADDRESS, SLOT_RELAY_REMOTO_0, Souliss_T1n_ToggleCmd);
  //} else {
//    coilMuro.stato = mInput(SLOT_LOCALE_X_RELAY_REMOTO_0);
 // }
} // doubleclick

//========================= T R I P L O   C L I C K ============================================
// this function will be called when the button was pressed 3 times in a short timeframe.
void tripleClick() {
//  if (coilForno.stato !=  Souliss_T1n_OnCoil) {
//    coilForno.stato = Souliss_T1n_OnCoil;
//  } else {
//    coilForno.stato = Souliss_T1n_OffCoil;
//  }
  
#ifdef SERIAL_DEBUG
  Serial.print("Triplo click - " );
  Serial.println("Forno");
  //Serial.println(coilForno.stato);
#endif

//  if (mInput(SLOT_LOCALE_X_RELAY_REMOTO_1) != coilForno.stato ) {
    //se lo stato remoto è diverso allora invio lo stato impostato localmente
 //   Serial.println("Forno - Invio comando");
    Send(REMOTE_ADDRESS, SLOT_RELAY_REMOTO_1, Souliss_T1n_ToggleCmd);
  //} else {
   // coilForno.stato = mInput(SLOT_LOCALE_X_RELAY_REMOTO_1);
 // }

} // tripleclick

//========================= L O N G   C L I C K ============================================
void longPress() {
  //if (!(coilTettoia.stato == Souliss_T1n_OnCoil | coilForno.stato == Souliss_T1n_OnCoil | coilMuro.stato == Souliss_T1n_OnCoil) ) { //se le luci sono tutte spente
#ifdef SERIAL_DEBUG
    Serial.println("LongPress - Tutte le luci sono spente, dunque accendo tutto" );
#endif
  //  coilsState.stato = Souliss_T1n_OnCoil;
 // } else {
#ifdef SERIAL_DEBUG
  //  Serial.println("LongPress - almeno una luce è accesa, dunque spengo tutto" );
#endif
   // coilsState.stato = Souliss_T1n_OffCoil;
 // }
 // coilsState.toDoCmd = true;

//  if (coilsState.toDoCmd && coilsState.stato == Souliss_T1n_OffCoil) {
//    Serial.println("Spegnimento di tutte le luci" );
//    coilTettoia.stato = Souliss_T1n_OffCoil;
//    coilForno.stato = Souliss_T1n_OffCoil;
//    coilMuro.stato = Souliss_T1n_OffCoil;
//  } else if (coilsState.toDoCmd && coilsState.stato == Souliss_T1n_OnCoil) {
//    Serial.println("Accensione di tutte le luci" );
//    coilTettoia.stato = Souliss_T1n_OnCoil;
//    coilForno.stato = Souliss_T1n_OnCoil;
//    coilMuro.stato = Souliss_T1n_OnCoil;
//  }
//
//  if (coilsState.toDoCmd) {
//    Send(REMOTE_ADDRESS, SLOT_RELAY_REMOTO_0, coilMuro.stato);
//    Send(REMOTE_ADDRESS, SLOT_RELAY_REMOTO_1, coilForno.stato);
//    coilsState.toDoCmd = false;
//  }
} // tripleclick
