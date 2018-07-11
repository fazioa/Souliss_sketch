/**************************************************************************
    Souliss - Typicals examples on Expressif ESP8266


***************************************************************************/


#include "SoulissFramework.h"
#include <ESP8266mDNS.h>
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/Gateway.h"
#include "conf/IPBroadcast.h"

// **** Define the WiFi name and password ****
#define WIFICONF_INSKETCH
#define WiFi_SSID               "asterix"
#define WiFi_Password           "ttony2013"


// Include framework code and libraries
#include <ESP8266WiFi.h>
#include "Souliss.h"

//*************************************************************************
#define thisGW_address  0xAB50
#define myvNet_subnet 0xFF00
//*************************************************************************

#define SLOT_T61                  0 //analogIn
#define SLOT_T67                  2 //powerIn

float fT61_setpoint;
float fT67_setpoint;
void setup()
{
  Serial.begin(115200);
  Serial.println("Start");

  Initialize();
  Serial.println("GetIPAddress");
  GetIPAddress();
  SetAsGateway(myvNet_dhcp);
  SetAddress(thisGW_address, myvNet_subnet, 0x0000);

  Serial.println("Set Typicals T6x");
  Set_Analog_Setpoint(SLOT_T61);
  Set_Power_Setpoint(SLOT_T67);

  Serial.println("Going to loop");
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_50ms() {
      Logic_AnalogIn(SLOT_T61);
      Logic_Power_Setpoint(SLOT_T67);
    }
    FAST_1110ms() {
      fT61_setpoint = mOutputAsFloat(SLOT_T61);
      fT67_setpoint = mOutputAsFloat(SLOT_T67);

      Serial.print("State T61: ");
      Serial.println(fT61_setpoint);
      Serial.print("State T67: ");
      Serial.println(fT67_setpoint);
    }

    FAST_GatewayComms();
  }
}





