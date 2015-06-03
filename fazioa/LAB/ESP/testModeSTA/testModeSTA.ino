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
uint16_t START_STORE_SSID_PASSWORD = 300;
#define ESCAPE_CHAR 27
#define CHAR255 255

ESP8266WebServer server(80);

const int led = 13;
String ssid = "";
String password = "";
uint16_t indiceEEPROM = START_STORE_SSID_PASSWORD;
uint8_t GPIO0_PIN = 0; //GPIO0

void setup(void)
{
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(100);

  pinMode(GPIO0_PIN, INPUT); //GPIO0

  ssid = read_from_EEPROM(&indiceEEPROM);
  password = read_from_EEPROM(&indiceEEPROM);

  if (ssid == "") {
    //No SSID in EEPROM
    Serial.println("Wifi Access Point Mode");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("SoulissESP");
    delay(2000);
    IPAddress Ip = (192, 168, 1, 1);
    IPAddress NMask = (255, 255, 255, 0);
    WiFi.softAPConfig(Ip, Ip, NMask);

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

//**************************
 // Get the IP network parameters
	IPAddress lIP  = WiFi.localIP();
	IPAddress sMk  = WiFi.subnetMask();
	IPAddress gIP  = WiFi.gatewayIP();
	
  Serial.print("localIP: ");
  Serial.println(lIP);
  Serial.print("subnetMask: ");
  Serial.println(sMk);
  Serial.print("gatewayIP: ");
  Serial.println(gIP);

	uint8_t i;
	uint8_t ipaddr[4];
	uint8_t subnet[4];
	uint8_t gateway[4];
	
	for(i=0;i<4;i++)
	{
		ipaddr[i]  = lIP[i];
		subnet[i]  = sMk[i];
		gateway[i] = gIP[i];
	}	

	// The last byte of the IP address is used as vNet address
	myvNet_dhcp = ipaddr[3];
	
	// Starting from IP configuration define the vNet ones
	for(i=0; i<4; i++)
	{
		if(DEFAULT_BASEIPADDRESS) 	DEFAULT_BASEIPADDRESS[i]=ipaddr[i];
		if(DEFAULT_SUBMASK) 		DEFAULT_SUBMASK[i] = subnet[i];
		if(DEFAULT_GATEWAY) 		DEFAULT_GATEWAY[i] = gateway[i];
	}
	
	U16 vNet_address = (U16)ipaddr[i-1];			// The last byte of the IP address is the vNet one
	DEFAULT_BASEIPADDRESS[i-1]=0;					// The BASEIPADDRESS has last byte always zero
	
	// Set the address
	Souliss_SetAddress(vNet_address, DYNAMICADDR_SUBNETMASK, 0);	
//******************************

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
      DigOut(5, Souliss_T1n_Coil, MYLEDLOGIC);
    }
    FAST_70ms() {
      server.handleClient();
    }
    FAST_110ms() {
      readGPIO_forReset();
    }

    // Here we handle here the communication with Android
    FAST_GatewayComms();
  }
}

//HOME PAGE - Used to choise of ssid and password of home wifi
void handle_root() {
  if (ssid != "") {
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
  ssid = server.arg("ssid");
  password = server.arg("password");

  indiceEEPROM = START_STORE_SSID_PASSWORD;
  EEPROM_Write_SSID_PWD(ssid, password, &indiceEEPROM);

  indiceEEPROM = START_STORE_SSID_PASSWORD;
  ssid = read_from_EEPROM(&indiceEEPROM);
  password = read_from_EEPROM(&indiceEEPROM);

  if (ssid != "") {
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
    readGPIO_forReset();
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void EEPROM_Write_SSID_PWD(String ssid, String password, uint16_t *indiceEEPROM) {

  const char *c_ssid;
  c_ssid = ssid.c_str();
  const char *c_password;
  c_password = password.c_str();

  for (int i = 0; i < strlen(c_ssid); i++) {
    Store_8bit((*indiceEEPROM)++, c_ssid[i]);
  }

  Serial.print("Stored: ");
  Serial.println(c_ssid);
  Store_8bit((*indiceEEPROM)++, ESCAPE_CHAR); //Escape

  for (int i = 0; i < strlen(c_password); i++) {
    Store_8bit((*indiceEEPROM)++, c_password[i]);
  }
  Serial.print("Stored: ");
  Serial.println(c_password);
  Store_8bit((*indiceEEPROM)++, ESCAPE_CHAR);
  Store_Commit();
}


String  read_from_EEPROM(uint16_t *indiceEEPROM) {
  int j = 0;
  uint8_t byte0;
  int iMax = PASSWORD_LENGTH;
  String sResult = "";

  while ((Return_8bit(*indiceEEPROM) != ESCAPE_CHAR) && (Return_8bit(*indiceEEPROM) != CHAR255) && (j < iMax)) {
    byte0 = Return_8bit(*indiceEEPROM);
    sResult += (char) byte0;
    (*indiceEEPROM)++;
    j++;
  }
  (*indiceEEPROM)++;
  return sResult;
}


void readGPIO_forReset() {
  long lastClickTime = 0;  // the last time the output pin was toggled
  long ClickTime = 5000;    // the debounce time; increase if the output flickers

  lastClickTime = millis();
  while (!digitalRead(GPIO0_PIN)) {
    if ((millis() - lastClickTime) > ClickTime) {
      Store_Clear();
      Store_Commit();
      Serial.println("EEPROM Cleared. Reset!");
    }

  }


}
