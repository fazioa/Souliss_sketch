#include "SoulissFramework.h"
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/IPBroadcast.h"

// **** Define the WiFi name and password ****
#define WIFICONF_INSKETCH
#define WiFi_SSID               "souliss"
#define WiFi_Password           "souliss01"


// Include framework code and libraries
#include <ESP8266WiFi.h>
#include "Souliss.h"
#include "topics.h"
//*************************************************************************
// Define the network configuration according to your router settingsuration according to your router settings
// and the other on the wireless oneless one
#define peer_address  0xAB31
#define myvNet_subnet 0xFF00
#define myvNet_supern 0xAB10

//*************************************************************************

#define SLOT_POWERSOCKET 0
#define PIN_POWERSOCKET 1
#define PIN_ACTIVITYLED 3
bool bActivityLed=LOW;

uint8_t mypayload_len = 0;
U8 mypayload;

void setup()
{
  // Serial.begin(9600);
  //  Serial.println("Start. Delay 10 seconds... ");
  //delay 30 seconds
  delay(10000);
  Initialize();
  //  Serial.print("Get IP Address: ");
  GetIPAddress();
  //Serial.println(WiFi.localIP());
  SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  // Define output pins
  pinMode(PIN_POWERSOCKET, OUTPUT);    // Relè
   pinMode(PIN_ACTIVITYLED, OUTPUT);    // Activity Led
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_50ms() {
      subcriptionHeating_ON_OFF();
    }
    FAST_2110ms(){
      bActivityLed=!bActivityLed;
      digitalWrite(PIN_ACTIVITYLED, bActivityLed);
    }

    FAST_PeerComms();
  }
}

void subcriptionHeating_ON_OFF() {
  if (subscribedata(SST_HEAT_ONOFF, &mypayload, &mypayload_len)) {
    if (mypayload == HEAT_ON)
      digitalWrite(PIN_POWERSOCKET, HIGH);
    else
      digitalWrite(PIN_POWERSOCKET, LOW);
  }
}


