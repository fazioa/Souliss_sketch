#define  BOARDTYPE_INSKETCH
#define  QC_BOARDTYPE  0x01      /** Freaklabs Chibiduino */

#define  GATEWAYTYPE_INSKETCH
#define  QC_GATEWAYTYPE  0x00    // NO Define board as gateway
//DA compilare selezionando la scheda ARDUINO PRO OR PRO MINI  e processore ATMEGA328 3.3V 8 MHZ

/*
Using a monostable wall switch (press and spring return) or a
			software command from user interface, each press will toogle
			the output status.
				#define Souliss_T2n_CloseCmd		0x01
				#define Souliss_T2n_OpenCmd			0x02
				#define Souliss_T2n_StopCmd			0x03

		Command recap, using:
		-  1(hex) as command, CLOSE request (stop if opening)
		-  2(hex) as command, OPEN request (stop if closing)
		-  3(hex) as command, STOP request

		Output status:
		- 1(hex) for CLOSE,
		- 2(hex) for OPEN,
		- 3(hex) for STOP.
*/

#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>

// network addresses
#define network_chibi_address	0x6514
#define network_my_subnet	0xFF00
#define network_my_supern	0x0069

// define the shared memory map
U8 memory_map[MaCaco_MEMMAP];
U8 cont=0;
// flag
U8 data_changed = 0;

#define time_base_fast		10				// Time cycle in milliseconds
#define time_base_slow		10000			// Time cycle in milliseconds
#define num_phases			255				// Number of phases

#define CURTAIN_slot1			0
#define CURTAIN_slot2			1
#define CURTAIN_slot3			2
#define T1A_slot			5

#define firstCurtainPIN       2
#define iPin_Pulsante_Uno 14
#define iPin_Pulsante_Due 15
#define iPin_Pulsante_Tre 16

U8 phase_speedy = 0, phase_fast = 0, phase_slow = 0;
unsigned long tmr_fast = 0, tmr_slow = 0;

//è il primo slot dei relè usati per il tipico 11
#define releFirstSLOT 3;
//pin usati per i relè avanzati. (degli 8, 6 sono usati per le tende)
#define firstPIN 8
#define lastPin 9

void setup()
{
  Serial.begin(9600);
  Souliss_SetAddress(network_chibi_address, network_my_subnet, network_my_supern);

  //SETUP LUCI
  //*****************************
  U8 iSLOT = releFirstSLOT;
  for (U8 iPin = firstPIN; iPin <= lastPin; iPin++)
  {
    pinMode(iPin, OUTPUT);
    digitalWrite(iPin, HIGH);
    Souliss_SetT11(memory_map, iSLOT++);
  }
  //*****************************
  
Souliss_SetT1A(memory_map, T1A_slot);

  //SETUP TENDA 1, 2 ,3
  //*****************************
  U8 iPin = firstCurtainPIN;

  //dichiara 3 tende ed i pin in sequenza dal 2 sino al 7
  Souliss_SetT22(memory_map, CURTAIN_slot1);
  pinMode(iPin, OUTPUT);
  pinMode(++iPin, OUTPUT);
  pinMode(iPin_Pulsante_Uno, INPUT); //SU QUESTO PIN VENGONO GESTITUI GLI STATI ALTO, BASSO, INTERMEDIO


  Souliss_SetT22(memory_map, CURTAIN_slot2);
  pinMode(++iPin, OUTPUT);
  pinMode(++iPin, OUTPUT);
  pinMode(iPin_Pulsante_Due, INPUT); //SU QUESTO PIN VENGONO GESTITUI GLI STATI ALTO, BASSO, INTERMEDIO


  Souliss_SetT22(memory_map, CURTAIN_slot3);
  pinMode(++iPin, OUTPUT);
  pinMode(++iPin, OUTPUT);
  pinMode(iPin_Pulsante_Tre, INPUT); //SU QUESTO PIN VENGONO GESTITUI GLI STATI ALTO, BASSO, INTERMEDIO

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
      // Use OPEN and CLOSE Commands

      Souliss_AnalogIn2Buttons(iPin_Pulsante_Uno, Souliss_T2n_OpenCmd_Local, Souliss_T2n_CloseCmd_Local, memory_map, CURTAIN_slot1);
      Souliss_AnalogIn2Buttons(iPin_Pulsante_Due, Souliss_T2n_OpenCmd_Local, Souliss_T2n_CloseCmd_Local, memory_map, CURTAIN_slot2);
      Souliss_AnalogIn2Buttons(iPin_Pulsante_Tre, Souliss_T2n_OpenCmd_Local, Souliss_T2n_CloseCmd_Local, memory_map, CURTAIN_slot3);

      //*****************************

      // LOGICA LUCI
      //*****************************
      U8 iSLOT;
      iSLOT = releFirstSLOT;
      for (U8 iPin = firstPIN; iPin <= lastPin; iPin++)
      {
        Souliss_Logic_T11(memory_map, iSLOT, &data_changed);
        Souliss_LowDigOut(iPin, Souliss_T1n_Coil, memory_map, iSLOT);
        iSLOT++;
      }
      
      

      
      
      // LOGICA TENDA 1, 2, 3
      //*****************************
      Souliss_Logic_T22(memory_map, CURTAIN_slot1, &data_changed);
      Souliss_Logic_T22(memory_map, CURTAIN_slot2, &data_changed);
      Souliss_Logic_T22(memory_map, CURTAIN_slot3, &data_changed);
      //*****************************

      U8 iPin = firstCurtainPIN;
      //tenda 1
      Souliss_LowDigOut(iPin, Souliss_T2n_Coil_Open, memory_map, CURTAIN_slot1);
      Souliss_LowDigOut(++iPin, Souliss_T2n_Coil_Close, memory_map, CURTAIN_slot1);
      //tenda 2
      Souliss_LowDigOut(++iPin, Souliss_T2n_Coil_Open, memory_map, CURTAIN_slot2);
      Souliss_LowDigOut(++iPin, Souliss_T2n_Coil_Close, memory_map, CURTAIN_slot2);
      //tenda 3
      Souliss_LowDigOut(++iPin, Souliss_T2n_Coil_Open, memory_map, CURTAIN_slot3);
      Souliss_LowDigOut(++iPin, Souliss_T2n_Coil_Close, memory_map, CURTAIN_slot3);
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
      //                  // Define the hold time for the outputs
      Souliss_T22_Timer(memory_map, CURTAIN_slot1);
      Souliss_T22_Timer(memory_map, CURTAIN_slot2);
      Souliss_T22_Timer(memory_map, CURTAIN_slot3);
    }

    else if (abs(millis() - tmr_slow) > time_base_slow)
    {
      tmr_slow = millis();
      phase_slow = (phase_slow + 1) % num_phases;
      // Execute the code every 11 time_base_slow
      
      if (!(phase_slow % 1))
      {
      Souliss_Logic_T1A(memory_map, T1A_slot, &data_changed);
      ++cont;
      if( memory_map[MaCaco_OUT_s + T1A_slot] != cont ) {
	memory_map[MaCaco_OUT_s + T1A_slot] = cont;
	data_changed = Souliss_TRIGGED;
      } 
      Serial.println(cont);
      }
    }


  }

}
