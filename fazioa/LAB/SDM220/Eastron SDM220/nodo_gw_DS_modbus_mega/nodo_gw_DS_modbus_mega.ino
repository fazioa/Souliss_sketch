
/**************************************************************************
    Souliss - Gateway 
    
      - Arduino Uno with Ethernet Shield (W5100)
    
      - Slot 0 - 3 Relay
      - Slot 4,6 18b20
      - Slot 8,9 Feedback
      - Slot 10,12 Modbus
      
***************************************************************************/

// Configure the framework
#include "bconf/StandardArduino.h"          // Use a standard Arduino
#include "conf/ethW5100.h"                  // Ethernet through Wiznet W5100
#include "conf/Gateway.h"                   // The main node is the Gateway, we have just one node
#include <SimpleModbusMasterSDM120.h>


// Include framework code and libraries
#include <SPI.h>
#include "Souliss.h"
#include <OneWire.h>
#include <DallasTemperature.h>


//LOGICS & DEF
#define EL1pw            0
#define EL1on            1
#define EL2pw            2
#define EL2on            3
#define DALLAS1          4
#define DALLAS2          6
#define EL1sw            8
#define EL2sw            9
#define SLOT_POW  10 // Power (0, 6500) W                - 2 Slot
#define SLOT_ENE  12 // Power (0, 6500) W                - 2 Slot

//-------------------------MODBUS-DEF-----------------------------------
#define VOL_ADR 0X0000    // Tensione.
#define CUR_ADR 0X0006    // Corrente.
#define POW_ADR 0X000C    // Potenza attiva. 
#define VAM_ADR 0X0012    // Potenza apparente.
#define PRE_ADR 0X0018    // Potenza reattiva.
#define PFA_ADR 0X001E    // Fattore di potenza.
#define FRE_ADR 0X0046    // Frequenza.
#define ENE_ADR 0X0048    // Energia usata
#define POE_ADR 0X004A    // Energia ceduta.
#define NEE_ADR 0X0156    // Energia attiva totale.

#define SDM120C_METER_NUMBER   1                
#define SDM120C_BAUDRATE       2400
#define SDM120C_BYTEFORMAT     SERIAL_8N2    //Prty n
#define MTIMEOUT 1000
#define POLLING 5000    // the scan rate 
#define RETRYCOUNT 10   
#define TXENPIN  2     // Pin RS485

// This is the easiest way to create new packets
// Add as many as you want. TOTAL_NO_OF_PACKETS
// is automatically updated.
enum
{
 PACKET1,
 PACKET2,
// PACKET3,
// PACKET4,
 TOTAL_NO_OF_PACKETS // leave this last entry
};

// Create an array of Packets to be configured
Packet packets[TOTAL_NO_OF_PACKETS];

// Masters register array
//packetPointer volPacket = &packets[PACKET1];
//packetPointer curPacket = &packets[PACKET2];
packetPointer powPacket = &packets[PACKET1];
packetPointer enePacket = &packets[PACKET2]; 
 
// Union 
union datas{
 byte  b[2];
 float f;
 unsigned int Array[2]; 
}power,energy; //voltage, current ,power, energy;

//-------------------------MODBUS-END-----------------------------------


// Define the network configuration
uint8_t ip_address[4]  = {10, 0, 0, 200};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {10, 0, 0, 1};
#define Gateway_address   200
#define Peer_address_n1   201
#define Peer_address_n2   202
#define myvNet_address  ip_address[3]       // The last byte of the IP address (201) is also the vNet address
#define myvNet_subnet   0xFF00
#define myvNet_supern   Gateway_address


//PINS
#define EV1SW         4 
#define EV2SW         5 
#define DALLASPIN     3 
#define	OUT1pw        32
#define	OUT1on        34
#define	OUT2pw        36
#define	OUT2on        38
//#define LED          47

OneWire ourWire(DALLASPIN); 
DallasTemperature sensors(&ourWire); 


void setup()
{   
    digitalWrite(OUT1pw,HIGH);
    digitalWrite(OUT1on,HIGH);
    digitalWrite(OUT2pw,HIGH);
    digitalWrite(OUT2on,HIGH);
    pinMode(OUT1pw, OUTPUT);       // Use pin as output 
    pinMode(OUT1on, OUTPUT);       // Use pin as output 
    pinMode(OUT2pw, OUTPUT);       // Use pin as output 
    pinMode(OUT2on, OUTPUT);       // Use pin as output 

    //Serial.begin(9600);
    Initialize();
    sensors.begin();

    // Get the IP address from DHCP
    // GetIPAddress();
    
    Souliss_SetIPAddress(ip_address, subnet_mask, ip_gateway); 
    SetAsGateway(myvNet_address);                                   // Set this node as gateway for SoulissApp   
    // This node as gateway will get data from the Peer 
    SetAsPeerNode(Peer_address_n1, 1);  
    SetAsPeerNode(Peer_address_n2, 2); 
   
    modbus_construct_SDM120(powPacket, SDM120C_METER_NUMBER, POW_ADR, power.Array);
    modbus_construct_SDM120(enePacket, SDM120C_METER_NUMBER, ENE_ADR, energy.Array);
    modbus_configure(&Serial1, SDM120C_BAUDRATE, SDM120C_BYTEFORMAT, MTIMEOUT, POLLING, RETRYCOUNT, TXENPIN, packets, TOTAL_NO_OF_PACKETS);

    
    //Set typicals
    Set_Temperature(DALLAS1);
    Set_Temperature(DALLAS2);
    Set_SimpleLight(EL1pw);    
    Set_SimpleLight(EL1on); 
    Set_SimpleLight(EL2pw);    
    Set_SimpleLight(EL2on); 
    Set_T13(EL1sw);
    Set_T13(EL2sw);
    Set_T57(SLOT_POW);
    Set_T57(SLOT_ENE);

}

void loop()
{ 
    // Here we start to play
    EXECUTEFAST() {                     
        UPDATEFAST();   
        
        FAST_70ms() {   // We process the logic and relevant input and output every 50 milliseconds
            Logic_SimpleLight(EL1pw);
            LowDigOut(OUT1pw, Souliss_T1n_Coil,EL1pw);
            Logic_SimpleLight(EL1on);
            LowDigOut(OUT1on, Souliss_T1n_Coil,EL1on);
            Logic_SimpleLight(EL2pw);
            LowDigOut(OUT2pw, Souliss_T1n_Coil,EL2pw);         
            Logic_SimpleLight(EL2on);
            LowDigOut(OUT2on, Souliss_T1n_Coil,EL2on); 
            DigIn2State(EV1SW, Souliss_T1n_OnCmd, Souliss_T1n_OffCmd,EL1sw);
            Logic_T13(EL1sw);
            DigIn2State(EV2SW, Souliss_T1n_OnCmd, Souliss_T1n_OffCmd,EL2sw);
            Logic_T13(EL2sw);
         } 

        FAST_510ms() {
        }
          
        FAST_910ms() {
          sensors.requestTemperatures(); //Prepara el sensor para la lectura
          float dallas1 = sensors.getTempCByIndex(0);
          float dallas2 = sensors.getTempCByIndex(1);
          //Serial.println(sensors.getTempCByIndex(0));
          //Serial.println(sensors.getTempCByIndex(1));  
          Souliss_ImportAnalog(memory_map, DALLAS1, &dallas1);
          Souliss_ImportAnalog(memory_map, DALLAS2, &dallas2);
         }
  
        FAST_2110ms()
        {
          Souliss_Logic_T57(memory_map, SLOT_POW, 0.005, &data_changed);
          Souliss_Logic_T57(memory_map, SLOT_ENE, 0.005, &data_changed);
          modbus_update();
          float v;
          v= (float)power.f;
         // Serial.println(v, 1);
          ImportAnalog(SLOT_POW, &v);
          v= (float)energy.f;
         // Serial.println(v, 2);  
          ImportAnalog(SLOT_ENE, &v); 
          
          Logic_Temperature(DALLAS1);
          Logic_Temperature(DALLAS2);
        }    
        
        FAST_GatewayComms();                                        
    }
} 

