/**0............

************************************************************************
Souliss Bridge 

Setting on QuickConf file:
QC_BOARDTYPE			0x40
QC_GATEWAYTYPE			0x01
DHT_SENSOR				0x01

***************************************************************************/
/*#
network_address_1             ----> Bridge Ethernet Card
network_chibi_address_1       ----> Onboard Chibi Wireless 
network_chibi_address_2       ----> Chibi Remote Node
 ***************************************************************************/
#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>
//*******************************************************************************
//*******************************************************************************
//*******************************************************************************
//*******************************************************************************
#include <SensuinoEth.h>
#include <SensuinoSerial.h>

SensuinoEth pf;
const long int device_id = 15357; // This is your device's ID
const unsigned long int feed_41614 = 41614; // This is the feed "dht22-souliss-temp"
//*******************************************************************************
//*******************************************************************************
//*******************************************************************************
//*******************************************************************************
#define network_address_1	0x0011
#define network_chibi_address_1      0x6511
#define network_chibi_address_2	0x6512

#define network_my_subnet	0xFF00
#define network_my_supern	0x0000

#define	DHT_id1					1			// Identify the sensor, in case of more than one used on the same board

#define TEMPERATURE				0			// This is the memory slot used for the execution of the logic in network_address1
#define HUMIDITY				2			// This is the memory slot used for the execution of the logic
#define DEADBAND				0.05		// Deadband value 5%  

// define the shared memory map
U8 memory_map[MaCaco_MEMMAP];

// flag 
U8 data_changed = 0;

#define time_base_fast		10				// Time cycle in milliseconds
#define time_base_slow		10000			// Time cycle in milliseconds
#define num_phases			255				// Number of phases

U8 phase_speedy=0, phase_fast=0, phase_slow=0;
unsigned long tmr_fast=0, tmr_slow=0;  

// Setup the DHT sensor
ssDHT22_Init(2, DHT_id1);

void setup()
{
 Serial.begin(9600);  
  // Load the address also in the memory_map
  Souliss_SetLocalAddress(memory_map, network_address_1);

  // Init the Modbus protocol, board act as Modbus slave
  //ModbusInit();

  Souliss_SetAddress(network_address_1, network_my_subnet, network_my_supern);	
  // Setup the network configuration	
  Souliss_SetAddress(network_chibi_address_1, network_my_subnet, network_my_supern);	
  // Set the addresses of the remote nodes
  Souliss_SetRemoteAddress(memory_map, network_chibi_address_2, 1);

  // Load the address also in the memory_map
  Souliss_SetLocalAddress(memory_map, network_address_1);	
  
  // Set the typical to use
	Souliss_SetT52(memory_map, TEMPERATURE);
	Souliss_SetT53(memory_map, HUMIDITY);
	
	ssDHT_Begin(DHT_id1);
}
int iNrNodi=1;
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

		// Execute the code every 7 time_base_fast		  
		if (!(phase_fast % 7))
		{   
			// Parse JSON requests
			JSONServer(memory_map);		
		}


    // Execute the code every 31 time_base_fast		  
    if (!(phase_fast % 31))
    {   
      // Get logic typicals once and at every refresh
      Souliss_GetTypicals(memory_map, iNrNodi);
    }

    // Execute the code every 51 time_base_fast		  
    if (!(phase_fast % 51))
    {   
      // Open a communication channel with remote nodes
      Souliss_CommunicationChannels(memory_map, iNrNodi);
    }
    // Execute the code every 101 time_base_fast		  
    if (!(phase_fast % 70))
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

    // Execute the code every 7 time_base_slow	 
    if (!(phase_slow % 7))
    {
      // Refresh logic typicals
      Souliss_RefreshTypicals();
    }	
    // Execute the code every 11 time_base_slow
    if (!(phase_slow % 1))
    {                 
      // Read temperature value from DHT sensor and convert from single-precision to half-precision
      float temperature = ssDHT_readTemperature(DHT_id1);
      Souliss_ImportAnalog(memory_map, TEMPERATURE, &temperature);
    // Serial.print("temperature: ");
    //  Serial.println( temperature);

      // Read humidity value from DHT sensor and convert from single-precision to half-precision
      float humidity = ssDHT_readHumidity(DHT_id1);
      Souliss_ImportAnalog(memory_map, HUMIDITY, &humidity);			
    //       Serial.print("humidity: ");
    // Serial.println( humidity);

    } 			
  }	
} 

