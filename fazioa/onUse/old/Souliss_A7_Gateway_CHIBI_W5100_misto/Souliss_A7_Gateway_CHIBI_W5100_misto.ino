// Configure the framework
//#define VNET_DEBUG_INSKETCH
//#define VNET_DEBUG  1
//#define  SOULISS_DEBUG_INSKETCH
//#define SOULISS_DEBUG      1

// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"

#include "bconf/Chibiduino_v1.h"			// Use a Chibiduino 2.4 GHz wireless board
#include "conf/ethW5100.h"                  // Ethernet through Wiznet W5100
#include "conf/Gateway.h"                   // The main node is the Gateway, we have just one node
#include "conf/Webhook.h"                   // Enable DHCP and DNS

#include "conf/DynamicAddressing.h"         // Use dynamic address
#include "conf/IPBroadcast.h"

#include <SPI.h>
#include <EEPROM.h>
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
#define peer_wifi_address_termostato_soggiorno  0xAB20 //termostato soggiorno
#define peer_wifi_address_termostato_piano_terra  0xAB21 //termostato piano terra

#define PIN_RESET A0
void setup()
{
  Serial.begin(9600);

  digitalWrite(PIN_RESET, HIGH);
  pinMode(PIN_RESET, INPUT); // to gnd for eeprom reset

  Serial.println(F("Delay 3s"));
  //delay 3 seconds
  delay(3000);
  check_for_reset_now();
  Serial.println(F("Start"));
  Initialize();
  // Set network parameters
  SetIPAddress(ip_address, subnet_mask, ip_gateway);
  SetAsGateway(myvNet_address);                                   // Set this node as gateway for SoulissApp
  
  SetAddress(chibi_bridge_address, myvNet_subnet, 0x0000);	// Address on the wireless interface
  SetAddress(wifi_bridge_address, myvNet_subnet, 0x0000);

  

  // This node as gateway will get data from the Peer
  SetAsPeerNode(peer_eth_address_soggiorno, 1);
  SetAsPeerNode(peer_chibi_address_giardino, 2);
  SetAsPeerNode(peer_chibi_address_termocamino, 3);
  SetAsPeerNode(peer_chibi_address_fotovoltaico, 4);
  SetAsPeerNode(peer_wifi_address_luceTettoia, 5);
  SetAsPeerNode(peer_wifi_address_termostato_soggiorno, 6);
  SetAsPeerNode(peer_wifi_address_termostato_piano_terra, 7);
    
  // This node will serve all the others in the network providing an address
  SetAddressingServer();
}

void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    // This node does just networking, bridging the Peer node to the Ethernet network
    FAST_GatewayComms();
  }

  EXECUTESLOW() {
    UPDATESLOW();
  }
}

void check_for_reset_now() {
  if (!digitalRead(PIN_RESET)) {
    Serial.println("");
    Serial.println(F("Reset"));
    Store_Init();
    Store_Clear();
    Store_Commit();
    Serial.println(F("OK"));
  }
}
