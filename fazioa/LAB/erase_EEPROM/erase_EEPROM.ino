/**************************************************************************
Souliss OTA
ERASE EEPROM

Test Node with Dimmer Light
with ERASE EEPROM
 
***************************************************************************/
// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"
// Configure the Souliss framework
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/RuntimeGateway.h"            // This node is a Peer and can became a Gateway at runtime
/*** All configuration includes should be above this line ***/ 
#include "Souliss.h"
   

void setup()
{
    // Erase network configuration parameters from previous use of ZeroConf
  Store_Init();
  Store_Clear();
  Store_Commit();

}

void loop()
{  
  
}    
