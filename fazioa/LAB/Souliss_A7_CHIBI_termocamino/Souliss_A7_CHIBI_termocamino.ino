//NODO per la gestione di un termocamino e di un boiler solare
//Il nodo è costruito con scheda Chibiduino, con due sonde di temperatura DS18B20 ed una scheda a 2 relè.
//Il primo relè viene eccitato soltanto quando la temperatura della sonda "termocamino" supera la temperatura della sonda "boiler".
//Il secondo relè viene eccitato quando la temperatura della sonta "termocamino" supera la temperatura di setpoint, impostata di default a 85°C
//AUTORE: Antonino Fazio - giugno 2015

// FROM
// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

#include "bconf/Chibiduino_v1.h"			// Use a Chibiduino 2.4 GHz wireless board
#include "conf/SmallNetwork.h"                   // The main node is the Gateway, we have just one node

#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>
#include <OneWire.h>
#include "DallasTemperature.h"


#define SLOT_TEMPERATURE_BOILER      	0
#define SLOT_TEMPERATURE_TERMOCAMINO	        2
#define SLOT_SWITCH_BOILER_TERMOCAMINO       4
//#define SLOT_SWITCH_ALLARME_TERMOCAMINO       5
#define SLOT_SWITCH_AUTOMODE       6
#define SLOT_T31_THERMOSTAT          7

#define PIN_TEMPERATURE_ONEWIRE        14
#define PIN_SWITCH_BOILER_TERMOCAMINO       5
#define PIN_SWITCH_ALLARME_TERMOCAMINO      6

#define DEADBAND_TEMP      0.5
#define ALARM_TEMP      85

// Define the network configuration according to your router settingsuration according to your router settings
#define	Gateway_address	0x6511				// The Gateway node has two address, one on the Ethernet side 69				// The Gateway node has two address, one on the Ethernet side
// and the other on the wireless oneless one
#define	Peer_address	0x6512
#define	myvNet_subnet	0xFF00
#define	myvNet_supern	Gateway_address

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(PIN_TEMPERATURE_ONEWIRE);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

float temp_boiler, temp_termocamino;
void setup()
{
  Serial.begin(9600);

  Initialize();
  // Setup the network configuration
  Souliss_SetAddress(Peer_address, myvNet_subnet, myvNet_supern);					// Address on the wireless interface
  // Set the typical to use
  Set_T52(SLOT_TEMPERATURE_BOILER);
  Set_T52(SLOT_TEMPERATURE_TERMOCAMINO);
  Set_T11(SLOT_SWITCH_BOILER_TERMOCAMINO);
  // Set_T11(SLOT_SWITCH_ALLARME_TERMOCAMINO);
  Set_T11(SLOT_SWITCH_AUTOMODE);

  Set_Thermostat(SLOT_T31_THERMOSTAT);
   
  U8 setpoint = ALARM_TEMP;
  set_Setpoint(SLOT_T31_THERMOSTAT, setpoint);
  set_ThermostatMode(SLOT_T31_THERMOSTAT);


  Serial.println("Setup Automode On ");
  setT11_State(SLOT_SWITCH_AUTOMODE, Souliss_T1n_OnCoil);

  pinMode(PIN_TEMPERATURE_ONEWIRE, INPUT);
  pinMode(PIN_SWITCH_BOILER_TERMOCAMINO, OUTPUT);
  pinMode(PIN_SWITCH_ALLARME_TERMOCAMINO, OUTPUT);


  // Start up the library
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12.
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();
    FAST_50ms() {
      Logic_T52(SLOT_TEMPERATURE_BOILER);
      Logic_T52(SLOT_TEMPERATURE_TERMOCAMINO);

      Logic_T11(SLOT_SWITCH_BOILER_TERMOCAMINO);
      DigOut(PIN_SWITCH_BOILER_TERMOCAMINO, Souliss_T1n_Coil, SLOT_SWITCH_BOILER_TERMOCAMINO);

      // Logic_T11(SLOT_SWITCH_ALLARME_TERMOCAMINO);
      // DigOut(PIN_SWITCH_ALLARME_TERMOCAMINO, Souliss_T1n_Coil, SLOT_SWITCH_ALLARME_TERMOCAMINO);

      Logic_T11(SLOT_SWITCH_AUTOMODE);

      Logic_Thermostat(SLOT_T31_THERMOSTAT);
      nLowDigOut(PIN_SWITCH_ALLARME_TERMOCAMINO, Souliss_T3n_HeatingOn, SLOT_T31_THERMOSTAT);    // Heater
    }

    FAST_2110ms()	{
      sensors.requestTemperatures(); // Send the command to get temperatures
      // SENSORE 1
      temp_boiler = sensors.getTempCByIndex(1);
      ImportAnalog(SLOT_TEMPERATURE_BOILER, &temp_boiler);
      Serial.print("temp_boiler ");
      Serial.println(temp_boiler);

      // SENSORE 2
      temp_termocamino = sensors.getTempCByIndex(0);
      ImportAnalog(SLOT_TEMPERATURE_TERMOCAMINO, &temp_termocamino);
      ImportAnalog(SLOT_T31_THERMOSTAT + 1, &temp_termocamino);
      Serial.print("temp_termocamino ");
      Serial.println(temp_termocamino);
    }

    FAST_7110ms() {
      //attiva il relè se il termocamino ha temperatura maggiore.
      if (getT11_State(SLOT_SWITCH_AUTOMODE) == Souliss_T1n_OnCoil) {
        if ( abs(temp_boiler - temp_termocamino) > DEADBAND_TEMP) {
          Serial.print("Fuori deadband. Diff temp_boiler - temp_termocamino= ");
          Serial.println(temp_boiler - temp_termocamino);
          if (temp_termocamino > temp_boiler) {
            //azionare RELE'1
            Serial.println("Temp.termocamino maggiore");
            setT11_State(SLOT_SWITCH_BOILER_TERMOCAMINO, Souliss_T1n_OnCoil);
          } else {
            //rilasciare RELE'1
            Serial.println("Temp.boiler maggiore");
            setT11_State(SLOT_SWITCH_BOILER_TERMOCAMINO, Souliss_T1n_OffCoil);
          }
        }
      }
    }
    // Process the communication
    FAST_PeerComms();
  }
}

void setT11_State(U8 slot, U8 value) {
  if ( memory_map[MaCaco_OUT_s + slot] != value ) {
    memory_map[MaCaco_OUT_s + slot] = value;
    data_changed = Souliss_TRIGGED;
    Serial.print("SLOT: ");
    Serial.println(slot);
    Serial.print("VALUE: ");
    Serial.println(value);
  }
}

U8 getT11_State(U8 slot) {
  return memory_map[MaCaco_OUT_s + slot];
}

void set_Setpoint(U8 slot, float input) {
  Souliss_HalfPrecisionFloating((memory_map + MaCaco_OUT_s + slot + 3), &input);
}

void set_ThermostatMode(U8 slot) {
  memory_map[MaCaco_OUT_s + slot] |= Souliss_T3n_HeatingMode | Souliss_T3n_SystemOn;
}
