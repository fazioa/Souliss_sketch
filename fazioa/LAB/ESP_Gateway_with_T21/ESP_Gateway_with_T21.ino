/**************************************************************************
    Souliss - Typicals examples on Expressif ESP8266

      T21: Motorized devices with limit switches

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
#define thisGW_address  0xAB20
#define myvNet_subnet 0xFF00
//*************************************************************************
#define slotT21 0


#define INPUTPIN_LIMIT_OPENING_T21 3
#define INPUTPIN_LIMIT_CLOSING_T21 1

#define INPUTPIN_TOGGLE_CMD_T21 0



#define OUTPUTPIN_UP_pinT2n 14 //output pin
#define OUTPUTPIN_DOWN_pinT2n 15 //output pin



void setup()
{
  Serial.begin(9600);
  Serial.println("Start");
  Initialize();
  GetIPAddress();
  SetAsGateway(myvNet_dhcp);
  SetAddress(thisGW_address, myvNet_subnet, 0x0000);

  //*************************************************************************
  //*************************************************************************
  Set_Windows (slotT21);
  // Define inputs, outputs pins
  pinMode(INPUTPIN_LIMIT_OPENING_T21, INPUT_PULLUP);                  // Hardware pulldown required
  pinMode(INPUTPIN_LIMIT_CLOSING_T21, INPUT_PULLUP);

  pinMode(INPUTPIN_TOGGLE_CMD_T21, INPUT_PULLUP);

  pinMode(OUTPUTPIN_UP_pinT2n, OUTPUT);
  pinMode(OUTPUTPIN_DOWN_pinT2n, OUTPUT);

}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_510ms() {
      LowDigIn(INPUTPIN_TOGGLE_CMD_T21, Souliss_T2n_ToggleCmd , slotT21);


      LowDigIn(INPUTPIN_LIMIT_OPENING_T21, Souliss_T2n_LimSwitch_Open, slotT21);
      LowDigIn(INPUTPIN_LIMIT_CLOSING_T21, Souliss_T2n_LimSwitch_Close, slotT21);


      Logic_Windows(slotT21); //input PIN -> INPUTPIN_LIMIT_OPENING_T21 and INPUTPIN_LIMIT_CLOSING_T21

      DigOut(OUTPUTPIN_UP_pinT2n, Souliss_T2n_Coil_Open, slotT21); //set pin to HIGH when output state is Souliss_T2n_Coil_Open
      DigOut(OUTPUTPIN_DOWN_pinT2n, Souliss_T2n_Coil_Close, slotT21); //set pin to HIGH when output state is Souliss_T2n_Coil_Close
    }

    FAST_1110ms() {
      Timer_Windows(slotT21);
    }

    FAST_GatewayComms();
  }

  EXECUTESLOW() {
    UPDATESLOW();
    SLOW_10s() {

    }
  }
}
