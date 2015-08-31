#define  MaCaco_DEBUG_INSKETCH
#define MaCaco_DEBUG  		1

#define	VNET_DEBUG_INSKETCH
#define VNET_DEBUG  		1

// Configure the framework
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/Gateway.h"                   // The main node is the Gateway, we have just one node
#include "conf/DynamicAddressing.h"

#include <ESP8266WiFi.h>
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
uint8_t GPIO_POUT = 5; //GPI14
boolean flagGateway = true;
void setup(void)
{
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(100);

 // pinMode(GPIO0_PIN, INPUT); //GPIO0

  ssid = read_from_EEPROM(&indiceEEPROM);
  password = read_from_EEPROM(&indiceEEPROM);

  if (read_from_EEPROM(&indiceEEPROM) == "G") {
    flagGateway = true;
  } else {
    flagGateway = false;
  }

  if (ssid == "") {
    //No SSID in EEPROM
    Serial.println("Wifi Access Point Mode");
    WiFi.mode(WIFI_AP);
    delay(1000);
    //IPAddress ip = (192, 168, 1, 1);
    // IPAddress gateway = (192, 168, 1, 1);
    // IPAddress NMask = (255, 255, 255, 0);
    // WiFi.softAPConfig(ip, gateway, NMask);

    WiFi.softAP("SoulissESP");

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

  while (WiFi.status() != WL_CONNECTED) {
    server.handleClient();
  }

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

  //******************************

  init_local();

  if (flagGateway) {
    Serial.println("Set Node as Gateway");
    SetAsGateway(myvNet_dhcp);       // Set this node as gateway for SoulissApp
    // This node will serve all the others in the network providing an address
    SetAddressingServer();
  } else
  {
    Serial.println("Set Node as Peer");
    // Get address dynamically
    SetDynamicAddressing();
    GetAddress();
  }

  Set_SimpleLight(MYLEDLOGIC);        // Define a simple LED light logic
  pinMode(GPIO_POUT, OUTPUT);                 // Use pin GPIO_POUT as output

}

void loop(void)
{
  // Here we start to play
  EXECUTEFAST() {
    UPDATEFAST();

    FAST_50ms() {   // We process the logic and relevant input and output every 50 milliseconds
      Logic_SimpleLight(MYLEDLOGIC);
      DigOut(GPIO_POUT, Souliss_T1n_Coil, MYLEDLOGIC);
    }
    FAST_110ms() {
      server.handleClient();
    }
    FAST_910ms(){
      readGPIO_forReset();      
      }

    if (flagGateway) {
      // Here we handle here the communication with Android
      FAST_GatewayComms();
    } else {
      // Here we handle here the communication with Android
      FAST_PeerComms();
    }
  }

  EXECUTESLOW() {
    UPDATESLOW();
    if (flagGateway) {
    }
    else {
      SLOW_PeerJoin();
    }
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
  else server.send(200, "text/html", "<form action=""formresponse""> SSID: <input type=""text"" name=""ssid""><br>Password: <input type=""text"" name=""password""><br>Mode (Gateway / Peer): <select name=""mode""><option selected>Peer <option>Gateway</select><br> <input type=""submit"" value=""Submit""></form>");
}

//RESPONSE PAGE - Use to read url and try to connect to home wifi, write on EEPROM SSID and Password
void handle_response() {
  ssid = server.arg("ssid");
  password = server.arg("password");
  String mode_GW_PEER = server.arg("mode");
  String sMode_to_write = "P";
  if (mode_GW_PEER == "Gateway")
    sMode_to_write = "G";


  indiceEEPROM = START_STORE_SSID_PASSWORD;
  EEPROM_Write_SSID_PWD(ssid, password, mode_GW_PEER, &indiceEEPROM);

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

void EEPROM_Write_SSID_PWD(String ssid, String password, String mode_GW_or_PEER, uint16_t *indiceEEPROM) {

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

  Serial.print("Stored: ");
  Serial.println(mode_GW_or_PEER);
  Store_8bit((*indiceEEPROM)++, mode_GW_or_PEER[0]);
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

long lastClickTime = 0;  // the last time the output pin was toggled
void readGPIO_forReset() {

  long ClickTime = 3000;    // the debounce time; increase if the output flickers
  if (!digitalRead(GPIO0_PIN)) {
    Serial.print("Time: ");
    Serial.println(millis() - lastClickTime);
    if ((millis() - lastClickTime) > ClickTime) {
      Store_Clear();
      Store_Commit();
      Serial.println("EEPROM Cleared.");
      ESP.reset();
    }
  }
  else lastClickTime = millis();
}

void init_local() {
  // Get the IP network parameters
  IPAddress lIP  = WiFi.localIP();
  IPAddress sMk  = WiFi.subnetMask();
  IPAddress gIP  = WiFi.gatewayIP();

  uint8_t i;
  uint8_t ipaddr[4];
  uint8_t subnet[4];
  uint8_t gateway[4];

  for (i = 0; i < 4; i++)
  {
    ipaddr[i]  = lIP[i];
    subnet[i]  = sMk[i];
    gateway[i] = gIP[i];
  }

  // The last byte of the IP address is used as vNet address
  myvNet_dhcp = ipaddr[3];

  // Starting from IP configuration define the vNet ones
  for (i = 0; i < 4; i++)
  {
    if (DEFAULT_BASEIPADDRESS) 	DEFAULT_BASEIPADDRESS[i] = ipaddr[i];
    if (DEFAULT_SUBMASK) 		DEFAULT_SUBMASK[i] = subnet[i];
    if (DEFAULT_GATEWAY) 		DEFAULT_GATEWAY[i] = gateway[i];
  }

  U16 vNet_address = (U16)ipaddr[i - 1];			// The last byte of the IP address is the vNet one
  DEFAULT_BASEIPADDRESS[i - 1] = 0;					// The BASEIPADDRESS has last byte always zero

  // Set the address
  Souliss_SetAddress(vNet_address, DYNAMICADDR_SUBNETMASK, 0);
}
