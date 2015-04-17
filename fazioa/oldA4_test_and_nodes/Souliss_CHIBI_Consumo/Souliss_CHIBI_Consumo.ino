/**************************************************************************
 * 
 ***************************************************************************/
#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>
#include "EmonLib.h"             // Include Emon Library
EnergyMonitor emon1;             // Create an instance

#define network_chibi_address_2	0x6512
#define network_my_subnet	0xFF00
#define network_my_supern	0x0069

#define SLOT_Watt 0 // This is the memory slot 

#define DEADBAND				0.05		// Deadband value

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
  Serial.begin(9600);	
  Souliss_SetAddress(network_chibi_address_2, network_my_subnet, network_my_supern);		
  // Load the address also in the memory_map
  // Souliss_SetLocalAddress(memory_map, network_chibi_address_2);	

  // Set the typical to use in slot 0
  Souliss_SetT57(memory_map, SLOT_Watt);

  emon1.voltage(15, 224, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current(14, 10);       // Current: input pin, calibration.
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
      Souliss_Logic_T57(memory_map, SLOT_Watt, DEADBAND, &data_changed);

      // Acquire data from the microcontroller ADC
      emon1.calcVI(20,2000); 
      fVal =emon1.realPower;
      //        if(abs(fVal)<0.99) fVal=0.01; //...per evitare fluttuazioni del valore ad utenze spente. Pongo a 0.01 perchè a 0.00 Android non aggiorna più, forse è un bug.
        Serial.println(fVal);

      Souliss_ImportAnalog(memory_map, SLOT_Watt, &fVal);

    } 

  }		
} 



