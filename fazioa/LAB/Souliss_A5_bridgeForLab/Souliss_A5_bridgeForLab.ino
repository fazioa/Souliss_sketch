#define  BOARDTYPE_INSKETCH
#define  QC_BOARDTYPE  0x02      /** Freaklabs Chibiduino with Ethernet Shield (W5100) */

#define  GATEWAYTYPE_INSKETCH
#define  QC_GATEWAYTYPE  0x01    // Define board as gateway

#define  MaCaco_DEBUG_INSKETCH
#define MaCaco_DEBUG  		0

#define	VNET_DEBUG_INSKETCH
#define VNET_DEBUG  		0
//DA compilare selezionando la scheda ARDUINO PRO OR PRO MINI  e processore ATMEGA328 3.3V 8 MHZ

/**************************************************************************
	Souliss - Bridge
  
	CONFIGURATION IS MsANDATORY BEFORE COMPILING
	Before compiling this code, is mandatory the configuration of the framework
	this ensure the use of proper drivers based functionalities and requested
	communication interface.	
	
	Configuration files are located on /conf folder, is suggested to use this 
	code on one of the boards listed below.	
	
	Run this code on one of the following boards:
	
		Board Conf Code			Board Model
		0x02					Freaklabs Chibiduino with Ethernet Shield		

	******************** Configuration Parameters *********************
	
		Configuration file		Parameter
		QuickCfg.h				#define	QC_ENABLE		0x01
		QuickCfg.h				#define	QC_BOARDTYPE		0x02
                                                        #define DYNAMICADDRESSING  	0x00
		
		QuickCfg.h				#define	QC_GATEWAYTYPE		0x03
       ***************************************************************************/	
       
#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>

#define eth_bridge_address		0x006A
#define chibi_bridge_address      0x6511

#define network_chibi_address_4	0x6524 //tende
#define network_chibi_address_3	0x6513 //giardino

#define network_my_subnet		0xFF00
#define network_my_supern		0x006A

#define SLOT_REMOTE_CONTROLLER                  0
#define SLOT_CONTACT_T13                  1
#define SLOT_T16                  2

#define     PIN_OUTPUT_REMOTE_CONTROLLER     2
#define     PIN_INPUT_CONTACT 7

#define     PIN_RBG_INPUT 3
#define     PIN_RBG_R 4
#define     PIN_RBG_G 5
#define     PIN_RBG_B 6

#define     timer_RemoteController 500
// define the shared memory map
U8 memory_map[MaCaco_MEMMAP];

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
	// Setup the network configuration	
	Souliss_SetAddress(chibi_bridge_address, network_my_subnet, network_my_supern);		
	Souliss_SetAddress(eth_bridge_address, network_my_subnet, network_my_supern);		  
	// Load the address also in the memory_map
	Souliss_SetLocalAddress(memory_map, eth_bridge_address);

	// Set the addresses of the remote nodes
      //  Souliss_SetRemoteAddress(memory_map, network_chibi_address_2, 1);
          Souliss_SetRemoteAddress(memory_map, network_chibi_address_3, 2);
        Souliss_SetRemoteAddress(memory_map, network_chibi_address_4, 1);
          
        pinMode(PIN_OUTPUT_REMOTE_CONTROLLER, OUTPUT); //PIN FOR PULSE REMOTE CONTROLLER
        Souliss_SetT11(memory_map, SLOT_REMOTE_CONTROLLER);
        
        pinMode(PIN_INPUT_CONTACT, INPUT);
        Souliss_SetT13(memory_map, SLOT_CONTACT_T13);
        //RGB
        Souliss_SetT16(memory_map, SLOT_T16);
        
        
        // Define inputs, outputs pins
	pinMode(PIN_RBG_INPUT, INPUT);					// Hardware pulldown required
	pinMode(PIN_RBG_R, OUTPUT);					// Power the LED
	pinMode(PIN_RBG_G, OUTPUT);					// Power the LED
	pinMode(PIN_RBG_B, OUTPUT);					// Power the LED
}


int tmr_prec_RemoteController=millis();
void loop()
{ 
 	if(abs(millis()-tmr_fast) > time_base_fast)
	{	
		tmr_fast = millis();
		phase_fast = (phase_fast + 1) % num_phases;
		
                if (!(phase_fast % 10))
		{
		        Souliss_Logic_T11(memory_map, SLOT_REMOTE_CONTROLLER, &data_changed);
			Souliss_DigOut(PIN_OUTPUT_REMOTE_CONTROLLER, Souliss_T1n_Coil, memory_map, SLOT_REMOTE_CONTROLLER);	

                        Souliss_DigIn2State(PIN_INPUT_CONTACT, Souliss_T1n_OnCmd, Souliss_T1n_OffCmd, memory_map, SLOT_CONTACT_T13);	
                        Souliss_Logic_T13(memory_map, SLOT_CONTACT_T13, &data_changed);
                        
                        Souliss_DigIn2State(PIN_RBG_INPUT, Souliss_T1n_BrightUp, Souliss_T1n_BrightDown, memory_map, SLOT_T16);	
                        Souliss_Logic_T16(memory_map, SLOT_T16, &data_changed);
                   
		} 

		// Execute the code every 7 time_base_fast		  
		if (!(phase_fast % 7))
		{   
 			// Retreive data from the MaCaco communication channel
			Souliss_CommunicationData(memory_map, &data_changed);		
		}

		// Execute the code every 31 time_base_fast		  
		if (!(phase_fast % 31))
		{   
			// Get logic typicals once and at every refresh
			Souliss_GetTypicals(memory_map);
		}
		
		// Execute the code every 51 time_base_fast		  
		if (!(phase_fast % 51))
		{   
			// Open a communication channel with remote nodes
			Souliss_CommunicationChannels(memory_map);
		}	
                 
                                  
                  if (!(phase_fast % 53))
                  {
                        if((memory_map[MaCaco_OUT_s + SLOT_REMOTE_CONTROLLER] == Souliss_T1n_OnCoil) && abs(millis()-tmr_prec_RemoteController) > timer_RemoteController)	
                        {
                  //DISATTIVA RELE' RADIOCOMANDO CANCELLO
                         data_changed=Souliss_TRIGGED;
                         memory_map[MaCaco_OUT_s + SLOT_REMOTE_CONTROLLER] = Souliss_T1n_OffCoil;			// Switch off the output
                         memory_map[MaCaco_IN_s + SLOT_REMOTE_CONTROLLER] = Souliss_T1n_RstCmd;			// Reset
                         tmr_prec_RemoteController = millis();
                         } else if(memory_map[MaCaco_OUT_s + SLOT_REMOTE_CONTROLLER] == Souliss_T1n_OffCoil )
                         {
                           tmr_prec_RemoteController = millis();
                         }
                   }
                   
                   // Execute the code every 101 time_base_fast		  
		if (!(phase_fast % 255)){
                        
	}	
        else if(abs(millis()-tmr_slow) > time_base_slow)
          {	
            tmr_slow = millis();
            phase_slow = (phase_slow + 1) % num_phases;
            // Execute the code every 11 time_base_slow
            if (!(phase_slow % 1))
            {                    
            
          } 			
          }
} 
}
