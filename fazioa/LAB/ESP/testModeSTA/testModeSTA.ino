#define MaCaco_DEBUG_INSKETCH
#define  MaCaco_DEBUG  0x01 


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
  
// This identify the number of the LED logic
#define MYLEDLOGIC          0     
//EEPROM ADDRESS TO START MEMORIZATION
uint16_t START_STORE_SSID_PASSWORD=400;

ESP8266WebServer server(80);
 
const int led = 13;
String ssid="";
String password="";

void setup(void)
{
  Serial.begin(115200);
  EEPROM.begin(512);
  
  EEPROM_Read_SSID_PWD(&ssid, &password);
  
 if(ssid==""){
   //No SSID in EEPROM
  Serial.println("Wifi Access Point Mode");
  WiFi.mode(WIFI_AP);
  WiFi.softAP("SoulissESP");
  delay(2000);
  //IPAddress Ip = (192, 168, 1, 1);
  //IPAddress NMask = (255, 255, 255, 0);
  //WiFi.softAPConfig(Ip, Ip, NMask);
  
  Serial.println("");

  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
 } else {
   Connection(ssid, password);
   }
   
  
   
  server.on("/", handle_root);
  server.on("/formresponse", handle_response);
  
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("Init");
   Initialize();
   
   // Get the IP network parameters
	IPAddress lIP  = WiFi.softAPIP();
	IPAddress sMk  = WiFi.subnetMask();
	IPAddress gIP  = WiFi.gatewayIP();
	
	uint8_t ipaddr[4];
	uint8_t subnet[4];
	uint8_t gateway[4];
	
	for(uint8_t i=0;i<4;i++)
	{
		ipaddr[i]  = lIP[i];
		subnet[i]  = sMk[i];
		gateway[i] = gIP[i];
	}	

	// The last byte of the IP address is used as vNet address
	myvNet_dhcp = ipaddr[3];
	
	// Set the values in the vNet stack
	Souliss_SetIPAddress(ipaddr, subnet, gateway);

  Serial.println("Set Node as Gateway");   
    SetAsGateway(myvNet_dhcp);       // Set this node as gateway for SoulissApp  
 
    // This node will serve all the others in the network providing an address
    SetAddressingServer();
 
    Set_SimpleLight(MYLEDLOGIC);        // Define a simple LED light logic
    pinMode(5, OUTPUT);                 // Use pin 5 as output 
}
 
void loop(void)
{
   // Here we start to play
    EXECUTEFAST() {                     
        UPDATEFAST();   
        
        FAST_50ms() {   // We process the logic and relevant input and output every 50 milliseconds
            Logic_SimpleLight(MYLEDLOGIC);
            DigOut(5, Souliss_T1n_Coil,MYLEDLOGIC);
        } 
        FAST_70ms(){
         server.handleClient(); 
          }
        // Here we handle here the communication with Android
        FAST_GatewayComms();                                        
    }
} 
  
//HOME PAGE - Used to choise of ssid and password of home wifi
void handle_root() {
  if (ssid != ""){
    Serial.println("Connected to ssid: " + ssid);
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.send(200, "text/html", "Connected to ssid: " + ssid);
    }
    else server.send(200, "text/html", "<form action=""formresponse""> SSID: <input type=""text"" name=""ssid""><br>Password: <input type=""text"" name=""password""><br>  <input type=""submit"" value=""Submit""></form>");
}
 
//RESPONSE PAGE - Use to read url and try to connect to home wifi, write on EEPROM SSID and Password
void handle_response() {
  ssid=server.arg("ssid");
  password=server.arg("password");
EEPROM_Write_SSID_PWD(ssid, password);
EEPROM_Read_SSID_PWD(&ssid, &password);

if (ssid != ""){
  server.send(200, "text/html", "Connection to ssid: " + ssid);
  Connection(ssid, password);
}
}

void Connection(String ssid, String password) {
  const char * c_ssid = ssid.c_str();
  const char * c_password = password.c_str();
  Serial.println("Connection to ssid: " + ssid + ", password: " + password);
  delay(2000);
  WiFi.begin(c_ssid, c_password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
   Serial.println("");
  Serial.println("Store on EEPROM: ssid+password");  
  
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void EEPROM_Write_SSID_PWD(String ssid, String password) {

    const char *c_ssid;
    c_ssid = ssid.c_str();
    const char *c_password;
    c_password = password.c_str();
    uint16_t indiceEEPROM=START_STORE_SSID_PASSWORD;
   for(int i=0;i<strlen(c_ssid);i++){
     Store_8bit(indiceEEPROM++, c_ssid[i]);
       Serial.print("write: ");
  Serial.println(c_ssid[i]);
   }
  Serial.print("Stored: ");
  Serial.println(c_ssid);
  Store_8bit(indiceEEPROM++, 27); //Escape

  for(int i=0;i<strlen(c_password);i++){
     Store_8bit(indiceEEPROM++, c_password[i]);
  }
  Serial.print("Stored: ");
  Serial.println(c_password);
  Store_8bit(indiceEEPROM++, 27);
  Store_Commit();
   
  }
  
void EEPROM_Read_SSID_PWD(String *ssid, String *password) {
  
  char buf_ssid[SSID_LENGTH];
  char buf_password[PASSWORD_LENGTH];
  int j=0,i=0;
  uint8_t byte0;
   int indiceEEPROM=START_STORE_SSID_PASSWORD;

 Serial.println("\nRead SSID");
int iMax=sizeof(buf_ssid);
 while ((Return_8bit(indiceEEPROM)!=27) && (j < iMax)){
      byte0 =  Return_8bit(indiceEEPROM);
      Serial.println((String) byte0);
        Serial.print("sizeof(buf_ssid) ");
   Serial.print(sizeof(buf_ssid));
         Serial.print(" - indiceEEPROM ");
         Serial.print(indiceEEPROM);
                  Serial.print(" - indice j ");
         Serial.print(j);
         Serial.print(" - byte0 ");
         Serial.print(byte0);
         Serial.print(" -  ");
                           
      buf_ssid[j++]=byte0;
      indiceEEPROM++;
      }
      indiceEEPROM++;
      j=0;
      
  Serial.println("\nRead PASSWORD");
  iMax=sizeof(buf_password);
  while ((Return_8bit(indiceEEPROM)!=27) && (j < iMax)){
      byte0 =  Return_8bit(indiceEEPROM);
      Serial.println((String) byte0);
      buf_password[j++]=byte0;
      indiceEEPROM++;
      }
      
    Serial.println("Readed from EEPROM");
      String sSSID=buf_ssid;
      Serial.print("SSID: ");  
      Serial.println(sSSID);  
      String sPassword=buf_password;
      Serial.print("Password: ");  
      Serial.println(sPassword);  
      ssid= &sSSID;
      password=&sPassword;
    
  } 
