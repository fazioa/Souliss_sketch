//#define VNET_DEBUG_INSKETCH
//#define VNET_DEBUG  1
//#define SOULISS_DEBUG_INSKETCH
//#define SOULISS_DEBUG      1
//#define MaCaco_DEBUG_INSKETCH
//#define MaCaco_DEBUG      1
/**************************************************************************
  Sketch: ESP8266 WiFi Relay V3 - Souliss
  Author: Tonino Fazio

  This example is only supported on ESP8266.

  //Used pins
  // pin 12: onboad relay ON
  // pin 13: onboad relay OFF
  // pin 14: switch
***************************************************************************/
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

#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/IPBroadcast.h"
//#include "topics.h"

#include "Souliss.h"

#include <SPI.h>
#include <OneWire.h>
#include "DallasTemperature.h"

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
#define PIN_RELAY_ON 12
#define PIN_RELAY_OFF 13
#define PIN_LED 2

#define PIN_ONEWIRE_SENSORS 14

#define DEADBAND        0.01    // Deadband value 1%

// Initialize DHT sensors
OneWire oneWire(PIN_ONEWIRE_SENSORS);
DallasTemperature sensors(&oneWire);

uint16_t output16;
uint8_t valByteArray[2];
float temperature;
void setup()
{
 // Serial.begin(9600);
  Initialize();
  GetIPAddress();

  SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  //*************************************************************************
  //*************************************************************************

  //  pinMode(PIN_SWITCH, INPUT_PULLUP);    // Switch

  digitalWrite(PIN_RELAY_ON, LOW);
  pinMode(PIN_RELAY_ON, OUTPUT);    // Relay ON

  digitalWrite(PIN_RELAY_OFF, LOW);
  pinMode(PIN_RELAY_OFF, OUTPUT);    // Relay OFF
  pinMode(PIN_LED, OUTPUT);


  pinMode(PIN_ONEWIRE_SENSORS, INPUT);

  Set_SimpleLight(SLOT_RELAY_0);
  Set_Temperature(SLOT_TEMPERATURE_ONE);
  Set_Temperature(SLOT_TEMPERATURE_TWO);

  mOutput(SLOT_RELAY_0) = Souliss_T1n_OnCoil; //Set output to ON, then first execution of DigIn2State cause a change state to OFF.

  // Start up the library
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12.

  // Init the OTA
  ArduinoOTA.setHostname("ex-store-birra");
  ArduinoOTA.begin();
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_2110ms() {
      activity();
    }

    FAST_50ms() {
      //    DigIn2State(PIN_SWITCH, Souliss_T1n_ToggleCmd, Souliss_T1n_ToggleCmd, SLOT_RELAY_0);
      Logic_SimpleLight(SLOT_RELAY_0);

      PulseLowDigOut(PIN_RELAY_ON, Souliss_T1n_OnCoil, SLOT_RELAY_0);
      PulseLowDigOut(PIN_RELAY_OFF, Souliss_T1n_OffCoil, SLOT_RELAY_0);
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
   // Serial.println(temperature);
    ImportAnalog(SLOT_TEMPERATURE_ONE, &temperature);

    //    //PUBLISH
    //    float16(&output16, &temperature);
    //    valByteArray[0] = C16TO8L(output16);
    //    valByteArray[1] = C16TO8H(output16);
    //    publishdata(TEMPERATURE_TOPIC_NODE_ESP_EXSTORE_ds18b20_SENSOR_1, valByteArray, 2);

  }

  SHIFT_2110ms(100) {
    temperature = sensors.getTempCByIndex(1);
  //  Serial.println(temperature);
    ImportAnalog(SLOT_TEMPERATURE_TWO, &temperature);


    //PUBLISH
//    float16(&output16, &temperature);
//    valByteArray[0] = C16TO8L(output16);
//    valByteArray[1] = C16TO8H(output16);
//    publishdata(TEMPERATURE_TOPIC_NODE_ESP_EXSTORE_ds18b20_SENSOR_2, valByteArray, 2);
  }

  EXECUTESLOW() {
    UPDATESLOW();
    SLOW_10s() {
      Timer_SimpleLight(SLOT_RELAY_0);
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
