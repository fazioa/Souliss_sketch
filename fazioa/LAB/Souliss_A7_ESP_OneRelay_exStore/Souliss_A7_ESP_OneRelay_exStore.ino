#define HOST_NAME_INSKETCH
#define HOST_NAME "ESP8266-WiFi-Relay-V3"

/**************************************************************************
Sketch: POWER SOCKET - VER.1 - Souliss - Web Configuration
Author: Tonino Fazio

ESP Core 1.6.5 Staging 1.6.5-1160-gef26c5f
 This example is only supported on ESP8266.
***************************************************************************/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Configure the Souliss framework
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/RuntimeGateway.h"            // This node is a Peer and can became a Gateway at runtime
#include "conf/DynamicAddressing.h"         // Use dynamically assigned addresses
#include "conf/WEBCONFinterface.h"          // Enable the WebConfig interface

#include "Souliss.h"
#include "DHT.h"
//*************************************************************************
//*************************************************************************

/************************* Adafruit.io Setup *********************************/

//#define AIO_SERVER      "io.adafruit.com"
//#define AIO_SERVERPORT  1883
//#define AIO_USERNAME    "...your AIO username (see https://accounts.adafruit.com)..."
#define AIO_USERNAME    "ESP8266"
//#define AIO_KEY         "...your AIO key..."

/************************* iot.eclipse.org Setup *********************************/
#define AIO_SERVER      "192.168.1.121"
#define AIO_SERVERPORT  1883

// Store the MQTT server, client ID, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
//const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;
// Set a unique MQTT client ID using the AIO key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
//const char MQTT_CLIENTID[] PROGMEM  = __TIME__ AIO_USERNAME;
//const char MQTT_CLIENTID[] PROGMEM  = __TIME__;
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = "";

const char MQTT_CLIENTID[] PROGMEM  = __TIME__ AIO_USERNAME;
const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);
Adafruit_MQTT_Publish MQTTtemperature = Adafruit_MQTT_Publish(&mqtt, "/feeds/temperature");
Adafruit_MQTT_Publish MQTTrelay0 = Adafruit_MQTT_Publish(&mqtt, "/feeds/relay0");



#define SLOT_RELAY_0 0
#define SLOT_TEMPERATURE        1     // This is the memory slot used for the execution of the logic in network_address1
#define SLOT_HUMIDITY        3     // This is the memory slot used for the execution of the logic
//#define SLOT_RELAY_1 1
//#define SLOT_RELAY_2 2
//#define SLOT_RELAY_3 3
//#define SLOT_RELAY_4 4
//#define SLOT_RELAY_5 5
//#define SLOT_RELAY_6 6
//#define SLOT_RELAY_7 7
//#define SLOT_RELAY_8 8
//#define SLOT_RELAY_9 9


//#define PIN_0 0
//#define PIN_2 2
//#define PIN_4 4
//#define PIN_5 5
#define PIN_14 14
//#define PIN_16 16
//
//#define PIN_1 1
#define PIN_12 12
#define PIN_13 13
//#define PIN_15 15

#define PIN_DHT      16
#define DHTTYPE DHT22   // DHT 22 
#define DEADBAND        0.01    // Deadband value 1% 
// Initialize DHT sensor for normal 8mhz Arduino
DHT dht(PIN_DHT, DHTTYPE, 2);

// Setup the libraries for Over The Air Update
OTA_Setup();
void setup()
{
  Serial.begin(115200);
  //delay 30 seconds
  delay(15000);
  Initialize();
  Serial.println(F("Inizialize OK"));
  // Read the IP configuration from the EEPROM, if not available start
  // the node as access point
  if (!ReadIPConfiguration())
  {
    // Start the node as access point with a configuration WebServer
    SetAccessPoint();
    startWebServer();
    Serial.println(F("startWebServer OK"));
    // We have nothing more than the WebServer for the configuration
    // to run, once configured the node will quit this.
    while (1)
    {
      yield();
      runWebServer();
      Serial.println(F("runWebServer"));
    }

  }

  if (IsRuntimeGateway())
  {
    // Connect to the WiFi network and get an address from DHCP
    SetAsGateway(myvNet_dhcp);       // Set this node as gateway for SoulissApp
    SetAddressingServer();
    Serial.println(F("SetAsGateway OK"));
  }
  else
  {
    // This board request an address to the gateway at runtime, no need
    // to configure any parameter here.
    SetDynamicAddressing();
    GetAddress();
    Serial.println(F("SetAsPeer OK"));
  }

  //*************************************************************************
  //*************************************************************************
  Set_SimpleLight(SLOT_RELAY_0);
  Set_Temperature(SLOT_TEMPERATURE);
  Set_Humidity(SLOT_HUMIDITY);
  pinMode(PIN_DHT, INPUT);
  dht.begin();
  // Set_SimpleLight(SLOT_RELAY_1);
  // Set_SimpleLight(SLOT_RELAY_2);
  // Set_SimpleLight(SLOT_RELAY_3);
  //  Set_SimpleLight(SLOT_RELAY_4);
  //  Set_SimpleLight(SLOT_RELAY_5);

  //  Set_SimpleLight(SLOT_RELAY_6);
  // Set_SimpleLight(SLOT_RELAY_7);
  // Set_SimpleLight(SLOT_RELAY_8);
  //  Set_SimpleLight(SLOT_RELAY_9);

  // Define output pins
  //  pinMode(PIN_0, OUTPUT);    // Relè
  //  pinMode(PIN_2, OUTPUT);    // Relè
  //  pinMode(PIN_4, OUTPUT);    // Relè
  //  pinMode(PIN_5, OUTPUT);    // Relè
  pinMode(PIN_14, INPUT_PULLUP);    // Relè

  //  pinMode(PIN_16, OUTPUT);    // Relè
  //
  //  pinMode(PIN_1, OUTPUT);    // Relè

  digitalWrite(PIN_12, LOW);
  pinMode(PIN_12, OUTPUT);    // Relè
  digitalWrite(PIN_13, LOW);
  pinMode(PIN_13, OUTPUT);    // Relè
  //  pinMode(PIN_15, OUTPUT);    // Relè

  Serial.println(F("Set Typical OK"));
  // Init the OTA
  OTA_Init();
  Serial.println(F("OTA_Init OK"));
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    MQTT_connect();

    FAST_50ms() {
      DigIn2State(PIN_14, Souliss_T1n_ToggleCmd, Souliss_T1n_ToggleCmd, SLOT_RELAY_0);
      Logic_SimpleLight(SLOT_RELAY_0);
      PulseDigOut(PIN_12, Souliss_T1n_OnCoil, SLOT_RELAY_0);
      PulseDigOut(PIN_13, Souliss_T1n_OffCoil, SLOT_RELAY_0);

      //
      //      Logic_SimpleLight(SLOT_RELAY_1);
      //      DigOut(PIN_2, Souliss_T1n_Coil, SLOT_RELAY_1);
      //
      //      Logic_SimpleLight(SLOT_RELAY_2);
      //      DigOut(PIN_4, Souliss_T1n_Coil, SLOT_RELAY_2);
      //
      //      Logic_SimpleLight(SLOT_RELAY_3);
      //      DigOut(PIN_5, Souliss_T1n_Coil, SLOT_RELAY_3);
      //
      //      Logic_SimpleLight(SLOT_RELAY_4);
      //      DigOut(PIN_14, Souliss_T1n_Coil, SLOT_RELAY_4);
      //
      //      Logic_SimpleLight(SLOT_RELAY_5);
      //      DigOut(PIN_16, Souliss_T1n_Coil, SLOT_RELAY_5);
      //
      //      Logic_SimpleLight(SLOT_RELAY_6);
      //      DigOut(PIN_1, Souliss_T1n_Coil, SLOT_RELAY_6);
      //
      //      Logic_SimpleLight(SLOT_RELAY_7);
      //      DigOut(PIN_12, Souliss_T1n_Coil, SLOT_RELAY_7);
      //
      //      Logic_SimpleLight(SLOT_RELAY_8);
      //      DigOut(PIN_13, Souliss_T1n_Coil, SLOT_RELAY_8);

      //      Logic_SimpleLight(SLOT_RELAY_9);
      //      DigOut(PIN_15, Souliss_T1n_Coil, SLOT_RELAY_9);

    }

    FAST_510ms() {
      Logic_Temperature(SLOT_TEMPERATURE);
      Logic_Humidity(SLOT_HUMIDITY);
    }


    // Run communication as Gateway or Peer
    if (IsRuntimeGateway())
      FAST_GatewayComms();
    else
      FAST_PeerComms();
  }

  EXECUTESLOW() {
    UPDATESLOW();
    SLOW_10s() {  // Process the timer every 10 seconds
      Timer_SimpleLight(SLOT_RELAY_0);
      // Read temperature value from DHT sensor and convert from single-precision to half-precision
      float temperature = dht.readTemperature();
      ImportAnalog(SLOT_TEMPERATURE, &temperature);

      // Read humidity value from DHT sensor and convert from single-precision to half-precision
      float humidity = dht.readHumidity();
      ImportAnalog(SLOT_HUMIDITY, &humidity);

      if (!MQTTtemperature.publish(temperature)) {
        Serial.println(F("Failed"));
      } else {
        Serial.print(".publish(temperature): "); Serial.println(temperature);
      }

    }
    // If running as Peer
    if (!IsRuntimeGateway())
      SLOW_PeerJoin();
  }
  // Look for a new sketch to update over the air
  OTA_Process();
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}
