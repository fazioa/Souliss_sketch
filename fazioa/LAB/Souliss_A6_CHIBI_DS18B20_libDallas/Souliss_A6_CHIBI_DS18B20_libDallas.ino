#define  BOARDTYPE_INSKETCH
#define  QC_BOARDTYPE  0x01      /** Freaklabs Chibiduino */

#define  GATEWAYTYPE_INSKETCH
#define  QC_GATEWAYTYPE  0x00    // NO Define board as gateway

// FROM
// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>
#include <OneWire.h>
#include "DallasTemperature.h"

#define network_chibi_address_3	0x6512
#define network_my_subnet	0xFF00
#define network_my_supern	0x0069


#define TEMPERATURE				0
//#define TEMPERATURE				2
#define DEADBAND				0.02		// Deadband value 5%  

// define the shared memory map
U8 memory_map[MaCaco_MEMMAP];

// flag
U8 data_changed = 0;

#define time_base_fast		10				// Time cycle in milliseconds
#define time_base_slow		2000			// Time cycle in milliseconds
#define num_phases			255				// Number of phases
#define DSPIN       14

U8 phase_speedy = 0, phase_fast = 0, phase_slow = 0;
unsigned long tmr_fast = 0, tmr_slow = 0;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(DSPIN);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

void setup()
{
  Serial.begin(9600);

  Souliss_SetAddress(network_chibi_address_3, network_my_subnet, network_my_supern);

  // Set the typical to use
  Souliss_SetT52(memory_map, TEMPERATURE);
  //RIMUOVERE COMMENTI PER AGGIUNGERE IL SECONDO SENSORE ONEWIRE
//  Souliss_SetT52(memory_map, TEMPERATURE2);

  // Start up the library
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12.
}

void loop()
{

  if (abs(millis() - tmr_fast) > time_base_fast)
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
//    Souliss_Logic_T52(memory_map, TEMPERATURE2, DEADBAND, &data_changed);
    }

  }
  else if (abs(millis() - tmr_slow) > time_base_slow)
  {
    tmr_slow = millis();
    phase_slow = (phase_slow + 1) % num_phases;

    // Execute the code every 1 time_base_slow
    if (!(phase_slow % 1))
    {
      float temperature;
      sensors.requestTemperatures(); // Send the command to get temperatures
      // SENSORE 1
      temperature = sensors.getTempCByIndex(0);
      Souliss_ImportAnalog(memory_map, TEMPERATURE, &temperature);
      // SENSORE 2
      //temperature = sensors.getTempCByIndex(1); 
      //Souliss_ImportAnalog(memory_map, TEMPERATURE2, &temperature);
    }
  }


}
