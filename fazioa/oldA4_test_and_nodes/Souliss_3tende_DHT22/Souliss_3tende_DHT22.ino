#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>

// network addresses
#define network_chibi_address_3      0x6513
#define network_my_subnet	0xFF00
#define network_my_supern	0x0069

// define the shared memory map
U8 memory_map[MaCaco_MEMMAP];

// flag 
U8 data_changed = 0;

#define time_base_fast		10				// Time cycle in milliseconds
#define time_base_slow		10000			// Time cycle in milliseconds
#define num_phases			255				// Number of phases

#define CURTAIN_slot1			0			
#define CURTAIN_slot2			1
#define CURTAIN_slot3			2
//è il primo slot dei relè usati per il tipico 11
#define releFirstSLOT 7;
#define firstCurtainPIN       2
//PIN usato dal sensore DHT22
#define DHT22PIN       14

#define	DHT_id1					1			// Identify the sensor, in case of more than one used on the same board

#define TEMPERATURE				3			// This is the memory slot used for the execution of the logic in network_address1
#define HUMIDITY				5			// This is the memory slot used for the execution of the logic
#define DEADBAND				0.02		// Deadband value 5% 

U8 phase_speedy=0, phase_fast=0, phase_slow=0;
unsigned long tmr_fast=0, tmr_slow=0;  


//primo pin dei relè che avanzano dagli otto usati dalle tende
//firstPIN è usato dal termostato
#define firstPIN 8
#define lastPin 8


// Setup the DHT sensor
ssDHT22_Init(DHT22PIN, DHT_id1);
void setup()
{	
	Souliss_SetAddress(network_chibi_address_3, network_my_subnet, network_my_supern);		
	// Load the address also in the memory_map
	Souliss_SetLocalAddress(memory_map, network_chibi_address_3);	

//SETUP LUCI
//*****************************
U8 iSLOT=releFirstSLOT;
for(U8 iPin=firstPIN;iPin<=lastPin;iPin++)
{
 pinMode(iPin, OUTPUT);
 digitalWrite(iPin, HIGH);
 Souliss_SetT11(memory_map,iSLOT++);
}


//SETUP TENDA 1, 2 ,3
//*****************************
U8 iPin=firstCurtainPIN;
//dichiara 3 tende ed i pin in sequenza dal 2 sino al 7
//TENDA 1
Souliss_SetT22(memory_map, CURTAIN_slot1);
 pinMode(iPin, OUTPUT);
  //digitalWrite(iPin, HIGH);
 pinMode(++iPin, OUTPUT);
  //digitalWrite(iPin, HIGH);

//TENDA 2
Souliss_SetT22(memory_map, CURTAIN_slot2);
 pinMode(++iPin, OUTPUT);
  //digitalWrite(iPin, HIGH);
 pinMode(++iPin, OUTPUT);
  //digitalWrite(iPin, HIGH);

//TENDA 3
Souliss_SetT22(memory_map, CURTAIN_slot3);
 pinMode(++iPin, OUTPUT);
  //digitalWrite(iPin, HIGH);
 pinMode(++iPin, OUTPUT);
  //digitalWrite(iPin, HIGH);
//*****************************

//SENSORE DHT22
	Souliss_SetT52(memory_map, TEMPERATURE);
	Souliss_SetT53(memory_map, HUMIDITY);
        ssDHT_Begin(DHT_id1);
        
//*****************************

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
			// Use Pin2 as ON command, Pin3 as OFF command
			//Souliss_DigIn(2, Souliss_T1n_ToogleCmd, memory_map, LEDLOGICNO);		
			
			// Execute the logic that handle the LED
      
      // LOGICA LUCI (TIPICO 11 USATO PER CALDAIA
            //*****************************
       U8 iSLOT;
       iSLOT=releFirstSLOT;
	for(U8 iPin=firstPIN;iPin<=lastPin;iPin++)
        {		
              Souliss_Logic_T11(memory_map, iSLOT, &data_changed);
              Souliss_LowDigOut(iPin, Souliss_T1n_Coil, memory_map, iSLOT);	
              iSLOT++;
        }
      //*****************************


      // LOGICA TENDA 1, 2, 3
      //*****************************
        Souliss_Logic_T22(memory_map, CURTAIN_slot1, &data_changed);
        Souliss_Logic_T22(memory_map, CURTAIN_slot2, &data_changed);
        Souliss_Logic_T22(memory_map, CURTAIN_slot3, &data_changed);
        //*****************************
        
        U8 iPin=firstCurtainPIN;
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
      //*****************************		

		// Execute the code every 5 time_base_fast		  
		if (!(phase_fast % 5))
		{   
			// Retreive data from the communication channel
			Souliss_CommunicationData(memory_map, &data_changed);
		}			
	
       // Execute the code every 101 time_base_fast		  
    if (!(phase_fast % 70))
    {
      // Compare the acquired input with the stored one, send the new value to the
      // user interface if the difference is greater than the deadband
      Souliss_Logic_T52(memory_map, TEMPERATURE, DEADBAND, &data_changed);
      Souliss_Logic_T53(memory_map, HUMIDITY, DEADBAND, &data_changed);		
     
    }

}else if(abs(millis()-tmr_slow) > time_base_slow)
  {	
    tmr_slow = millis();
    phase_slow = (phase_slow + 1) % num_phases;

      // Execute the code every 1 time_base_slow
    if (!(phase_slow % 1))
    {                 
      // Read temperature value from DHT sensor and convert from single-precision to half-precision
      float temperature = ssDHT_readTemperature(DHT_id1);
      Souliss_ImportAnalog(memory_map, TEMPERATURE, &temperature);
           
      // Read humidity value from DHT sensor and convert from single-precision to half-precision
      float humidity = ssDHT_readHumidity(DHT_id1);
      Souliss_ImportAnalog(memory_map, HUMIDITY, &humidity);			
      } 			
  }	
} 

