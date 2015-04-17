#define  BOARDTYPE_INSKETCH
#define  QC_BOARDTYPE  0x02      /** Freaklabs Chibiduino with Ethernet Shield (W5100) */

#define  GATEWAYTYPE_INSKETCH
#define  QC_GATEWAYTYPE  0x01    // Define board as gateway

//DA compilare selezionando la scheda ARDUINO PRO OR PRO MINI  e processore ATMEGA328 3.3V 8 MHZ

/**************************************************************************
	Souliss - Bridge

	CONFIGURATION IS MANDATORY BEFORE COMPILING
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
#include "EmonLib.h"             // Include Emon Library
EnergyMonitor emon1;             // Create an instance

#define eth_bridge_address		0x0069
#define chibi_bridge_address      0x6511 //soggiorno
#define network_chibi_address_2	0x6512 //libero
#define network_chibi_address_3	0x6513 //giardino
#define network_chibi_address_4	0x6514 //tende

#define network_my_subnet		0xFF00
#define network_my_supern		0x0069

#define	DHT_id1					1			// Identify the sensor, in case of more than one used on the same board

#define TEMPERATURE				0			// This is the memory slot used for the execution of the logic in network_address1
#define HUMIDITY				2			// This is the memory slot used for the execution of the logic
#define SLOT_Watt                               4
#define SLOT_REMOTE_CONTROLLER                  6
#define SLOT_APRIPORTA                          7

#define DEADBAND				0.02		// Deadband value 5%  

#define     PIN_DHT     2
#define     PIN_OUTPUT_REMOTE_CONTROLLER     3
#define     PIN_OUTPUT_APRIPORTA             4
#define     timer_RemoteController 500
#define     timer_Apriporta         500

// define the shared memory map
U8 memory_map[MaCaco_MEMMAP];

// flag
U8 data_changed = 0;

#define time_base_fast		10				// Time cycle in milliseconds
#define time_base_slow		10000			// Time cycle in milliseconds
#define num_phases			255				// Number of phases

U8 phase_speedy = 0, phase_fast = 0, phase_slow = 0;
unsigned long tmr_fast = 0, tmr_slow = 0;

// Setup the DHT sensor
ssDHT22_Init(PIN_DHT, DHT_id1);

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

  // Set the typical to use
  Souliss_SetT52(memory_map, TEMPERATURE);
  Souliss_SetT53(memory_map, HUMIDITY);
  ssDHT_Begin(DHT_id1);

  Souliss_SetT57(memory_map, SLOT_Watt);
  emon1.voltage(15, 226, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current(14, 6);       // Current: input pin, calibration.


  pinMode(PIN_OUTPUT_REMOTE_CONTROLLER, OUTPUT); //PIN FOR PULSE REMOTE CONTROLLER
  Souliss_SetT11(memory_map, SLOT_REMOTE_CONTROLLER);
  pinMode(PIN_OUTPUT_APRIPORTA, OUTPUT); //PIN FOR PULSE SLOT_APRIPORTA
  Souliss_SetT11(memory_map, SLOT_APRIPORTA);

}

float fVal;
int tmr_prec_RemoteController = millis();
int tmr_prec_Apriporta = millis();

void loop()
{
  if (abs(millis() - tmr_fast) > time_base_fast)
  {
    tmr_fast = millis();
    phase_fast = (phase_fast + 1) % num_phases;

    if (!(phase_fast % 10))
    {
      Souliss_Logic_T11(memory_map, SLOT_REMOTE_CONTROLLER, &data_changed);
      Souliss_LowDigOut(PIN_OUTPUT_REMOTE_CONTROLLER, Souliss_T1n_Coil, memory_map, SLOT_REMOTE_CONTROLLER);
      Souliss_Logic_T11(memory_map, SLOT_APRIPORTA, &data_changed);
      Souliss_LowDigOut(PIN_OUTPUT_APRIPORTA, Souliss_T1n_Coil, memory_map, SLOT_APRIPORTA);
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

    if (!(phase_fast % 251)) {
      //emon1.calcVI(20,2000); //esegue il campionamento // Calculate all. No.of wavelengths, time-out
      emon1.calcVI(5, 200);
      fVal = emon1.realPower;
      if (abs(fVal) < 0.99) fVal = 0.01; //pongo a 0.01 perchè a 0.00 Android non aggiorna più, forse è un bug.
      Souliss_ImportAnalog(memory_map, SLOT_Watt, &fVal);

      // Compare the acquired input with the stored one, send the new value to the
      // user interface if the difference is greater than the deadband
      Souliss_Logic_T57(memory_map, SLOT_Watt, DEADBAND, &data_changed);

    }

    if (!(phase_fast % 53))
    {
      if ((memory_map[MaCaco_OUT_s + SLOT_REMOTE_CONTROLLER] == Souliss_T1n_OnCoil) && abs(millis() - tmr_prec_RemoteController) > timer_RemoteController)
      {
        //DISATTIVA RELE' RADIOCOMANDO CANCELLO
        data_changed = Souliss_TRIGGED;
        memory_map[MaCaco_OUT_s + SLOT_REMOTE_CONTROLLER] = Souliss_T1n_OffCoil;			// Switch off the output
        memory_map[MaCaco_IN_s + SLOT_REMOTE_CONTROLLER] = Souliss_T1n_RstCmd;			// Reset
        tmr_prec_RemoteController = millis();
      } else if (memory_map[MaCaco_OUT_s + SLOT_REMOTE_CONTROLLER] == Souliss_T1n_OffCoil )
      {
        tmr_prec_RemoteController = millis();
      }

      if ((memory_map[MaCaco_OUT_s + SLOT_APRIPORTA] == Souliss_T1n_OnCoil) && abs(millis() - tmr_prec_Apriporta) > timer_Apriporta)
      {
        //DISATTIVA RELE' APRIPORTA
        data_changed = Souliss_TRIGGED;
        memory_map[MaCaco_OUT_s + SLOT_APRIPORTA] = Souliss_T1n_OffCoil;			// Switch off the output
        memory_map[MaCaco_IN_s + SLOT_APRIPORTA] = Souliss_T1n_RstCmd;			// Reset
        tmr_prec_Apriporta = millis();
      } else if (memory_map[MaCaco_OUT_s + SLOT_APRIPORTA] == Souliss_T1n_OffCoil )
      {
        tmr_prec_Apriporta = millis();
      }
    }

    // Execute the code every 101 time_base_fast
    if (!(phase_fast % 101))
    {
      // Compare the acquired input with the stored one, send the new value to the
      // user interface if the difference is greater than the deadband
      Souliss_Logic_T52(memory_map, TEMPERATURE, DEADBAND, &data_changed);
      Souliss_Logic_T53(memory_map, HUMIDITY, DEADBAND, &data_changed);
    }

  }
  else if (abs(millis() - tmr_slow) > time_base_slow)
  {
    tmr_slow = millis();
    phase_slow = (phase_slow + 1) % num_phases;
    // Execute the code every 11 time_base_slow
    if (!(phase_slow % 11))
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
