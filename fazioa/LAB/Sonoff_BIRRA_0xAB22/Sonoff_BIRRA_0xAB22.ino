/**************************************************************************
  Sketch: Sonoff Basic con due sensori di temperatura 
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
#include "SoulissFramework.h"


// **** Define the WiFi name and password ****
#define WIFICONF_INSKETCH
#define WiFi_SSID               "asterix"
#define WiFi_Password           "ttony2013"



// Include framework code and libraries
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define HOSTNAME "souliss-sonoff-birra"

#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/IPBroadcast.h"

#include "Souliss.h"

#include <SPI.h>
#include <OneWire.h>
 #include "DallasTemperature.h"
#include "credenziali.h"
#include <UniversalTelegramBot.h>

//*************************************************************************
// Define the network configuration according to your router settingsuration according to your router settings
// and the other on the wireless oneless one
#define peer_address  0xAB22
#define myvNet_subnet 0xFF00
#define myvNet_supern 0xAB10

//*************************************************************************

#define SLOT_RELAY_0 0

#define SLOT_TEMPERATURE_ONE        1     // This is the memory slot used for the execution of the logic in network_address1
#define SLOT_TEMPERATURE_TWO     3

//#define PIN_SWITCH 14
#define PIN_RELAY 12

#define PIN_LED 13

#define PIN_ONEWIRE_SENSORS 14

#define DEADBAND        0.01    // Deadband value 1%

//Variable to Handle WiFio Signal
long rssi = 0;
int bars = 0;
#define T_WIFI_STRDB  5 //It takes 2 slots
#define T_WIFI_STR    7 //It takes 2 slots


// Initialize DHT sensors
OneWire oneWire(PIN_ONEWIRE_SENSORS);
DallasTemperature sensors(&oneWire);

uint16_t output16;
uint8_t valByteArray[2];
float temperature;
void setup()
{
#ifdef SERIAL_DEBUG
  Serial.begin(115200);
  Serial.println("Node Starting");
#endif

  //delay 10 seconds
  //delay(10000);

  Initialize();
  GetIPAddress();

  SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  //*************************************************************************
  //*************************************************************************

    pinMode(PIN_RELAY, OUTPUT);    // Relay ON
  digitalWrite(PIN_RELAY, LOW);
  
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  pinMode(PIN_ONEWIRE_SENSORS, INPUT);

  Set_SimpleLight(SLOT_RELAY_0);
  Set_Temperature(SLOT_TEMPERATURE_ONE);
  Set_Temperature(SLOT_TEMPERATURE_TWO);

  
  // Start up the library
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12.

  Set_T51(T_WIFI_STRDB); //Imposto il tipico per contenere il segnale del Wifi in decibel
  Set_T51(T_WIFI_STR); //Imposto il tipico per contenere il segnale del Wifi in barre da 1 a 5

  // Init the OTA
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.begin();
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_2110ms() {
      activity();
    }

    FAST_7110ms() {
      //Processa le logiche per il segnale WiFi
      Read_T51(T_WIFI_STRDB);
      Read_T51(T_WIFI_STR);
    }

    FAST_50ms() {
      //    DigIn2State(PIN_SWITCH, Souliss_T1n_ToggleCmd, Souliss_T1n_ToggleCmd, SLOT_RELAY_0);
      Logic_SimpleLight(SLOT_RELAY_0);
 DigOut(PIN_RELAY, Souliss_T1n_Coil, SLOT_RELAY_0);

    }
    FAST_PeerComms();
  }

  FAST_510ms() {
    // Compare the acquired input with the stored one, send the new value to the
    // user interface if the difference is greater than the deadband
    Logic_Temperature(SLOT_TEMPERATURE_ONE);
    Logic_Temperature(SLOT_TEMPERATURE_TWO);
  }


  SHIFT_2110ms(0) {
    sensors.requestTemperatures(); // Send the command to get temperatures

    temperature = sensors.getTempCByIndex(0);
    #ifdef SERIAL_DEBUG
     Serial.print("Temperatura Sensore 1: ");
     Serial.println(temperature);
    #endif
    ImportAnalog(SLOT_TEMPERATURE_ONE, &temperature);
  }

  SHIFT_2110ms(100) {
    temperature = sensors.getTempCByIndex(1);
    #ifdef SERIAL_DEBUG
     Serial.print("Temperatura Sensore 2: ");
     Serial.println(temperature);
    #endif
    ImportAnalog(SLOT_TEMPERATURE_TWO, &temperature);


  }

  EXECUTESLOW() {
    UPDATESLOW();
    SLOW_10s() {
      Timer_SimpleLight(SLOT_RELAY_0);
      check_wifi_signal();
    }
  }

  // Look for a new sketch to update over the air
  ArduinoOTA.handle();
}


U8 activity_led_status = 0;
void activity() {
  if (activity_led_status == 0) {
    digitalWrite(PIN_LED, HIGH);
    activity_led_status = 1;
  } else {
    digitalWrite(PIN_LED, LOW);
    activity_led_status = 0;
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
  Serial.print("wifi rssi:");
  Serial.println(f_rssi);
  Serial.print("wifi bars:");
  Serial.println(f_bars);
#endif
}
