#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>

// network addresses
#define network_address_2	0x0012 	// 0x0011 is equal to 17 in decimal
#define network_my_subnet	0xFF00
#define network_my_supern	0x0000

// define the shared memory map
U8 memory_map[MaCaco_MEMMAP];

// flag 
U8 data_changed = 0;

#define time_base_fast		10				// Time cycle in milliseconds
#define time_base_slow		10000			// Time cycle in milliseconds
#define num_phases			255				// Number of phases

U8 phase_speedy=0, phase_fast=0, phase_slow=0;
unsigned long tmr_fast=0, tmr_slow=0;  


U8 firstPIN=2; //first pin used by Arduino for relay board
U8 lastPin=9;  //last pin used by Arduino for relay board

U8 iSLOT=0;   //first slot used by Typical 11

void setup()
{	
	Souliss_SetAddress(network_address_2, network_my_subnet, network_my_supern);		
	// Load the address also in the memory_map
	Souliss_SetLocalAddress(memory_map, network_address_2);	

//setup 8 Typical 11
for(U8 iPin=firstPIN;iPin<=lastPin;iPin++)
{
  pinMode(iPin, OUTPUT);
  digitalWrite(iPin, HIGH);
  Souliss_SetT11(memory_map,iSLOT++);
  }

}

void loop()
{ 
	// The Souliss methods are scheduled in phases, this allow load
	// balance and proper timing.

	if(abs(millis()-tmr_fast) > time_base_fast)
	{	
		tmr_fast = millis();
		phase_fast = (phase_fast + 1) % num_phases;

		// Execute the code every 3 time_base_fast		
		if (!(phase_fast % 3))
		{
			// TO IMPLEMENT: SETUP PIN FOR WALL SWITCH
			//Souliss_DigIn( ETC....
			
	// Execute the logic for typicals 11
        U8 iSlot;
       iSLOT=0;
	for(U8 iPin=firstPIN;iPin<=lastPin;iPin++)
        {		
              Souliss_Logic_T11(memory_map, iSLOT, &data_changed);
              Souliss_LowDigOut(iPin, Souliss_T1n_Coil, memory_map, iSLOT);	
              iSLOT++;
        }
  	}	
		
		// Execute the code every 5 time_base_fast		  
		if (!(phase_fast % 5))
		{   
			// Retreive data from the communication channel
			Souliss_CommunicationData(memory_map, &data_changed);
		}			
	}
} 
