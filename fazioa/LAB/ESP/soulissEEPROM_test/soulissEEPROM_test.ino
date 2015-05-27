// Configure the framework
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/Gateway.h"                   // The main node is the Gateway, we have just one node
#include "conf/DynamicAddressing.h"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "Souliss.h"

 #define SSID_LENGTH 32;
 #define PASSWORD_LENGTH 64;
 #define ESCAPE_CHAR 27
 #define START_STORE_SSID_PASSWORD 0;
void setup() {
   Serial.begin(115200);
   EEPROM.begin(512);
   delay(100);
   Store_Clear();

   String ssid="myssid";
   Serial.println(ssid);
   const char *c_ssid;
   c_ssid = ssid.c_str();
   
   String password="mypwd";
   Serial.println(password);
   const char *c_password;
   c_password = password.c_str();
         
   uint16_t indiceEEPROM=START_STORE_SSID_PASSWORD;
   for(int i=0;i<strlen(c_ssid);i++){
     Serial.println(c_ssid[i]);
     Store_8bit(indiceEEPROM++, c_ssid[i]);
   }
  Serial.print("Stored: ");
  Serial.println(c_ssid);
  Store_8bit(indiceEEPROM++, ESCAPE_CHAR); //Escape

  for(int i=0;i<strlen(c_password);i++){
    Serial.println(c_password[i]);
     Store_8bit(indiceEEPROM++, c_password[i]);
  }
  Serial.print("Stored: ");
  Serial.println(c_password);
     Store_8bit(indiceEEPROM++, ESCAPE_CHAR);
     Store_Commit();

  indiceEEPROM=START_STORE_SSID_PASSWORD;
  String sResSSID = read_from_EEPROM(&indiceEEPROM);
  String sResPWD = read_from_EEPROM(&indiceEEPROM);

      Serial.print("Risultato: ");
      Serial.println(sResSSID.length());
      Serial.println(sResPWD.length());
      Serial.println(sResSSID);
      Serial.println(sResPWD);
  }
  
String  read_from_EEPROM(uint16_t *indiceEEPROM){
  int j=0;
  uint8_t byte0;
  int iMax=PASSWORD_LENGTH;
  String sResult="";
  while ((Return_8bit(*indiceEEPROM)!=ESCAPE_CHAR) && (j < iMax)){
     byte0 = Return_8bit(*indiceEEPROM);
      Serial.println((String) byte0);
      sResult+=(char) byte0;
      (*indiceEEPROM)++;
      j++;
  }
      (*indiceEEPROM)++;  
  return sResult;
  }
  
void loop() {
  // put your main code here, to run repeatedly:

}
