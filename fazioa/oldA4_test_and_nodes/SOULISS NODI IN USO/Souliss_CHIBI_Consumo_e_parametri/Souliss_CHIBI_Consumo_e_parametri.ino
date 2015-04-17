#define  BOARDTYPE_INSKETCH
#define  QC_BOARDTYPE  0x02      /** Freaklabs Chibiduino with Ethernet Shield (W5100) */

#define  GATEWAYTYPE_INSKETCH
#define  QC_GATEWAYTYPE  0x01    // Define board as gateway

/**************************************************************************
Souliss_Logic_T55 - Voltage (0, 400) V
Souliss_Logic_T56 - Current (0, 25)  A
Souliss_Logic_T57 - Power (0, 6500)  W
 ***************************************************************************/
#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>
#include "EmonLib.h"             // Include Emon Library
EnergyMonitor emon1;             // Create an instance

#define eth_bridge_address		0x0069
#define chibi_bridge_address      0x6511
#define network_chibi_address_2	0x6512 
#define network_chibi_address_3	0x6513 //giardino
#define network_chibi_address_4	0x6514 //tende

#define network_my_subnet		0xFF00
#define network_my_supern		0x0069

#define SLOT_Watt 0
#define SLOT_Volt 2
#define SLOT_Current 4

#define DEADBAND				0.05		// Deadband value 10%  

// define the shared memory map
U8 memory_map[MaCaco_MEMMAP] = { 
  0x00};

// flag 
U8 data_changed = 0;

#define time_base_fast		10				// Time cycle in milliseconds
#define time_base_slow		10000			// Time cycle in milliseconds
#define num_phases			255				// Number of phases

U8 phase_speedy=0, phase_fast=0, phase_slow=0;
unsigned long tmr_fast=0, tmr_slow=0;  

void setup()
{
 // Setup the network configuration	
	Souliss_SetAddress(chibi_bridge_address, network_my_subnet, network_my_supern);		
	Souliss_SetAddress(eth_bridge_address, network_my_subnet, network_my_supern);		  
	// Load the address also in the memory_map
	Souliss_SetLocalAddress(memory_map, eth_bridge_address);

	// Set the addresses of the remote nodes
        Souliss_SetRemoteAddress(memory_map, network_chibi_address_2, 1);
        Souliss_SetRemoteAddress(memory_map, network_chibi_address_3, 2);
        Souliss_SetRemoteAddress(memory_map, network_chibi_address_4, 3);
        
   Souliss_SetT55(memory_map, SLOT_Volt);
   Souliss_SetT56(memory_map, SLOT_Current);
   Souliss_SetT57(memory_map, SLOT_Watt);

  emon1.voltage(15, 226, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current(14, 6);       // Current: input pin, calibration.
}

float fVal;
void loop()
{ 

  if(abs(millis()-tmr_fast) > time_base_fast)
  {	
    tmr_fast = millis();
    phase_fast = (phase_fast + 1) % num_phases;

    // Execute the code every 5 time_base_fast		  
    if (!(phase_fast % 5))
    {   
      // Retreive data from the communication channel
      Souliss_CommunicationData(memory_map, &data_changed);		
    }	

    // Execute the code every 101 time_base_fast		  
    if (!(phase_fast % 101))
    {
      // Compare the acquired input with the stored one, send the new value to the
      // user interface if the difference is greater than the deadband
      Souliss_Logic_T55(memory_map, SLOT_Volt, DEADBAND, &data_changed);
    Souliss_Logic_T56(memory_map, SLOT_Current, DEADBAND, &data_changed);
    Souliss_Logic_T57(memory_map, SLOT_Watt, DEADBAND, &data_changed);
      
        // Acquire data from the microcontroller ADC
        emon1.calcVI(5,200); 
        fVal =emon1.Vrms;
     //   Serial.print(fVal);
      //  Serial.print(" ");
  
        Souliss_ImportAnalog(memory_map, SLOT_Volt, &fVal);
        fVal =emon1.Irms;
      //  Serial.print(fVal);
      //  Serial.print(" ");
        Souliss_ImportAnalog(memory_map, SLOT_Current, &fVal);

        fVal =emon1.realPower;
        if(abs(fVal)<0.99) fVal=0.01; //pongo a 0.01 perchè a 0.00 Android non aggiorna più, forse è un bug.
      //  Serial.println(fVal);
        Souliss_ImportAnalog(memory_map, SLOT_Watt, &fVal);
      } 

    }		
  } 


