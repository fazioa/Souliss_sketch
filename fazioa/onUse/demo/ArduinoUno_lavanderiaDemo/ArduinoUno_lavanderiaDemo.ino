// Configure the framework
#include "SoulissFramework.h"
#include "bconf/StandardArduino.h"          // Use a standard Arduino
#include "conf/ethW5100.h"                  // Ethernet through Wiznet W5100
#include "conf/Webhook.h"                   // Enable DHCP and DNS

#include "Souliss.h"
#include "Typicals.h"
//#include <SPI.h>

uint8_t Gateway_address[4]  = {192, 168, 20, 20};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {192, 168, 20, 1};

#define Gateway_vNet_address Gateway_address[3]              // The Gateway node has two address, one on the Ethernet side
#define  peer_address  0x11
#define myvNet_subnet 0xFF00
#define myvNet_supern Gateway_vNet_address

// This identify the number of the LED logic
#define slotT22_saracinesca 0
#define slotT11_WARNINGLIGHT 2
#define slotT13_CURTAINLIGHT 3

#define pinInputOPEN   2   //used internal pull up resistor
#define pinInputCLOSE   3 //used internal pull up resistor

#define pinOutputReleOPEN   4
#define pinOutputReleCLOSE   5
#define pinOutputReleWARNINGLIGHT   6
#define pinInputCURTAINLIGHT   7  //need pull down resistor

#define timer_saracinesca      Souliss_T2n_Timer_Off+0x12 //18 DEC - Circa 20 secondi

U8 precPositionT22;
void setup()
{
  //Serial.begin(9600);
  Initialize();

  // Get the IP address from DHCP
  GetIPAddress();
  SetAddress(peer_address, myvNet_subnet, myvNet_supern);          // Address on the wireless interface

  pinMode(pinInputOPEN, INPUT_PULLUP);
  pinMode(pinInputCLOSE, INPUT_PULLUP);
  pinMode(pinInputCURTAINLIGHT, INPUT);

  pinMode(pinOutputReleOPEN, OUTPUT);
  pinMode(pinOutputReleCLOSE, OUTPUT);
  pinMode(pinOutputReleWARNINGLIGHT, OUTPUT);

  Set_Windows(slotT22_saracinesca);


  Set_SimpleLight(slotT11_WARNINGLIGHT); //luce lampeggiante di sicurezza
  Set_DigitalInput(slotT13_CURTAINLIGHT); //barriera infrarosso di sicurezza - E' impostata solo per avere lo stato in SoulissApp
  mInput(slotT22_saracinesca) = Souliss_T2n_StopCmd;
}

void loop()
{
  // Here we start to play
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_50ms() {   // We process the logic and relevant input and output every 50 milliseconds
      LowDigIn(pinInputOPEN, Souliss_T2n_OpenCmd_Local, slotT22_saracinesca);
      LowDigIn(pinInputCLOSE, Souliss_T2n_CloseCmd_Local, slotT22_saracinesca);
      DigIn2State(pinInputCURTAINLIGHT, Souliss_T1n_OnCmd, Souliss_T1n_OffCmd,  slotT13_CURTAINLIGHT); //sensore a tenda

      Souliss_Logic_T22(memory_map, slotT22_saracinesca, &data_changed, timer_saracinesca);
      Logic_SimpleLight(slotT11_WARNINGLIGHT); //processa la logica per la luce lampeggiante di sicurezza
      Logic_DigitalInput(slotT13_CURTAINLIGHT);   //processa la logica per la barriera infrarosso di sicurezza


      DigOut(pinOutputReleWARNINGLIGHT, Souliss_T1n_OnCoil, slotT11_WARNINGLIGHT); //processa la logica per la luce di sicurezza
      DigOut(pinOutputReleOPEN, Souliss_T2n_Coil_Open, slotT22_saracinesca); //pone a 1 il PIN di OUTPUT quando lo stato di uscita dello slot è Souliss_T2n_Coil_Open
      DigOut(pinOutputReleCLOSE, Souliss_T2n_Coil_Close, slotT22_saracinesca); //pone a 1 il PIN di OUTPUT quando lo stato di uscita dello slot è Souliss_T2n_Coil_Close


      if (mOutput(slotT13_CURTAINLIGHT) == Souliss_T1n_OnCoil) {
        mInput(slotT22_saracinesca) = Souliss_T2n_StopCmd; //ferma il movimento se il sensore a tenda è attivato
      }
    }

    FAST_510ms() {

    
       
      if (mOutput(slotT22_saracinesca)  == Souliss_T2n_Coil_Open || mOutput(slotT22_saracinesca)  == Souliss_T2n_Coil_Close) {
        mInput(slotT11_WARNINGLIGHT) = Souliss_T1n_OnCmd;  //accende la luce di sicurezza quando la saracinesca è in movimento
       } else if (mOutput(slotT22_saracinesca) != precPositionT22) {
        mInput(slotT11_WARNINGLIGHT) = Souliss_T1n_OffCmd;   //spegne la lampada solo se è rilevata una variazione dello stato del T22
        Serial.println("slotT11_WARNINGLIGHT = Souliss_T1n_OffCmd");
      }
  precPositionT22 = mOutput(slotT22_saracinesca);
  Serial.print("precPositionT22 -->"); Serial.println(precPositionT22);
    }



    FAST_1110ms() {
      // Time out commands if no limit switches are received
      Timer_Windows(slotT22_saracinesca);
    }
    // Process the communication
    FAST_PeerComms();
  }
 }
