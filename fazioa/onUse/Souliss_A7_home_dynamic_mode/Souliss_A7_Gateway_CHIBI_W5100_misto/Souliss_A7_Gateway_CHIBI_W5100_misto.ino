// Configure the framework
//#define VNET_DEBUG_INSKETCH
//#define VNET_DEBUG  1
//#define  SOULISS_DEBUG_INSKETCH
//#define SOULISS_DEBUG      1

#include "bconf/Chibiduino_v1.h"			// Use a Chibiduino 2.4 GHz wireless board
#include "conf/ethW5100.h"                  // Ethernet through Wiznet W5100
#include "conf/Gateway.h"                   // The main node is the Gateway, we have just one node
#include "conf/Webhook.h"                   // Enable DHCP and DNS
//#include "conf/SmallNetwork.h"                   // The main node is the Gateway, we have just one node
#include "conf/DynamicAddressing.h"         // Use dynamic address

#include <SPI.h>
#include <EEPROM.h>
#include "Souliss.h"



// Define the network configuration according to your router settings
uint8_t ip_address[4]  = {192, 168, 1, 105};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {192, 168, 1, 1};
#define Gateway_address 0x6511              // The Gateway node has two address, one on the Ethernet side
// and the other on the wireless one
#define myvNet_address  ip_address[3]       // The last byte of the IP address (105) is also the vNet address
#define myvNet_subnet   0xFF00
#define myvNet_supern   Gateway_address

#define chibi_bridge_address    0x6511 //gateway
#define peer_chibi_address_termocamino	0x6512 //termocamino
#define peer_chibi_address_giardino	0x6513 //giardino
#define peer_chibi_address_4	0x6514 //tende - DA USARE ANCORA...
#define peer_chibi_address_soggiorno  0x0010 //soggiorno

void setup()
{
  Serial.begin(9600);
  Initialize();
  // Set network parameters
  Souliss_SetIPAddress(ip_address, subnet_mask, ip_gateway);
  SetAsGateway(myvNet_address);                                   // Set this node as gateway for SoulissApp  														// Set this node as gateway for SoulissApp
  Souliss_SetAddress(chibi_bridge_address, myvNet_subnet, myvNet_supern);	// Address on the wireless interface

  // This node as gateway will get data from the Peer
  SetAsPeerNode(peer_chibi_address_soggiorno, 1);
  SetAsPeerNode(peer_chibi_address_giardino, 2);
  SetAsPeerNode(peer_chibi_address_termocamino, 3);
  SetAsPeerNode(peer_chibi_address_4, 4);
  
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
