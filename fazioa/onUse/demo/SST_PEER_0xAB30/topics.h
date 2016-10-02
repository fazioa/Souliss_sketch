#include <Arduino.h>

//PUBLISH&SUBSCRIBE TOPICS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define TOPIC1 0xF001,0x15
#define TOPIC2 0xF001,0x16
#define TOPIC3 0xF001,0x0A
#define TOPIC4 0xF001,0x17
#define TOPIC5 0xF001,0x18
#define TOPIC6 0xF001,0x0A

#define SST_HEAT_ONOFF 0xF001,0x0B
U8 HEAT_ON = 0X01;
U8 HEAT_OFF = 0X00;

//*****************************************************
//  ES:
//#define TEMPERATURE_TOPIC_NODE_DHT_RELE 0xF001,0x15
//#define HUMIDITY_TOPIC_NODE_DHT_RELE 0xF001,0x16


