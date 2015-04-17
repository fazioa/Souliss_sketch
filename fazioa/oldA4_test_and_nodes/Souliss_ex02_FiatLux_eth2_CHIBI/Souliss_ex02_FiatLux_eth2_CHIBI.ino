/**************************************************************************
	Souliss - Fiat Lux
	
	Fiat Lux is a latin phrase that means Let There Be Light, this example
	show how handle light via hardwired pushbutton geographically distributed
	over the home, the Android (or any other direct) user interface can also
	be used.
	
	It require an Ethernet board based on Wiznet W5100 or Microchip ENC28J60.
	
	Applicable for : 
			- Lights controller via relay,
			- Plugs and other ON/OFF devices.
	
	The first device has
		- A coil (relay or other) on Pin 8 and Pin 9
		- A couple of inputs on Pin 2 and 3, pulldown required
		
	The Remote Device has
		- A couple of inputs on Pin 2 and 3, pulldown required
  
	CONFIGURATION IS MANDATORY BEFORE COMPILING
	Before compiling this code, is mandatory the configuration of the framework
	this ensure the use of proper drivers based functionalities and requested
	communication interface.	
	
	Configuration files are located on /conf folder, is suggested to use this 
	code on one of the boards listed below, the code can also compile on other
	boards but may require modification on I/O definitions.	
	
	file "conf/ethUsrCfg.h"
        # define ETH_ENC28J60 1

        file "conf/hwBoards.h"
        # define BOARD_MODEL 0x01
        # define COMMS_MODEL 0x02

        file  "conf/vNetCfg.h"
        # define VNET_SUPERNODE 1
        # define VNET_MEDIA1_ENABLE 1 
        # define VNET_MEDIA2_ENABLE 1 
		
***************************************************************************/
#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>

#define network_address_1	0x0011
#define network_chibi_address_1 0x6511
#define network_chibi_address_2	0x6512


#define network_my_subnet	0xFF00
#define network_my_supern	0x0000

#define LIGHT1_NODE1			0			 
#define LIGHT2_NODE1			1			 
#define LIGHT1_NODE2			0			 
#define LIGHT2_NODE2			1	

// define the shared memory map
U8 memory_map[MaCaco_MEMMAP] = {0x00};

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
	//
	//	The vNet address is 12(hex) that is 18(dec), so the IP address is
	//	the DEFAULT_BASEIPADDRESS[] defined in ethUsrCfg.h plus 18 on last 
	//  octect. If DEFAULT_BASEIPADDRESS[] = {192, 168, 1, 0} the IP address
	//  for the board will be 192.168.1.18
	Souliss_SetAddress(network_chibi_address_2, network_my_subnet, network_my_supern);		


	// Set the typical to use in slot 0 and 1
	Souliss_SetT11(memory_map, LIGHT1_NODE2);
	Souliss_SetT11(memory_map, LIGHT2_NODE2);
	
	// Define inputs, outputs pins
	pinMode(2, INPUT);	// Hardware pulldown required
	pinMode(3, INPUT);	// Hardware pulldown required
}


void loop()
{ 
  
	if(abs(millis()-tmr_fast) > time_base_fast)
	{	
		tmr_fast = millis();
		phase_fast = (phase_fast + 1) % num_phases;

		// Execute the code every 3 time_base_fast		
		if (!(phase_fast % 3))
		{
  
  // Use Pin2 as ON/OFF command
			Souliss_DigIn(2, Souliss_T1n_ToogleCmd, memory_map, LIGHT1_NODE2);	
			Souliss_DigIn(3, Souliss_T1n_ToogleCmd, memory_map, LIGHT2_NODE2);
			
		
			// Make Pin2 and Pin3 remote inputs for node network_address_1
			if((Souliss_Input(memory_map, LIGHT1_NODE2) != 0x00) || (Souliss_Input(memory_map, LIGHT2_NODE2) != 0x00))
			{
				// Send data from SLOT1
				Souliss_RemoteInput(network_chibi_address_1, LIGHT1_NODE2, Souliss_Input(memory_map, LIGHT1_NODE2));
				Souliss_ResetInput(memory_map, LIGHT1_NODE2);
				
				// Send data from SLOT2
				Souliss_RemoteInput(network_chibi_address_1, LIGHT2_NODE2, Souliss_Input(memory_map, LIGHT2_NODE2));
				Souliss_ResetInput(memory_map, LIGHT2_NODE2);
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
