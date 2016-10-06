// Configure the framework
//#define VNET_DEBUG_INSKETCH
//#define VNET_DEBUG  1
//#define  SOULISS_DEBUG_INSKETCH
//#define SOULISS_DEBUG      1
//#define  MaCaco_DEBUG_INSKETCH
//#define MaCaco_DEBUG      1


#include "SoulissFramework.h"

// Configure the framework
#include "bconf/StandardArduino.h"          // Use a standard Arduino
#include "conf/ethW5100.h"                  // Ethernet through Wiznet W5100
#include "conf/Gateway.h"                   // The main node is the Gateway, we have just one node
#include "conf/IPBroadcast.h"
//#include "conf/Webhook.h"                   // Enable DHCP and DNS

#include <SPI.h>
#include "Souliss.h"


// Define the network configuration according to your router settings
uint8_t ip_address[4]  = {192, 168, 20, 20};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {192, 168, 20, 1};

#define myvNet_address ip_address[3]              // The Gateway node has two address, one on the Ethernet side
#define myvNet_subnet   0xFF00
#define myvNet_supern   myvNet_address

#define peer_wifi_address_DHT	0xAB15
#define peer_eth_address_DHT_rele  0x10
#define peer_wifi_address_SST 0xAB30 //SST

#define peer_wifi_address_power_socket  0xAB12 //LYT
#define peer_wifi_address_LYT1  0xAB13 //LYT
#define peer_wifi_address_LYT2  0xAB14 //LYT
#define peer_eth_address_lavanderia 0x11


void setup()
{
  Serial.begin(9600);

  Initialize();
  // Set network parameters
  // GetIPAddress();
  SetIPAddress(ip_address, subnet_mask, ip_gateway);
  SetAsGateway(myvNet_address);                                   // Set this node as gateway for SoulissApp
  SetAddress(0xAB10, 0xFF00, 0x0000);

  // This node as gateway will get data from the Peer
  SetAsPeerNode(peer_wifi_address_DHT, 1);
  SetAsPeerNode(peer_eth_address_DHT_rele, 2);
  SetAsPeerNode(peer_wifi_address_SST, 3);

  SetAsPeerNode(peer_wifi_address_LYT1, 4);
  SetAsPeerNode(peer_wifi_address_LYT2, 5);
  SetAsPeerNode(peer_wifi_address_power_socket, 6);
  SetAsPeerNode(peer_eth_address_lavanderia, 7);
  
  Set_SimpleLight(0);
  Set_SimpleLight(1);
  Set_SimpleLight(2);
  Set_SimpleLight(3);

  pinMode(A0, OUTPUT);                  // Hardware pulldown required
  pinMode(A1, OUTPUT);                 // Power the LED
  pinMode(A2, OUTPUT);                  // Hardware pulldown required
  pinMode(A3, OUTPUT);                 // Power the LED
}


void loop()
{
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_50ms() {
      Logic_SimpleLight(0);
      Logic_SimpleLight(1);
      Logic_SimpleLight(2);
      Logic_SimpleLight(3);
      DigOut(A0, Souliss_T1n_Coil, 0);
      DigOut(A1 , Souliss_T1n_Coil, 1);
      DigOut(A2, Souliss_T1n_Coil, 2);
      DigOut(A3 , Souliss_T1n_Coil, 3);
    }

    // This node does just networking, bridging the Peer node to the Ethernet network
    FAST_GatewayComms();
  }

  EXECUTESLOW() {
    UPDATESLOW();
  }
}


