/**************************************************************************
    Souliss - Hello World

    This is the basic example, control one LED via a push-button or Android
    using SoulissApp (get it from Play Store).

    Run this code on one of the following boards:
      - Arduino Ethernet (W5100)
      - Arduino with Ethernet Shield (W5100)

    As option you can run the same code on the following, just changing the
    relevant configuration file at begin of the sketch
      - Arduino with W5200 Ethernet Shield
      - Arduino with W5500 Ethernet Shield

***************************************************************************/

// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"

// Configure the framework
#include "bconf/StandardArduino.h"          // Use a standard Arduino
#include "conf/ethW5100.h"                  // Ethernet through Wiznet W5100
#include "conf/Gateway.h"                   // The main node is the Gateway, we have just one node
#include "conf/Webhook.h"                   // Enable DHCP and DNS

// Include framework code and libraries
#include <SPI.h>

/*** All configuration includes should be above this line ***/
#include "Souliss.h"

// This identify the number of the LED logic
#define slotT22_saracinesca 0

#define pinInputOPEN   2
#define pinInputCLOSE   3
#define pinOutputReleOPEN   4
#define pinOutputReleCLOSE   5
#define pinOutputReleWARNINGLIGHT   6

#define Souliss_T2n_Timer_Val      0xA3

U8 bOut = HIGH;

void setup()
{
  Initialize();

  // Get the IP address from DHCP
  GetIPAddress();
  SetAsGateway(myvNet_dhcp);       // Set this node as gateway for SoulissApp

  Set_Windows(slotT22_saracinesca);


  // We connect a pushbutton between 5V and pin2 with a pulldown resistor
  // between pin2 and GND, the LED is connected to pin9 with a resistor to
  // limit the current amount

  pinMode(pinInputOPEN, INPUT_PULLUP);
  // digitalWrite(pinInputOPEN, LOW);

  pinMode(pinInputCLOSE, INPUT_PULLUP);
  //   digitalWrite(pinInputCLOSE, LOW);

  pinMode(pinOutputReleOPEN, OUTPUT);
  pinMode(pinOutputReleCLOSE, OUTPUT);
  pinMode(pinOutputReleWARNINGLIGHT, OUTPUT);
}

void loop()
{
  // Here we start to play
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_50ms() {   // We process the logic and relevant input and output every 50 milliseconds
      DigIn2State(pinInputOPEN, Souliss_T2n_StopCmd, Souliss_T2n_OpenCmd_Local, slotT22_saracinesca);
      DigIn2State(pinInputCLOSE, Souliss_T2n_StopCmd, Souliss_T2n_CloseCmd_Local, slotT22_saracinesca);
      Souliss_Logic_T22(memory_map, slotT22_saracinesca, &data_changed, Souliss_T2n_Timer_Val + 10);                      // Drive the LED as per command

      DigOut(pinOutputReleOPEN, Souliss_T2n_OpenCmd, slotT22_saracinesca);
      DigOut(pinOutputReleCLOSE, Souliss_T2n_CloseCmd, slotT22_saracinesca);
    }
    FAST_510ms() {
      if (mOutput(slotT22_saracinesca)  == Souliss_T2n_OpenCmd || mOutput(slotT22_saracinesca)  == Souliss_T2n_CloseCmd) {
      digitalWrite(pinOutputReleWARNINGLIGHT, bOut);
        bOut = !bOut;
      } else digitalWrite(pinOutputReleWARNINGLIGHT, LOW);
    }


    FAST_1110ms() {
      // Time out commands if no limit switches are received
      Timer_Windows(slotT22_saracinesca);
    }
    // Here we handle here the communication with Android, commands and notification
    // are automatically assigned to MYLEDLOGIC
    FAST_GatewayComms();

  }
  EXECUTESLOW() {
    UPDATESLOW();


  }
}
