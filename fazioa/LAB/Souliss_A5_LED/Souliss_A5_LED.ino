#define  BOARDTYPE_INSKETCH
#define  QC_BOARDTYPE  0x01      /** Freaklabs Chibiduino */

#define  GATEWAYTYPE_INSKETCH
#define  QC_GATEWAYTYPE  0x00    // NO Define board as gateway
//DA compilare selezionando la scheda ARDUINO PRO OR PRO MINI  e processore ATMEGA328 3.3V 8 MHZ

#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>

// network addresses
#define network_chibi_address	0x6512
#define network_my_subnet	0xFF00
#define network_my_supern	0x0069

// define the shared memory map
U8 memory_map[MaCaco_MEMMAP];
U8 cont=0;
// flag
U8 data_changed = 0;

#define time_base_fast		10				// Time cycle in milliseconds
#define time_base_slow		3000			// Time cycle in milliseconds
#define num_phases			255				// Number of phases

#define SLOT_T16                  2 //it take four slot
#define     PIN_RBG_INPUT 16
#define     PIN_RBG_R 3
#define     PIN_RBG_G 5
#define     PIN_RBG_B 6

#define SLOT_T19                  6 //it take two slot
#define     PIN_W_INPUT 18
#define     PIN_W 9

U8 phase_speedy = 0, phase_fast = 0, phase_slow = 0;
unsigned long tmr_fast = 0, tmr_slow = 0;

void setup()
{
  Serial.begin(9600);
  Souliss_SetAddress(network_chibi_address, network_my_subnet, network_my_supern);

  
  // Define inputs, outputs pins
  pinMode(PIN_RBG_INPUT, INPUT);					// Hardware pulldown required
  pinMode(PIN_RBG_R, OUTPUT);					// Power the LED
  pinMode(PIN_RBG_G, OUTPUT);					// Power the LED
  pinMode(PIN_RBG_B, OUTPUT);					// Power the LED
  Souliss_SetT16(memory_map, SLOT_T16);
  
  pinMode(PIN_W_INPUT, INPUT);
  pinMode(PIN_W, OUTPUT);					// Power the LED
  Souliss_SetT19(memory_map, SLOT_T19);
    
  
  //*****************************
}

void loop()
{
  // The Souliss methods are scheduled in phases, this allow load
  // balance and proper timing.

  if (abs(millis() - tmr_fast) > time_base_fast)
  {
    tmr_fast = millis();
    phase_fast = (phase_fast + 1) % num_phases;

    // Execute the code every 3 time_base_fast


    if (!(phase_fast % 11))
    {
       Souliss_DigIn2State(PIN_RBG_INPUT, Souliss_T1n_OnCmd, Souliss_T1n_OffCmd, memory_map, SLOT_T16);	
       Souliss_Logic_T16(memory_map, SLOT_T16, &data_changed);
       
        float valR =  (float) memory_map[MaCaco_OUT_s + SLOT_T16+1];
        float valG =  (float) memory_map[MaCaco_OUT_s + SLOT_T16+2];
        float valB =  (float) memory_map[MaCaco_OUT_s + SLOT_T16+3];
       
       Serial.print("R: ");
       Serial.print(valR);
       Serial.print(" G: ");
       Serial.print(valG);
       Serial.print(" B: ");
       Serial.println(valB);
       
       analogWrite(PIN_RBG_R, valR);
       analogWrite(PIN_RBG_G, valG);
       analogWrite(PIN_RBG_B, valB);
       
       
       Souliss_DigIn2State(PIN_W_INPUT, Souliss_T1n_OnCmd, Souliss_T1n_OffCmd, memory_map, SLOT_T19);	
       Souliss_Logic_T19(memory_map, SLOT_T19, &data_changed);
       float valW =  (float) memory_map[MaCaco_OUT_s + SLOT_T19+1];
       
       Serial.print("W: ");
       Serial.println(valW);
       
       analogWrite(PIN_W, valW);
    }

    //==================================================================================================
    //==================================================================================================
    // Execute the code every 5 time_base_fast
    if (!(phase_fast % 5))
    {
      // Retreive data from the MaCaco communication channel
      Souliss_CommunicationData(memory_map, &data_changed);
    }

    //==================================================================================================
    //==================================================================================================
    // Execute the code every 221 time_base_fast
    if (!(phase_fast % 221))
    {
     
    }

    else if (abs(millis() - tmr_slow) > time_base_slow)
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
