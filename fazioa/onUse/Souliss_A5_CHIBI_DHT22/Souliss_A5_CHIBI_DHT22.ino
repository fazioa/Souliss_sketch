#define  BOARDTYPE_INSKETCH
#define  QC_BOARDTYPE  0x01      /** Freaklabs Chibiduino */

#define  GATEWAYTYPE_INSKETCH
#define  QC_GATEWAYTYPE  0x00    // NO Define board as gateway
//DA compilare selezionando la scheda ARDUINO PRO OR PRO MINI  e processore ATMEGA328 3.3V 8 MHZ

/**************************************************************************
Souliss Bridge 

Setting on QuickConf file:
QC_BOARDTYPE			0x01
QC_GATEWAYTYPE			0x00
DHT_SENSOR				0x01
DHT on PIN 2
***************************************************************************/
#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>

#define network_chibi_address_3      0x6513
#define network_my_subnet	0xFF00
#define network_my_supern	0x0069

#define	DHT_id1					1			// Identify the sensor, in case of more than one used on the same board

#define TEMPERATURE				0			// This is the memory slot used for the execution of the logic in network_address1
#define HUMIDITY				2			// This is the memory slot used for the execution of the logic
#define DEADBAND				0.02		// Deadband value 5%  

// define the shared memory map
U8 memory_map[MaCaco_MEMMAP];

// flag 
U8 data_changed = 0;

#define time_base_fast		10				// Time cycle in milliseconds
#define time_base_slow		10000			// Time cycle in milliseconds
#define num_phases			255				// Number of phases

U8 phase_speedy=0, phase_fast= 0, phase_slow=0;
unsigned long tmr_fast=0, tmr_slow=0;  

// Setup the DHT sensor
ssDHT22_Init(2, DHT_id1);

void setup()
{
 //Serial.begin(9600);  
 
  Souliss_SetAddress(network_chibi_address_3, network_my_subnet, network_my_supern);	
    
  // Set the typical to use
	Souliss_SetT52(memory_map, TEMPERATURE);
	Souliss_SetT53(memory_map, HUMIDITY);
	
	ssDHT_Begin(DHT_id1);
}

void loop()
{ 

  if(abs(millis()-tmr_fast) > time_base_fast)
  {	
    tmr_fast = millis();
    phase_fast = (phase_fast + 1) % num_phases;

    // Execute the code every 7 time_base_fast		  
    if (!(phase_fast % 7))
    {   
      // Retreive data from the MaCaco communication channel
      Souliss_CommunicationData(memory_map, &data_changed);	
       	
    }

       // Execute the code every 101 time_base_fast		  
    if (!(phase_fast % 91))
    {
      // Compare the acquired input with the stored one, send the new value to the
      // user interface if the difference is greater than the deadband
      Souliss_Logic_T52(memory_map, TEMPERATURE, DEADBAND, &data_changed);
      Souliss_Logic_T53(memory_map, HUMIDITY, DEADBAND, &data_changed);			
    }

  }
  else if(abs(millis()-tmr_slow) > time_base_slow)
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

