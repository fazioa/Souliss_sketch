#define  BOARDTYPE_INSKETCH
#define  QC_BOARDTYPE  0x01      /** Freaklabs Chibiduino */

#define  GATEWAYTYPE_INSKETCH
#define  QC_GATEWAYTYPE  0x02 // Gateway with PERSISTANCE Mode

#define INTERFACE_INSKETCH
#define QC_INTERFACE 0x02

#define NODESIZE_INSKETCH
#define MaCaco_NODES	4 // Number of remote nodes
#define MaCaco_SLOT	24

//#define MaCaco_DEBUG_INSKETCH
//#define MaCaco_DEBUG  1
//#define VNET_DEBUG_INSKETCH
//#define VNET_DEBUG    1
//#define SOULISS_DEBUG_INSKETCH
//#define SOULISS_DEBUG 1

#define pTYP                    MaCaco_P_TYP_s
#define pOUT                    MaCaco_P_OUT_s

#define pTypical(node,slot)     memory_map[pTYP+slot+(node*MaCaco_SLOT)]
#define pOutput(node,slot)      memory_map[pOUT+slot+(node*MaCaco_SLOT)]

#include "Souliss.h"
#include "Typicals.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define network_chibi_address_2	0x6515
#define network_my_subnet	0xFF00
#define network_my_supern	0x0069

#define chibi_bridge_address      0x6511 //soggiorno
#define network_chibi_address_3	0x6513 //giardino
#define network_chibi_address_4	0x6514 //tende

// define the shared memory map
U8 memory_map[MaCaco_MEMMAP];

// flag
U8 data_changed = 0;

#define time_base_fast		10				// Time cycle in milliseconds
#define time_base_slow		800			// Time cycle in milliseconds
#define num_phases			255				// Number of phases

U8 phase_speedy = 0, phase_fast = 0, phase_slow = 0;
unsigned long tmr_fast = 0, tmr_slow = 0;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

U8  retrieved_getdatain;
U8  retrieved_funcode;
U16 retrieved_putin;
U8  retrieved_startoffset;
U8  retrieved_numberof;
int node = 0, slot = 0;



float pOutputAsFloat(U8 node, U8 slot)
{
  float m_out;
  float32((U16*)(memory_map + pOUT + slot + (node * MaCaco_SLOT)), &m_out);
  return m_out;
}

void setup()
{
  Serial.begin(9600);
  lcd.begin(20, 4);  // initialize the lcd for 20 chars 4 lines, turn on backlight
  lcd.backlight(); // finish with backlight on

  Souliss_SetAddress(network_chibi_address_2 , network_my_subnet, network_my_supern);
  Souliss_SetLocalAddress(memory_map, network_chibi_address_2);

  Souliss_SetRemoteAddress(memory_map, chibi_bridge_address, 1);
  Souliss_SetRemoteAddress(memory_map, network_chibi_address_3, 2);
  Souliss_SetRemoteAddress(memory_map, network_chibi_address_4, 3);

  MaCaco_InternalSubcription(memory_map);
}

void loop()
{

  if (abs(millis() - tmr_fast) > time_base_fast)
  {
    tmr_fast = millis();
    phase_fast = (phase_fast + 1) % num_phases;

    if (!(phase_fast % 2))
    {
      MaCaco_DataIn();
      if (MaCaco_retrieve(&memory_map[0], MaCaco_NODATACHANGED))
      {
        // get header from the last frame
        retrieved_getdatain = MaCaco_getdatain();
        retrieved_funcode = MaCaco_getfuncode();
        retrieved_putin = MaCaco_getputin();
        retrieved_startoffset = MaCaco_getstartoffset();
        retrieved_numberof = MaCaco_getnumberof();

      }
    }

    // Execute the code every 7 time_base_fast
    if (!(phase_fast % 7))
    {

      // Retreive data from the MaCaco communication channel
      Souliss_CommunicationData(memory_map, &data_changed);
    }

    // Execute the code every 31 time_base_fast
    if (!(phase_fast % 31))
    {
      // Get logic typicals once and at every refresh
      Souliss_GetTypicals(memory_map);
    }

    // Execute the code every 51 time_base_fast
    if (!(phase_fast % 51))
    {
      // Open a communication channel with remote nodes
      Souliss_CommunicationChannels(memory_map);
    }
  }
  else if (abs(millis() - tmr_slow) > time_base_slow)
  {
    tmr_slow = millis();
    phase_slow = (phase_slow + 1) % num_phases;

    // Execute the code every 1 time_base_slow
    if (!(phase_slow % 1))
    {

      node = 1;
      slot = 4;
      //  lcd.clear();

      lcd.setCursor(0, 0); //Start at character 4 on line 0
      lcd.print("Temp.Est.");
      lcd.setCursor(10, 0);
      lcd.print("Temp.Sogg.");

      lcd.setCursor(0, 2); //Start at character 4 on line 0
      lcd.print("Consumo");

      lcd.setCursor(0, 1);
      lcd.print(pOutputAsFloat(2, 0) );
      lcd.setCursor(10, 1);
      lcd.print(pOutputAsFloat(1, 0) );
      lcd.setCursor(0, 3);
      lcd.print(pOutputAsFloat(1, 4) );

      //  printout();
    }
  }
}
void printout()
{
  Serial.println("Sketch: ");

  //  Print frame
  Serial.println("Frame header");
  Serial.print("< ");
  Serial.print(retrieved_funcode, HEX);
  Serial.print(",");
  Serial.print(retrieved_putin, HEX);
  Serial.print(",");
  Serial.print(retrieved_startoffset, HEX);
  Serial.print(",");
  Serial.print(retrieved_numberof, HEX);
  Serial.println(" >");

  Serial.println("");
  Serial.println("");

  for (U8 i = 0; i < MaCaco_SLOT; i++)
  {
    Serial.print("slot=");
    Serial.print(i, HEX);
    Serial.print(" typ=");
    Serial.print(pTypical(1, i));
    Serial.print(" val=");
    Serial.println(pOutput(1, i));
  }
  Serial.println("");
  Serial.println("");

  // Print memory map
  //  for (int i = 0; i < MaCaco_MEMMAP; i++)
  //  {
  //    Serial.print(" (");
  //    Serial.print(i, DEC);
  //    Serial.print(") ");
  //    Serial.print(memory_map[i], HEX);
  //    if (!((i + 1) % 5))  Serial.println("");
  //  }
  //  Serial.println("");
  //  Serial.println("");
}
