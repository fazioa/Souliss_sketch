// Configure the framework
//#define VNET_DEBUG_INSKETCH
//#define VNET_DEBUG  1
//#define  SOULISS_DEBUG_INSKETCH
//#define SOULISS_DEBUG      1
//#define  MaCaco_DEBUG_INSKETCH
//#define MaCaco_DEBUG      1

// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"
#include "topics.h"
#include "bconf/Chibiduino_v1.h"			// Use a Chibiduino 2.4 GHz wireless board
#include "conf/ethW5100.h"                  // Ethernet through Wiznet W5100
//#include "conf/Gateway.h"                   // The main node is the Gateway, we have just one node
#include "conf/Gateway_wLastin.h"
//#include "conf/Webhook.h"                   // Enable DHCP and DNS

//#include "conf/DynamicAddressing.h"         // Use dynamic address
#include "conf/IPBroadcast.h"

#include <SPI.h>
//#include <EEPROM.h>
#include "Souliss.h"


// Define the network configuration according to your router settings
uint8_t ip_address[4]  = {192, 168, 1, 105};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {192, 168, 1, 1};

#define Gateway_address ip_address[3]              // The Gateway node has two address, one on the Ethernet side
// and the other on the wireless one
#define myvNet_address  ip_address[3]       // The last byte of the IP address (105) is also the vNet address
#define myvNet_subnet   0xFF00

#define chibi_bridge_address    0x6511 //gateway
#define wifi_bridge_address    0xAB10 //gateway

#define peer_chibi_address_termocamino	0x6512 //termocamino
#define peer_chibi_address_giardino	0x6513 //giardino
#define peer_chibi_address_fotovoltaico	0x6514 //fotovoltaico
#define peer_eth_address_soggiorno  0x0010 //soggiorno
#define peer_wifi_address_luceTettoia  0xAB11 //soggiorno
#define peer_wifi_address_PowerSocket  0xAB12 //soggiorno
#define peer_wifi_address_LYT1  0xAB13 //LYT
#define peer_wifi_address_LYT2  0xAB14 //LYT
#define peer_wifi_address_SONOFF_Divano  0xAB15 //SONOFF Smart Socket - Divano
#define peer_wifi_address_SONOFF_Cucina  0xAB16 //SONOFF Smart Socket - Cucina
#define peer_wifi_address_SONOFF_Giardino  0xAB18 //SONOFF Smart Socket - Giardino
#define peer_wifi_address_termostato_soggiorno  0xAB20 //termostato soggiorno
#define peer_wifi_address_termostato_piano_terra  0xAB21 //termostato piano terra
#define peer_wifi_address_ex_store_birra  0xAB22 //nodo controllo temperatura birra
#define peer_wifi_address_ESP_consumo_variabile  0xAB23 //nodo consumo esubero solare


#define PIN_RESET A0

void setup()
{
  // Serial.begin(9600);

  digitalWrite(PIN_RESET, HIGH);
  pinMode(PIN_RESET, INPUT); // to gnd for eeprom reset

  //Serial.println(F("Delay 3s"));
  //delay 3 seconds
  //  delay(3000);
  //  check_for_reset_now();
  //  Serial.println(F("Start"));
  Initialize();
  // Set network parameters
  SetIPAddress(ip_address, subnet_mask, ip_gateway);
  SetAsGateway(Gateway_address);                                   // Set this node as gateway for SoulissApp

  SetAddress(chibi_bridge_address, myvNet_subnet, Gateway_address);	// Address on the wireless interface
  SetAddress(wifi_bridge_address, myvNet_subnet, Gateway_address);



  // This node as gateway will get data from the Peer
  SetAsPeerNode(peer_eth_address_soggiorno, 1);
  SetAsPeerNode(peer_chibi_address_giardino, 2);
  SetAsPeerNode(peer_chibi_address_termocamino, 3);
  SetAsPeerNode(peer_chibi_address_fotovoltaico, 4);
  SetAsPeerNode(peer_wifi_address_luceTettoia, 5);
  SetAsPeerNode(peer_wifi_address_termostato_soggiorno, 6);
  SetAsPeerNode(peer_wifi_address_termostato_piano_terra, 7);
  SetAsPeerNode(peer_wifi_address_LYT1, 8);
  SetAsPeerNode(peer_wifi_address_LYT2, 9);
  SetAsPeerNode(peer_wifi_address_PowerSocket, 10);
  SetAsPeerNode(peer_wifi_address_ex_store_birra, 11);
  SetAsPeerNode(peer_wifi_address_SONOFF_Divano, 12);
  SetAsPeerNode(peer_wifi_address_SONOFF_Cucina, 13);
  SetAsPeerNode(peer_wifi_address_SONOFF_Giardino, 14);

  

  // This node will serve all the others in the network providing an address
  SetAddressingServer();
}

// Sender

//lastin configuration
float valEnergy = 0;
uint8_t valByteArray[2];
uint8_t vNode_Energy = 1;
uint8_t vNode_slotEnergy = 4;

float valSolar = 0;
uint8_t vNode_Solar = 4;
uint8_t vNode_slotSolar = 0;

float valTemp = 0;
uint8_t vNode_Temp = 3;
uint8_t vNode_slotTemp = 0;

uint16_t output16;

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_90ms() {
      if (LastIn_IsData(vNode_Energy)) {
        valEnergy = LastIn_GetAnalog(vNode_Energy, vNode_slotEnergy);
        LastIn_ClearData(1);
      } else if (LastIn_IsData(vNode_Solar)) {
        valSolar = LastIn_GetAnalog(vNode_Solar, vNode_slotSolar);
        LastIn_ClearData(1);
      } else if (LastIn_IsData(vNode_Temp)) {
        valTemp = LastIn_GetAnalog(vNode_Temp, vNode_slotTemp);
        LastIn_ClearData(1);
      }
    }

    //Scelgo di fare pubblicare il valore direttamente dal nodo invece che dal GW
    //    SHIFT_7110ms(0) {
    //      float16(&output16, &valEnergy);
    //      valByteArray[0] = C16TO8L(output16);
    //      valByteArray[1] = C16TO8H(output16);
    //      Serial.print("Float: "); Serial.print(valEnergy);
    //      Serial.print(", Publish ENERGY_TOPIC: "); Serial.print( valByteArray[0]); Serial.print(" "); Serial.println( valByteArray[1]);
    //      publishdata(ENERGY_TOPIC, valByteArray, 2);
    //    }

    FAST_7110ms() {
      float16(&output16, &valSolar);
      valByteArray[0] = C16TO8L(output16);
      valByteArray[1] = C16TO8H(output16);
      //      Serial.print("Float: "); Serial.print(valSolar);
      //      Serial.print(", Publish SOLAR_TOPIC: "); Serial.print( valByteArray[0]); Serial.print(" "); Serial.println( valByteArray[1]);
      pblshdata(SOLAR_TOPIC, valByteArray, 2);
    }

    FAST_21110ms() {
      float16(&output16, &valTemp);
      valByteArray[0] = C16TO8L(output16);
      valByteArray[1] = C16TO8H(output16);
      //      Serial.print("Float: "); Serial.print(valTemp);
      //      Serial.print(", Publish TEMPERATURE_TOPIC: "); Serial.print( valByteArray[0]); Serial.print(" "); Serial.println( valByteArray[1]);
      pblshdata(TEMPERATURE_TOPIC, valByteArray, 2);
      // publish(Cloudy);
      //publish(Alarm);
    }

    // This node does just networking, bridging the Peer node to the Ethernet network
    FAST_GatewayComms();
  }

  EXECUTESLOW() {
    UPDATESLOW();
  }
}

//void check_for_reset_now() {
//  if (!digitalRead(PIN_RESET)) {
//    Serial.println("");
//    Serial.println(F("Reset"));
//    Store_Init();
//    Store_Clear();
//    Store_Commit();
//    Serial.println(F("OK"));
//  }
//}
