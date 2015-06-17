// Configure the framework
#include "bconf/Chibiduino_v1.h"			// Use a Chibiduino 2.4 GHz wireless board
#include "conf/ethW5100.h"                  // Ethernet through Wiznet W5100
#include "conf/Gateway.h"                   // The main node is the Gateway, we have just one node
#include "conf/SmallNetwork.h"                   // The main node is the Gateway, we have just one node

#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>
#include "DHT.h"
#include "EmonLib.h"             // Include Emon Library
EnergyMonitor emon1;             // Create an instance


// Define the network configuration according to your router settings
uint8_t ip_address[4]  = {192, 168, 1, 105};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {192, 168, 1, 1};
#define Gateway_address 0x6511              // The Gateway node has two address, one on the Ethernet side
                                            // and the other on the wireless one
#define myvNet_address  ip_address[3]       // The last byte of the IP address (77) is also the vNet address
#define myvNet_subnet   0xFF00
#define myvNet_supern   Gateway_address

#define chibi_bridge_address    0x6511 //soggiorno
#define peer_chibi_address_2	0x6512 //libero
#define peer_chibi_address_3	0x6513 //giardino
#define peer_chibi_address_4	0x6514 //tende

#define TEMPERATURE				0			// This is the memory slot used for the execution of the logic in network_address1
#define HUMIDITY				2			// This is the memory slot used for the execution of the logic
#define SLOT_Watt                               4
#define SLOT_REMOTE_CONTROLLER                  6
#define SLOT_APRIPORTA                          7

#define     PIN_DHT                     2
#define     PIN_OUTPUT_REMOTE_CONTROLLER     3
#define     PIN_OUTPUT_APRIPORTA             4

#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define DEADBAND				0.01		// Deadband value 1%  

// Initialize DHT sensor for normal 8mhz Arduino
DHT dht(PIN_DHT, DHTTYPE,2);

void setup()
{
  //Serial.begin(9600);
    Initialize();
       // Set network parameters
    Souliss_SetIPAddress(ip_address, subnet_mask, ip_gateway);
    SetAsGateway(myvNet_address);                                   // Set this node as gateway for SoulissApp  														// Set this node as gateway for SoulissApp
    Souliss_SetAddress(chibi_bridge_address, myvNet_subnet, myvNet_supern);	// Address on the wireless interface
  
    // This node as gateway will get data from the Peer
  SetAsPeerNode(peer_chibi_address_2, 1);
  SetAsPeerNode(peer_chibi_address_3, 2);
  SetAsPeerNode(peer_chibi_address_4, 3);

  // Set the typical to use
  Set_T52(TEMPERATURE);
  Set_T53(HUMIDITY);
  

  Set_T57(SLOT_Watt);
  emon1.voltage(15, 226, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current(14, 6);       // Current: input pin, calibration.

  pinMode(PIN_OUTPUT_REMOTE_CONTROLLER, OUTPUT); //PIN FOR PULSE REMOTE CONTROLLER
  Set_T14(SLOT_REMOTE_CONTROLLER);
  pinMode(PIN_OUTPUT_APRIPORTA, OUTPUT); //PIN FOR PULSE SLOT_APRIPORTA
  Set_T14(SLOT_APRIPORTA);
  pinMode(PIN_DHT, INPUT);
  
  dht.begin();
}

float fVal;

void loop()
{
   EXECUTEFAST() {
    UPDATEFAST();
    
    FAST_510ms() {
      // Compare the acquired input with the stored one, send the new value to the
      // user interface if the difference is greater than the deadband
      Logic_T52(TEMPERATURE);
      Logic_T53(HUMIDITY);
    }

    FAST_710ms(){
     Logic_T14(SLOT_REMOTE_CONTROLLER);
      Souliss_LowDigOut(PIN_OUTPUT_REMOTE_CONTROLLER, Souliss_T1n_Coil, memory_map, SLOT_REMOTE_CONTROLLER);
      Souliss_Logic_T14(memory_map, SLOT_APRIPORTA, &data_changed);
      Souliss_LowDigOut(PIN_OUTPUT_APRIPORTA, Souliss_T1n_Coil, memory_map, SLOT_APRIPORTA);
    }
    
    FAST_1110ms() {
      emon1.calcVI(5, 200);  //esegue il campionamento // Calculate all. No.of wavelengths, time-out
      fVal = emon1.realPower;
     // if (abs(fVal) < 0.99) fVal = 0.01; //pongo a 0.01 perchè a 0.00 Android non aggiorna più, forse è un bug.
      ImportAnalog(SLOT_Watt, &fVal);

      // Compare the acquired input with the stored one, send the new value to the
      // user interface if the difference is greater than the deadband
      Logic_T57(SLOT_Watt);
    }
    // This node does just networking, bridging the Peer node to the Ethernet network
    FAST_GatewayComms();
  }
  
   EXECUTESLOW() {
    UPDATESLOW();
   SLOW_10s(){
  
      // Read temperature value from DHT sensor and convert from single-precision to half-precision
      float temperature = dht.readTemperature();
      ImportAnalog(TEMPERATURE, &temperature);
      // Read humidity value from DHT sensor and convert from single-precision to half-precision
      float humidity = dht.readHumidity();
      ImportAnalog(HUMIDITY, &humidity);
    }
   }
}
