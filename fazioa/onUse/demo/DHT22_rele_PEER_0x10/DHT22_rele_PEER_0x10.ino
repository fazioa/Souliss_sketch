// Configure the framework
#include "SoulissFramework.h"
#include "bconf/StandardArduino.h"          // Use a standard Arduino
#include "conf/ethW5100.h"                  // Ethernet through Wiznet W5100
//#include "conf/IPBroadcast.h"
#include "conf/Webhook.h"                   // Enable DHCP and DNS

#include "Souliss.h"
#include "Typicals.h"
#include <SPI.h>
#include "DHT.h"
#include "topics.h"


uint8_t Gateway_address[4]  = {192, 168, 20, 20};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {192, 168, 20, 1};

#define Gateway_vNet_address Gateway_address[3]              // The Gateway node has two address, one on the Ethernet side
#define	peer_address	0x10
#define	myvNet_subnet	0xFF00
#define	myvNet_supern	Gateway_vNet_address

#define PIN_DHT      2
#define PIN_T11_ONE_IN      3
#define PIN_T11_TWO_IN      4
#define PIN_T11_ONE_OUT      5
#define PIN_T11_TWO_OUT      6


#define DHTTYPE DHT22   // DHT 22  

#define TEMPERATURE				0			// This is the memory slot used for the execution of the logic in network_address1
#define HUMIDITY				2			// This is the memory slot used for the execution of the logic
#define SLOT_T11_ONE        4
#define SLOT_T11_TWO        5

#define DEADBAND				0.01		// Deadband value 1%  

// Initialize DHT sensor for normal 8mhz Arduino
DHT dht(PIN_DHT, DHTTYPE, 2);
uint16_t output16;
uint8_t valByteArray[2];

void setup()
{
  //Serial.begin(9600);

  Initialize();
  GetIPAddress();
  SetAddress(peer_address, myvNet_subnet, myvNet_supern);					// Address on the wireless interface

  // Set the typical to use
  Set_T52(TEMPERATURE);
  Set_T53(HUMIDITY);
  pinMode(PIN_DHT, INPUT);


  dht.begin();


  pinMode(PIN_T11_ONE_IN, INPUT_PULLUP);
  pinMode(PIN_T11_TWO_IN, INPUT_PULLUP);


  pinMode(PIN_T11_ONE_OUT, OUTPUT);
  digitalWrite(PIN_T11_ONE_OUT, LOW);

  pinMode(PIN_T11_TWO_OUT, OUTPUT);
  digitalWrite(PIN_T11_TWO_OUT, LOW);

  Set_SimpleLight(SLOT_T11_ONE);
  Set_SimpleLight(SLOT_T11_TWO);
}

void loop()
{

  EXECUTEFAST() {
    UPDATEFAST();

    FAST_50ms() {   // We process the logic and relevant input and output every 50 milliseconds
      LowDigIn(PIN_T11_ONE_IN, Souliss_T1n_ToggleCmd, SLOT_T11_ONE);            // Use the PIN_T11_ONE_IN as ON/OFF toggle command
      Logic_SimpleLight(SLOT_T11_ONE);                          // Drive the LED as per command
      DigOut(PIN_T11_ONE_OUT, Souliss_T1n_Coil, SLOT_T11_ONE);                // Use the PIN_T11_ONE_OUT to give power to the LED according to the logic

      LowDigIn(PIN_T11_TWO_IN, Souliss_T1n_ToggleCmd, SLOT_T11_TWO);            // Use the PIN_T11_TWO_IN as ON/OFF toggle command
      Logic_SimpleLight(SLOT_T11_TWO);                          // Drive the LED as per command
      DigOut(PIN_T11_TWO_OUT, Souliss_T1n_Coil, SLOT_T11_TWO);                // Use the PIN_T11_TWO_OUT to give power to the LED according to the logic

    }


    // Process every 510ms the logic that control the curtain
    FAST_510ms() {
      // Compare the acquired input with the stored one, send the new value to the
      // user interface if the difference is greater than the deadband
      Logic_T52(TEMPERATURE);
      Logic_T53(HUMIDITY);
    }
    // Process the communication
    FAST_PeerComms();
  }

    EXECUTESLOW() {
    UPDATESLOW();
    SLOW_10s() {

      // Read temperature value from DHT sensor and convert from single-precision to half-precision
      float temperature = dht.readTemperature();
      ImportAnalog(TEMPERATURE, &temperature);

      //PUBLISH
      float16(&output16, &temperature);
      valByteArray[0] = C16TO8L(output16);
      valByteArray[1] = C16TO8H(output16);
      publishdata(TEMPERATURE_TOPIC_NODE_DHT_RELE, valByteArray, 2);


      // Read humidity value from DHT sensor and convert from single-precision to half-precision
      float humidity = dht.readHumidity();
      ImportAnalog(HUMIDITY, &humidity);

  //PUBLISH
      float16(&output16, &humidity);
      valByteArray[0] = C16TO8L(output16);
      valByteArray[1] = C16TO8H(output16);
      publishdata(HUMIDITY_TOPIC_NODE_DHT_RELE, valByteArray, 2);

      Timer_SimpleLight(SLOT_T11_ONE);
      Timer_SimpleLight(SLOT_T11_TWO);
    }
  }
}

