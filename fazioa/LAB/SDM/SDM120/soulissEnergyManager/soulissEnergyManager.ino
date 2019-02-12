/**************************************************************************
  Souliss - Energy Manager
***************************************************************************/

#define MaCaco_DEBUG_INSKETCH
#define MaCaco_DEBUG      0
#define  VNET_DEBUG_INSKETCH
#define  VNET_DEBUG         0
#define SOULISS_DEBUG_INSKETCH
#  define SOULISS_DEBUG      0

//#include <AltSoftSerial.h>
//AltSoftSerial  altSerial;

//#define SERIALPORT_INSKETCH
//#  define LOG          altSerial

#define VNET_RESETTIME 0x000217B

#define NRF24PINS_INSKETCH
#define NRF24_RADIOEN   10       // Chip Enable Pin
#define NRF24_SPICS     9       // SPI Chip Select Pin

// Configure the framework
#include "SoulissFramework.h"
#include "bconf/StandardArduino.h"
#include "conf/nRF24L01.h"
#include "Souliss.h"
#include <SPI.h>
#include <EEPROM.h>
#include <SimpleModbusMasterSDM120.h>


#define nrfAddressEnergy       0x6503
#define nrfAddressSalotto       0x6501
#define nrfSubnet               0x6500
#define lanSalotto        0x00C9  
#define network_address_1 0x00CB      
#define network_my_subnet 0xFF00
#define network_my_supern 0x00C9

#define SLOT_VOL  0
#define SLOT_CUR  2
#define SLOT_POW  4 
#define SLOT_ENE  6 

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
#define SDM120C_BAUDRATE       9600
#define SDM120C_BYTEFORMAT     SERIAL_8N2    //Prty n
#define MTIMEOUT 1000
#define POLLING 5000    // the scan rate 
#define RETRYCOUNT 10   
#define TXENPIN  3     // Pin RS485

// This is the easiest way to create new packets
// Add as many as you want. TOTAL_NO_OF_PACKETS
// is automatically updated.
enum
{
 PACKET1,
 PACKET2,
 PACKET3,
 PACKET4,
 TOTAL_NO_OF_PACKETS // leave this last entry
};

// Create an array of Packets to be configured
Packet packets[TOTAL_NO_OF_PACKETS];

// Masters register array
packetPointer volPacket = &packets[PACKET1];
packetPointer curPacket = &packets[PACKET2];
packetPointer powPacket = &packets[PACKET3];
packetPointer enePacket = &packets[PACKET4]; 

// Union 
union datas{
 byte  b[4];
 float f;
 unsigned int Array[2]; 
}voltage, current ,power, energy;
     
void setup()
{ 
  //altSerial.begin(9600);

     //Serial.begin(115200);
//altSerial.println("Start");
     Initialize();

     Souliss_SetAddress(nrfAddressEnergy, network_my_subnet, nrfAddressSalotto);   

    // modbus_construct(packet, id, function, address, data, register array)    
    // For functions 1 & 2 data is the number of points
    // For functions 3, 4 & 16 data is the number of registers
    // For function 15 data is the number of coils
    modbus_construct_SDM120(volPacket, SDM120C_METER_NUMBER, VOL_ADR, voltage.Array);    
    modbus_construct_SDM120(curPacket, SDM120C_METER_NUMBER, CUR_ADR, current.Array);
    modbus_construct_SDM120(powPacket, SDM120C_METER_NUMBER, POW_ADR, power.Array);
    modbus_construct_SDM120(enePacket, SDM120C_METER_NUMBER, ENE_ADR, energy.Array);
    modbus_configure(&Serial, SDM120C_BAUDRATE, SDM120C_BYTEFORMAT, MTIMEOUT, POLLING, RETRYCOUNT, TXENPIN, packets, TOTAL_NO_OF_PACKETS);


  Set_Voltage(SLOT_VOL);
  Set_Current(SLOT_CUR);
  Set_Power(SLOT_POW);
  Set_Power(SLOT_ENE);




}

void loop()
{ 
  EXECUTEFAST() {                                         
                UPDATEFAST();   
                START_PeerJoin();
                FAST_10ms(){
                   ProcessCommunication();
                   
                }
                FAST_30ms() {
                      modbus_update();
                }
                
                
                FAST_50ms() {  
                } 
                          
                FAST_70ms() {  
                     
                } 
                FAST_2110ms()
                {
                  
                  
                  
                  
                }                      
              }


  EXECUTESLOW()
  { 
               UPDATESLOW();
                SLOW_PeerJoin();
               SLOW_10s(){
                   modbus_update();
                  float v;
                  v= (float)voltage.f;
                  LOG.println(v, 1);  
                  ImportAnalog(SLOT_VOL, &v);
                   
                  v= (float)current.f;
                  LOG.println(v, 2);  
                  ImportAnalog(SLOT_CUR, &v); 
                  
                  v= (float)power.f;
                  LOG.println(v, 1);
                  ImportAnalog(SLOT_POW, &v);
                  
                  v= (float)energy.f;
                  LOG.println(v, 2);  
                  ImportAnalog(SLOT_ENE, &v); 

                  Souliss_Logic_T55(memory_map, SLOT_VOL, 0.02, &data_changed);
                  Souliss_Logic_T56(memory_map, SLOT_CUR, 0.02, &data_changed);
                  Souliss_Logic_T57(memory_map, SLOT_POW, 0.02, &data_changed);
                  Souliss_Logic_T57(memory_map, SLOT_ENE, 0.02, &data_changed);
               }
    SLOW_50s()
    {       
                       
    }     
  }
  


} 

