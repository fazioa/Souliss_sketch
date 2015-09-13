/**************************************************************************
    Souliss - Hello World for Expressif ESP8266 with TFT SPI ILI9341
    
    This is the basic example, create a software push-button on Android
    using SoulissApp (get it from Play Store).  
    
    Load this code on ESP8266 board using the porting of the Arduino core
    for this platform.


        
***************************************************************************/


// Configure the framework
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/IPBroadcast.h"


// **** Define the WiFi name and password ****
#define WIFICONF_INSKETCH
#define WiFi_SSID               "asterix"
#define WiFi_Password           "ttony2013"   


// Include framework code and libraries
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "Souliss.h"
#include <DHT.h>


// Define the network configuration
uint8_t ip_address[4]  = {192, 168, 1, 25};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {192, 168, 1, 1};
#define Gateway_address 18
#define  PEER4           25
#define myvNet_address  ip_address[3]       // The last byte of the IP address (18) is also the vNet address
#define myvNet_subnet   0xFF00
#define myvNet_supern   Gateway_address


//SLOT
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This identify the number of the LED logic
#define MYLEDLOGIC    0      
         
// **** Define here the right pin for your ESP module **** 
#define  OUTPUTPIN     5


//DHT22
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DHTPIN 12 
#define DHTTYPE DHT22


DHT dht(DHTPIN, DHTTYPE);
float temperature = 0;
float humidity = 0;


//ENCODER
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 int val; 
 int encoder0PinA = 3;
 int encoder0PinB = 4;
 int encoder0Pos = 220;
 int encoder0PinALast = LOW;
 int n = LOW;



//DISPLAY
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <Adafruit_ILI9341.h>



//PIN Display
#define TFT_DC 2
#define TFT_CS 15


// Use hardware SPI
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);


#define SERIAL_OUT Serial
int backLED = 16;
bool dbackLED = 0;
float setpoint=22.0;



void setup()
{   
  //SOULISS
  ///////////////////////////////////////////////////////////////////////////////////////////////////////
    Souliss_SetIPAddress(ip_address, subnet_mask, ip_gateway);
    //Initialize();


    // Connect to the WiFi network and get an address from DHCP
    //GetIPAddress();                          


    // This is the vNet address for this node, used to communicate with other
    // nodes in your Souliss network
    SetAddress(0xAB02, 0xFF00, 0xAB01);
    
    Set_SimpleLight(MYLEDLOGIC);        // Define a simple LED light logic
    pinMode(OUTPUTPIN, OUTPUT);         // Use pin as output 
    pinMode(backLED, OUTPUT);           // Background Display LED


    dht.begin();
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////  
  SERIAL_OUT.begin(115200);
  //SERIAL_OUT.begin(921600);
  SERIAL_OUT.println("Souliss T31 Test!");


  tft.begin();
  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  SERIAL_OUT.print("Display Power Mode: 0x"); SERIAL_OUT.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  SERIAL_OUT.print("MADCTL Mode: 0x"); SERIAL_OUT.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  SERIAL_OUT.print("Pixel Format: 0x"); SERIAL_OUT.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  SERIAL_OUT.print("Image Format: 0x"); SERIAL_OUT.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  SERIAL_OUT.print("Self Diagnostic: 0x"); SERIAL_OUT.println(x, HEX);
/////////////////////////////////////////////////////////////////////////////////////////////////////////


//ENCODER
/////////////////////////////////////////////////////////////////////////////////////////////////////////
   pinMode (encoder0PinA,INPUT);
   pinMode (encoder0PinB,INPUT);
}


void loop()
{ 
    // Here we start to play
    EXECUTEFAST() {                     
        UPDATEFAST();   
        
        FAST_50ms() {   // We process the logic and relevant input and output every 50 milliseconds
            Logic_SimpleLight(MYLEDLOGIC);
            DigOut(OUTPUTPIN, Souliss_T1n_Coil,MYLEDLOGIC);
        } 
              
        // Here we handle here the communication with Android
        FAST_PeerComms();
                                    
    }
       EXECUTESLOW()
  {
    UPDATESLOW();
    
      SLOW_10s(){
        temperature = dht.readTemperature();
        humidity = dht.readHumidity();
        tft.setRotation(3);
        tft.fillScreen(ILI9341_BLUE);  
        tft.setTextColor(ILI9341_BLACK);
        tft.setTextSize(2);
        //tft.print("Temp Attuale:"); tft.println(randNumber = random(10, 320));
        tft.setCursor(10, 10);
        tft.print("Temp Attuale:"); tft.println(temperature);
        tft.setCursor(10, 25);
        tft.print("Hum Attuale:"); tft.println(humidity);
        tft.setCursor(10, 60);
        tft.setTextColor(ILI9341_RED);  
        tft.setTextSize(3);
        tft.print("SETPOINT:"); tft.print(setpoint);
        dbackLED=1;
      }



      SLOW_50s(){
        
        }
      }   
   n = digitalRead(encoder0PinA);
   if ((encoder0PinALast == LOW) && (n == HIGH)) {
     if (digitalRead(encoder0PinB) == LOW) {
       //dbackLED=0;
       encoder0Pos++;
     } else {
       encoder0Pos--;
     }
     setpoint=encoder0Pos/10.0;
        tft.setRotation(3);
        tft.setTextSize(2);
        tft.setCursor(50, 100);
        tft.setTextColor(ILI9341_WHITE);  
        tft.print("SETPOINT:"); tft.print(setpoint);     
   } 
   encoder0PinALast = n; 
   digitalWrite(backLED,dbackLED);    


//if(temperature<setpoint){
//   digitalWrite(MYLEDLOGIC,1);     
//   }else{
//   digitalWrite(MYLEDLOGIC,0);      
//   }


}
