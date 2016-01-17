/**************************************************************************
    Souliss
    
  NODE:
  Typical 12 : ON/OFF Digital Output with AUTO mode
***************************************************************************/
  
#include "bconf/Chibiduino_v1.h"			// Use a Chibiduino 2.4 GHz wireless board

#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>


#define SLOT_T12      	0

#define PIN_auto        15
#define PIN_OUT        14

// Define the network configuration according to your router settingsuration according to your router settings
#define	Gateway_address	0x6511				// The Gateway node has two address, one on the Ethernet side 69				// The Gateway node has two address, one on the Ethernet side
// and the other on the wireless oneless one
#define	Peer_address	0x6513
#define	myvNet_subnet	0xFF00
#define	myvNet_supern	Gateway_address


void setup()
{
  Serial.begin(9600);

  Initialize();
  // Setup the network configuration
  Souliss_SetAddress(Peer_address, myvNet_subnet, myvNet_supern);					// Address on the wireless interface
  // Set the typical to use
  
  Set_T12(SLOT_T12);
 
  pinMode(PIN_auto, INPUT);
  pinMode(PIN_OUT, OUTPUT);

}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();
    FAST_50ms() {
      DigIn(PIN_auto,9,SLOT_T12);
      Logic_T12(SLOT_T12);
      nDigOut(PIN_OUT, Souliss_T1n_Coil & Souliss_T1n_AutoOnCoil, SLOT_T12);
//      LowDigOut(PIN_OUT, Souliss_T1n_AutoOnCoil , SLOT_T12);
  }
    // Process the communication
    FAST_PeerComms();
  }
  
      
    EXECUTESLOW() { 
        UPDATESLOW();
        SLOW_10s() {  // Process the timer every 10 seconds  
            Timer_AutoLight(SLOT_T12);
        }     
    }
}

