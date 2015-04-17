#define MaCaco_NODESIZE_INSKETCH
#define MaCaco_NODES	2 // Number of remote nodes
#define MaCaco_SLOT	24

//#define MaCaco_DEBUG_INSKETCH
//#define MaCaco_DEBUG  1
#define VNET_DEBUG_INSKETCH
#define VNET_DEBUG    1

// Configure the framework
#include "bconf/Chibiduino_v1.h"			// Use a Chibiduino 2.4 GHz wireless board
#include "conf/Gateway_wPersistence.h"			// The main node is the Gateway

// Include framework code and libraries
//#include <SPI.h>
#include "Souliss.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define	Gateway_address		0x6515		// The Gateway node has two address, one on the Ethernet side
// and the other on the wireless one
#define	Peer_address_n1		0x6511 //soggiorno
#define	Peer_address_n2		0x6513 //giardino
#define	Peer_address_n3		0x6514 //tende

#define	myvNet_subnet		0xFF00
#define	myvNet_supern		0x0069

#define pTYP                    MaCaco_P_TYP_s
#define pOUT                    MaCaco_P_OUT_s

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

float pOutput_As_Float(U8 node, U8 slot)
{
  float m_out;
  float32((U16*)(memory_map + pOUT + slot + (node * MaCaco_SLOT)), &m_out);
  return m_out;
}

int node = 0, slot = 0;
void setup()
{
 Serial.begin(9600);
 // Serial.println("setup");

  lcd.begin(20, 4);  // initialize the lcd for 20 chars 4 lines, turn on backlight
  lcd.backlight(); // finish with backlight on


  Initialize();
  // Setc network parameters
  SetAsGateway(Gateway_address);														// Set this node as gateway for SoulissApp
  Souliss_SetAddress(Gateway_address, myvNet_subnet, myvNet_supern);					// Address on the wireless interface

  // This node as gateway will get data from the Peer
  SetAsPeerNode(Peer_address_n1, 1);
  SetAsPeerNode(Peer_address_n2, 2);
  SetAsPeerNode(Peer_address_n3, 3);

  MaCaco_InternalSubcription(memory_map);

}

void loop()
{
  // Here we start to play
  EXECUTEFAST() {
    UPDATEFAST();

    // Process every 510ms the logic that control the curtain
    FAST_510ms() {

    }

    // This node does just networking, bridging the Peer node to the Ethernet network
    FAST_GatewayComms();

    // Execute the code every 2110ms
    FAST_2110ms() 	{
      //  lcd.clear();
      lcd.setCursor(0, 0); //Start at character 4 on line 0
      lcd.print("Temp.Est.");
      lcd.setCursor(10, 0);
      lcd.print("Temp.Sogg.");

      lcd.setCursor(0, 2); //Start at character 4 on line 0
      lcd.print("Consumo");

      lcd.setCursor(0, 1);

      lcd.print(pOutput_As_Float(node, slot));
      lcd.setCursor(10, 1);
      lcd.print(pOutput_As_Float(1, 0) );
      lcd.setCursor(0, 3);
      lcd.print(pOutput_As_Float(1, 4) );
    }
  }
}
