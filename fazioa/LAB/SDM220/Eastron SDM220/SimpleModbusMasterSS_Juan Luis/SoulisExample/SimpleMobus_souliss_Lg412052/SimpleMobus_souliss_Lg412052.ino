#include "SoulissFramework.h"
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define HOSTNAME "souliss-SDM220-consumi"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "SimpleModbusMaster.h"
#define baud 2400
#define modbusM_timeout 1000
#define modbusM_polling 200 // the scan rate
#define modbusM_retry_count 5
// used to toggle the receive/transmit pin on the driver
#define TxEnablePin 2
// The total amount of available memory on the master to store data
#define TOTAL_NO_OF_REGISTERS 4

// This is the easiest way to create new packets
// Add as many as you want. TOTAL_NO_OF_PACKETS
// is automatically updated.
enum
{
  PACKET1,
  TOTAL_NO_OF_PACKETS // leave this last entry
};

// Create an array of Packets to be configured
modbusM_Packet packets[TOTAL_NO_OF_PACKETS];

// Masters register array
unsigned int regs[TOTAL_NO_OF_REGISTERS];

///////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <SPI.h>

#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/IPBroadcast.h"

#include "credenziali.h"
#include "Souliss.h"
#include <UniversalTelegramBot.h>

#define peer_address  0xAB19
#define myvNet_subnet 0xFF00
#define myvNet_supern 0xAB10

// Analog Measurements/Sensors (T5n)
#define SLOT_T55  0 // Voltage(0, 400) V                - 2 Slot


void setup()
{
  Serial.begin(9600);
  Initialize();
  GetIPAddress();
  SetAddress(peer_address, myvNet_subnet, myvNet_supern);

  // Init the OTA
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.begin();
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  modbusM_construct(&packets[PACKET1], 5, READ_HOLDING_REGISTERS, 4096, 2, 0);//ID, Function  , Address,   data_lenght, local variable start adress
  // modbusM_construct(&packets[PACKET2], 1, PRESET_MULTIPLE_REGISTERS, 1, 1, 1);
  // modbusM_packet_Once(packets[PACKET2]);
  modbusM_enable_retriesForever(packets[PACKET1]);


  // Initialize the Modbus Finite State Machine
  modbusM_configure(&Serial, baud, SERIAL_8E1, modbusM_timeout, modbusM_polling, modbusM_retry_count, TxEnablePin, packets, TOTAL_NO_OF_PACKETS, regs);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  //define typicals
  Set_T55(SLOT_T55);
  NotificaTelegram();
}

void loop()
{
  EXECUTEFAST()
  {
    UPDATEFAST();
    FAST_510ms()
    {
      //Logic_T55(SLOT_T55);
      Souliss_Logic_T55(memory_map, SLOT_T55, 0.005, &data_changed);
      modbusM_update();
    }

    FAST_910ms()
    {
      long milivolts = (long)regs[0] * 0xFFFF + regs[1];
      float voltage = (float)milivolts / 1000;
      ImportAnalog(SLOT_T55, &voltage);
      Serial.print("Voltage: ");
      Serial.println(voltage);
    }
    FAST_PeerComms();
  }
}

//telegram
void sendToTelegram(String choose, String text ) {
  WiFiClientSecure botclient;
  UniversalTelegramBot bot(BOTTOKEN, botclient);
  if (bot.sendMessage(choose, text, "")) {
    Serial.println("TELEGRAM Successfully sent");
  }
  botclient.stop();
}

void NotificaTelegram() {
  Serial.println("Invio messaggio su Telegram");
  sendToTelegram(CHAT_ID, "Nodo \"" + (String) HOSTNAME + "\" avviato" + " - IP: " + WiFi.localIP().toString());
}
//end telegram
