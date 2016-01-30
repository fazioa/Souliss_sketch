#define HOST_NAME_INSKETCH
#define HOST_NAME "ESP8266-WiFi-Relay-V3"

/**************************************************************************
Sketch: POWER SOCKET - VER.1 - Souliss - Web Configuration
Author: Tonino Fazio

ESP Core 1.6.5 Staging 1.6.5-1160-gef26c5f
 This example is only supported on ESP8266.
***************************************************************************/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <WiFiUdp.h>

// Configure the Souliss framework
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/RuntimeGateway.h"            // This node is a Peer and can became a Gateway at runtime
#include "conf/DynamicAddressing.h"         // Use dynamically assigned addresses
#include "conf/WEBCONFinterface.h"          // Enable the WebConfig interface

#include "Souliss.h"
#include "DHT.h"
//*************************************************************************
//Configure MQTT Server
const char* mqtt_server = "iot.eclipse.org";
char* MQTT_TOPIC_TOP = "ESP8266/MEMBER/DVES_";
char* MQTT_TEMP = "/TEMP/";

String mqtt_id;
String mqtt_path;



uint8_t MAC_array[6];
char MAC_char[18];



WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];
//*************************************************************************

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

  // Read the IP configuration from the EEPROM, if not available start
  // the node as access point
  if (!ReadIPConfiguration())
  {
    // Start the node as access point with a configuration WebServer
    SetAccessPoint();
    startWebServer();
    // We have nothing more than the WebServer for the configuration
    // to run, once configured the node will quit this.
    while (1)
    {
      yield();
      runWebServer();
    }

  }

  if (IsRuntimeGateway())
  {
    // Connect to the WiFi network and get an address from DHCP
    SetAsGateway(myvNet_dhcp);       // Set this node as gateway for SoulissApp
    SetAddressingServer();
    Serial.println("Gateway Mode");
  }
  else
  {
    // This board request an address to the gateway at runtime, no need
    // to configure any parameter here.
    SetDynamicAddressing();
    GetAddress();
    Serial.println("Peer Mode");
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
  Serial.println("Set pins OK");

  //Init MQTT Client
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.println("Set MQTT Server OK");

  WiFi.macAddress(MAC_array);
  //example: if mac address is 5C:CF:7F:0A:23:26 it retrieve ID 0A:23:26
  for (int i = 3; i < sizeof(MAC_array); ++i) {
    sprintf(MAC_char, "%s%02x", MAC_char, MAC_array[i]);
  }
  mqtt_id = MAC_char;
  Serial.print("mqtt_id: "); Serial.println(mqtt_id);

  mqtt_path = MQTT_TOPIC_TOP + mqtt_id;
  Serial.print("MQTT Path: "); Serial.println(mqtt_path);
  // Init the OTA
  OTA_Init();
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_710ms() {
      if (!client.connected()) {
        reconnect();
      }
      client.loop();
    }

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

      //snprintf(mqtt_path, 100, "test");

      String sTopic = mqtt_path + MQTT_TEMP;

      char charBuf[sTopic.length()];
      sTopic.toCharArray(charBuf, sTopic.length());

      Serial.print("Publish message: ");
      Serial.print(sTopic + ": "); Serial.println(temperature);
      Serial.println("");
      sprintf(msg, "%f", temperature);
      //client.publish(charBuf, msg);
      client.publish("TEST", "tempxxx");

      // Read humidity value from DHT sensor and convert from single-precision to half-precision
      float humidity = dht.readHumidity();
      ImportAnalog(SLOT_HUMIDITY, &humidity);

    }
    // If running as Peer
    if (!IsRuntimeGateway())
      SLOW_PeerJoin();
  }
  // Look for a new sketch to update over the air
  OTA_Process();
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  // while (!client.connected()) {
  Serial.print("Attempting MQTT connection...");
  // Attempt to connect
  if (client.connect("ESP8266Client")) {
    Serial.println("connected");
    // Once connected, publish an announcement...
    // client.publish("outTopic", "hello world");
    // ... and resubscribe
    client.subscribe("RELAY");
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    // Wait 5 seconds before retrying
    //delay(5000);
  }
}

