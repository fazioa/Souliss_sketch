#define HOST_NAME_INSKETCH
#define HOST_NAME "Souliss-Solar-Power-Meter"

#define  SERIALPORT_INSKETCH
#define LOG Serial
/**************************************************************************
Sketch: POWER METER - VER.1 - Souliss - Web Configuration
Author: Tonino Fazio

ESP Core 1.6.5 Staging 1.6.5-1160-gef26c5f
 This example is only supported on ESP8266.
***************************************************************************/
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
#include "EmonLib.h"             // Include Emon Library
EnergyMonitor emon1;             // Create an instance
//*************************************************************************
//*************************************************************************

#define     PIN_VOLTAGE             4
#define     PIN_CURRENT             5

#define     SLOT_Watt               0
#define     SLOT_Voltage               2
#define     SLOT_Current               4


// Setup the libraries for Over The Air Update
OTA_Setup();

#define SIZE 6
float fPowerValues[SIZE];
int i = 0;
float iMedia = 0;

void setup()
{
  // Init Serial
  Serial.begin(115200);
  Serial.println("POWER METER - VER.1 - Souliss");
  //delay 30 seconds
  //delay(30000);
  Initialize();

  // Read the IP configuration from the EEPROM, if not available start
  // the node as access point
  if (!ReadIPConfiguration())
  {
    // Start the node as access point with a configuration WebServer
    SetAccessPoint();
    startWebServer();
    // We have nothing more than the WebServer for the configuration
    // to run, once configured the node will quit this.
    while (1)
    {
      yield();
      runWebServer();
    }

  }

  if (IsRuntimeGateway())
  {
    // Connect to the WiFi network and get an address from DHCP
    SetAsGateway(myvNet_dhcp);       // Set this node as gateway for SoulissApp
    SetAddressingServer();
  }
  else
  {
    // This board request an address to the gateway at runtime, no need
    // to configure any parameter here.
    SetDynamicAddressing();
    GetAddress();
  }

  //*************************************************************************
  //*************************************************************************
  Set_Power(SLOT_Watt);
  Set_Voltage(SLOT_Voltage);
  Set_Current(SLOT_Current);

  // Define output pins
  pinMode(PIN_VOLTAGE, INPUT);
  pinMode(PIN_CURRENT, INPUT);
  emon1.voltage(PIN_VOLTAGE, 226, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current(PIN_CURRENT, 6);       // Current: input pin, calibration.

  LOG.println("Setup and OTA Init OK");
  // Init the OTA
  OTA_Init();

}

float fVal;
void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_50ms() {

    }


    //acquisizione valori
    FAST_1110ms() {
      if (i < SIZE) {
        emon1.calcVI(20, 200);  //esegue il campionamento // Calculate all. No.of wavelengths, time-out
        fVal = emon1.realPower;
        fPowerValues[i++] = fVal;
        LOG.print("lettura valori: ");
        LOG.print(analogRead(A0)); LOG.print(" VOLTAGE - ");
        LOG.print(analogRead(PIN_CURRENT)); LOG.println(" CURRENT");

        fVal = emon1.Vrms;
        ImportAnalog(SLOT_Voltage, &fVal);
        Logic_Voltage(SLOT_Voltage);
        //LOG.print("lettura valori: ");
        //LOG.print(fVal); LOG.print(" V - ");

        fVal = emon1.Irms;
        ImportAnalog(SLOT_Current, &fVal);
        Logic_Current(SLOT_Current);
        //LOG.print(fVal); LOG.println(" A");


      } else {
        //calcola media ed acquisisce il valore
        for (int j = 0; j < SIZE; j++) {
          iMedia += fPowerValues[j];
        }
        iMedia = round(iMedia / SIZE);
        // LOG.print("calcola media ed acquisisce il valore: ");
        // LOG.println(iMedia);
        ImportAnalog(SLOT_Watt, &iMedia);
        Logic_Power(SLOT_Watt);
        i = 0;
        iMedia = 0;
      }
    }


    // Run communication as Gateway or Peer
    if (IsRuntimeGateway())
      FAST_GatewayComms();
    else
      FAST_PeerComms();
  }

  EXECUTESLOW() {
    UPDATESLOW();
    SLOW_10s() {  // Process the timer every 10 seconds

    }
    // If running as Peer
    if (!IsRuntimeGateway())
      SLOW_PeerJoin();
  }
  // Look for a new sketch to update over the air
  OTA_Process();
}


