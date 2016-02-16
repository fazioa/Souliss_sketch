#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <WiFiUdp.h>

// Configure the Souliss framework
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/RuntimeGateway.h"            // This node is a Peer and can became a Gateway at runtime
#include "conf/DynamicAddressing.h"         // Use dynamically assigned addresses
#include "conf/WEBCONFinterface.h"          // Enable the WebConfig interface

#include "Souliss.h"
//*************************************************************************
//*************************************************************************

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println(F("Delay 3s"));
  //delay 15 seconds
  delay(3000);
  Serial.println(F("Start"));


  Serial.println("");
  Serial.println(F("Reset"));
  Store_Init();
  Store_Clear();
  Store_Commit();
  Serial.println(F("OK"));

}

void loop() {
  // put your main code here, to run repeatedly:

}
