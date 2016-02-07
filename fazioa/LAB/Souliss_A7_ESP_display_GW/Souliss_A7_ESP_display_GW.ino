
// Configure the framework
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/Gateway_wPersistence.h"      // The main node is the Gateway
#include "conf/IPBroadcast.h"

// **** Define the WiFi name and password ****
#define WIFICONF_INSKETCH
#define WiFi_SSID               "asterix"
#define WiFi_Password           "ttony2013"

// Include framework code and libraries
#include <ESP8266WiFi.h>
#include <EEPROM.h>

/*** All configuration includes should be above this line ***/
#include "Souliss.h"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define	Gateway_address		0xAB01		// The Gateway node has two address, one on the Ethernet side
// and the other on the wireless one
#define chibi_bridge_address    0x6511 //gateway

uint8_t ip_address[4]  = {192, 168, 1, 105};
#define supernode_address  ip_address[3] 

#define peer_chibi_address_termocamino  0x6512 //termocamino
#define peer_chibi_address_giardino 0x6513 //giardino
#define peer_chibi_address_fotovoltaico  0x6514 //fotovoltaico
#define peer_chibi_address_soggiorno  0x0010 //soggiorno

#define pTYP                    MaCaco_P_TYP_s
#define pOUT                    MaCaco_P_OUT_s
#define pTypical(node,slot)     memory_map[pTYP+slot+(node*MaCaco_NODES*MaCaco_SLOT)]
#define pOutput(node,slot)      memory_map[pOUT+slot+(node*MaCaco_NODES*MaCaco_SLOT)]

//LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup()
{
  Serial.begin(9600);
  Serial.println("setup");

  lcd.begin(20, 4);  // initialize the lcd for 20 chars 4 lines, turn on backlight
  Serial.println("lcd.begin OK");
  lcd.backlight(); // finish with backlight on
  Serial.println("lcd.backlight OK");


  Initialize();
  Serial.println("Initialize OK");

  // Connect to the WiFi network and get an address from DHCP
  GetIPAddress();
  SetAsGateway(myvNet_dhcp);       // Set this node as gateway for SoulissApp

  Serial.println("SetAsGateway");
  Souliss_SetAddress(Gateway_address, 0xFF00, supernode_address);					// Address on the wireless interface
  Serial.println("Souliss_SetAddress");
  // This node as gateway will get data from the Peer
  SetAsPeerNode(peer_chibi_address_soggiorno, 1);
  SetAsPeerNode(peer_chibi_address_giardino, 2);
  SetAsPeerNode(peer_chibi_address_termocamino, 3);
  SetAsPeerNode(peer_chibi_address_fotovoltaico, 4);
  Serial.println("SetAsPeerNode ALL OK");

  MaCaco_InternalSubcription(memory_map);

}

void loop()
{
  // Here we start to play
  EXECUTEFAST() {
    UPDATEFAST();

    // This node does just networking, bridging the Peer node to the Ethernet network
    FAST_GatewayComms();

    // Execute the code every 2110ms
    FAST_2110ms() 	{
      //  lcd.clear();
      lcd.setCursor(0, 0); //Start at character 4 on line 0
      lcd.print("Temp.Est.");
      Serial.print("Temp.Est.p: ");
      Serial.println(  pOutput(2, 0));
      lcd.setCursor(10, 0);
      lcd.print("Temp.Sogg.");
      Serial.print("Temp.Sogg.: ");
      Serial.println(  pOutput(1, 0));

      lcd.setCursor(0, 2); //Start at character 4 on line 0
      lcd.print("Consumo");
      Serial.print("Consumo: ");
        
      Serial.println(  pOutput(1, 4));
      lcd.setCursor(0, 1);

      lcd.print(pOutput(1, 0));
      lcd.setCursor(10, 1);
      lcd.print(pOutput(1, 4) );
      lcd.setCursor(0, 3);

    }
  }

}
