/**************************************************************************
    Souliss - Serranda Lavanderia

    Run this code on one of the following boards:
	  - Arduino Uno
      - Arduino Ethernet (W5100)
***************************************************************************/
// If in debug mode - print debug information in Serial. Comment in production code, this bring performance.
// This method is good for development and verification of results. But increases the amount of code and decreases productivity.
//#define DEBUG


// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"

// Configure the framework
#include "bconf/DINo_v2.h"                  // Define the board type
#include "conf/Gateway.h"                   // The main node is the Gateway, we have just one node

// Include framework code and libraries
#include <SPI.h>

/*** All configuration includes should be above this line ***/
#include "Souliss.h"

// Define the network configuration according to your router settings
uint8_t ip_address[4]  = {192, 168, 1, 39};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {192, 168, 1, 1};
#define myvNet_address  ip_address[3]       // The last byte of the IP address (77) is also the vNet address
#define myvNet_subnet   0xFF00



// This identify the number of the LED logic
#define slotT22_saracinesca 0
#define slotT11_WARNINGLIGHT 2
#define slotT13_CURTAIN_SENSOR 3

// IN1 --> pinInputOPEN
// IN2 --> pinInputCLOSE
// IN3--> pinInputCURTAIN_SENSOR

// RELAY1 --> pinOutputReleOPEN
// RELAY2 --> pinOutputReleCLOSE
// RELAY3 --> pinOutputReleWARNINGLIGHT


#define timer_saracinesca      Souliss_T2n_Timer_Off+0x20 //32 DEC - Circa 35 secondi

U8 precPositionT22;
void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
#endif


  //Initialize();
  InitDINo();
  // Get the IP address from DHCP
  //GetIPAddress();
  // Set network parameters
  Souliss_SetIPAddress(ip_address, subnet_mask, ip_gateway);
  SetAsGateway(myvNet_address);       // Set this node as gateway for SoulissApp

  Set_Windows(slotT22_saracinesca);


  Set_SimpleLight(slotT11_WARNINGLIGHT); //luce lampeggiante di sicurezza
  Set_DigitalInput(slotT13_CURTAIN_SENSOR); //barriera infrarosso di sicurezza - E' impostata solo per avere lo stato in SoulissApp

    SetInput1(); // CURTAIN_SENSOR
  SetInput2(); // up / dwn
  SetInput3(); // up / dwn


  SetRelay1(); //OPEN
  SetRelay2(); // CLOSE
  SetRelay3(); //WARNINGLIGHT


#ifdef DEBUG
  Serial.println("Start");
#endif
  // Set and turn ON the status LED
  SetLED();
  TurnOnLED();

  mInput(slotT22_saracinesca) = Souliss_T2n_StopCmd;
  
}

void loop()
{
  // Here we start to play
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_90ms() {   // We process the logic and relevant input and output every 50 milliseconds
      DigIn(IN1, Souliss_T2n_OpenCmd_Local, slotT22_saracinesca);
      DigIn(IN2, Souliss_T2n_CloseCmd_Local, slotT22_saracinesca);
      DigIn2State(IN3, Souliss_T1n_OnCmd, Souliss_T1n_OffCmd,  slotT13_CURTAIN_SENSOR); //sensore a tenda
      Souliss_Logic_T22(memory_map, slotT22_saracinesca, &data_changed, timer_saracinesca);
      Logic_DigitalInput(slotT13_CURTAIN_SENSOR);   //processa la logica per la barriera infrarosso di sicurezza

      DigOut(RELAY1, Souliss_T2n_Coil_Open, slotT22_saracinesca); //pone a 1 il PIN di OUTPUT quando lo stato di uscita dello slot è Souliss_T2n_Coil_Open
      DigOut(RELAY2, Souliss_T2n_Coil_Close, slotT22_saracinesca); //pone a 1 il PIN di OUTPUT quando lo stato di uscita dello slot è Souliss_T2n_Coil_Close

      if (mOutput(slotT13_CURTAIN_SENSOR) == Souliss_T1n_OnCoil && mOutput(slotT22_saracinesca) != Souliss_T2n_Coil_Stop) {

#ifdef DEBUG
        Serial.println("Attivazione sensore a tenda. STOP movimento saracinesca");
#endif
        mInput(slotT22_saracinesca) = Souliss_T2n_StopCmd; //ferma il movimento se il sensore a tenda è attivato
      }
    }

    FAST_210ms() {
      Logic_SimpleLight(slotT11_WARNINGLIGHT); //processa la logica per la luce lampeggiante di sicurezza
      DigOut(RELAY3, Souliss_T1n_OnCoil, slotT11_WARNINGLIGHT); //processa la logica per la luce di sicurezza
    }

    FAST_510ms() {
      if (mOutput(slotT22_saracinesca)  == Souliss_T2n_Coil_Open || mOutput(slotT22_saracinesca)  == Souliss_T2n_Coil_Close) {
        mInput(slotT11_WARNINGLIGHT) = Souliss_T1n_OnCmd;  //accende la luce di sicurezza quando la saracinesca è in movimento
#ifdef DEBUG
        Serial.println("Saracinesca in movimento. Forza accensione luce di sicurezza");
#endif
      } else if (mOutput(slotT22_saracinesca) != precPositionT22) {
        mInput(slotT11_WARNINGLIGHT) = Souliss_T1n_OffCmd;   //spegne la lampada solo se è rilevata una variazione dello stato del T22
#ifdef DEBUG
        Serial.println("Saracinesca ferma. Spegnimento  luce di sicurezza");
#endif
      }
      precPositionT22 = mOutput(slotT22_saracinesca);

    }



    FAST_1110ms() {
      // Time out commands if no limit switches are received
      Timer_Windows(slotT22_saracinesca);
      ToggleLED();

    }
    // Here we handle here the communication with Android, commands and notification
    // are automatically assigned to slots
    FAST_GatewayComms();
  }
  EXECUTESLOW() {
    UPDATESLOW();
  }
}
