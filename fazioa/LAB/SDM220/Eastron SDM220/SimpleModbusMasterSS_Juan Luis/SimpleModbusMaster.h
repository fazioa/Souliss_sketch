#ifndef SIMPLE_MODBUS_MASTER_H
#define SIMPLE_MODBUS_MASTER_H

// SimplemodbusMasterV2r2

/* 
	 SimplemodbusMaster allows you to communicate
	 to any slave using the modbus RTU protocol.
	
	 To communicate with a slave you need to create a packet that will contain 
   all the information required to communicate to the slave.
   Information counters are implemented for further diagnostic.
   These are variables already implemented in a packet. 
   You can set and clear these variables as needed.
   
   The following modbus information counters are implemented:    
   
   requests - contains the total requests to a slave
   successful_requests - contains the total successful requests
	 failed_requests - general frame errors, checksum failures and buffer failures 
   retries - contains the number of retries
   exception_errors - contains the specific modbus exception response count
	 These are normally illegal function, illegal address, illegal data value
	 or a miscellaneous error response.
	
   And finally there is a variable called "connection" that 
   at any given moment contains the current connection 
   status of the packet. If true then the connection is 
   active. If false then communication will be stopped
   on this packet until the programmer sets the connection
   variable to true explicitly. The reason for this is 
   because of the time out involved in modbus communication.
   Each faulty slave that's not communicating will slow down
   communication on the line with the time out value. E.g.
   Using a time out of 1500ms, if you have 10 slaves and 9 of them
   stops communicating the latency burden placed on communication
   will be 1500ms * 9 = 13,5 seconds!  
   Communication will automatically be stopped after the retry count expires
   on each specific packet.
	
   All the error checking, updating and communication multitasking
   takes place in the background.
  
   In general to communicate with to a slave using modbus
   RTU you will request information using the specific
   slave id, the function request, the starting address
   and lastly the data to request.
   Function 1, 2, 3, 4, 5, 6, 15 & 16 are supported. In addition to
   this broadcasting (id = 0) is supported for function 5, 6, 15 & 16.
	 
   Constants are provided for:
	 Function 1  - READ_COIL_STATUS
	 Function 2  - READ_INPUT_STATUS
   Function 3  - READ_HOLDING_REGISTERS 
	 Function 4  - READ_INPUT_REGISTERS
	 Function 5 - FORCE_SINGLE_COIL
	 Function 6 - PRESET_SINGLE_REGISTER
	 Function 15 - FORCE_MULTIPLE_COILS
   Function 16 - PRESET_MULTIPLE_REGISTERS 
	 
	 Note:  
   The Arduino serial ring buffer is 64 bytes or 32 registers.
   Most of the time you will connect the Arduino using a MAX485 or similar.
 
   In a function 3 or 4 request the master will attempt to read from a
   slave and since 5 bytes is already used for ID, FUNCTION, NO OF BYTES
   and two BYTES CRC the master can only request 58 bytes or 29 registers.
 
   In a function 16 request the master will attempt to write to a 
   slave and since 9 bytes is already used for ID, FUNCTION, ADDRESS, 
   NO OF REGISTERS, NO OF BYTES and two BYTES CRC the master can only write
   54 bytes or 27 registers.
    
   Note:
   Using a USB to Serial converter the maximum bytes you can send is 
   limited to its internal buffer which differs between manufactures. 
 
   Since it is assumed that you will mostly use the Arduino to connect without
   using a USB to Serial converter the internal buffer is set the same as the
   Arduino Serial ring buffer which is 64 bytes.
   
   
   SimplemodbusMasterV2r2B1
	   by juan Luis
	   I added the option of send a packet only one time(connection=2), when it is sent it will be stopped(connection=0)
	   And we will need to activate again from main program  (setting connection=1 or connection=2)
	   Besides I added the option of disable counter of retries. That is to say that packet wont be stopped for comunication errors.
	   This version is fully compatible with last one.
	   Usage:
	   modbusM_packet_Off(packets[nOfPacket]);  //disable packet
	   modbusM_packet_Off(packets[nOfPacket]);   //enable packet and send it sequentially (only way so far)
	   modbusM_packet_Once(packets[nOfPacket]);   // packet will be sent only one time
	   
	   modbusM_disable_retriesForever(packets[nOfPacket]); (only way so far) after certain number of transmision errors packet will be disable
	   modbusM_enable_retriesForever(packets[nOfPacket]); packet will retries for ever
	   
	   It is now compatible with AVR and Espressif MCU's
	   
	  SimplemodbusMasterV2r2B1_SS
	  by Juan Luis
	  Changed some global variables for work into Souliss
	  
	  
   
*/

#include "Arduino.h"


//MACRO FOR TURN ON OR OFF THEN PACKET_OFF
 #define modbusM_packet_Off(packet) packet.connection = 0		//Packet wont be sent
 #define modbusM_packet_On(packet) packet.connection = 1		//Packet will be sent in al loop
 #define modbusM_packet_Once(packet) packet.connection = 2      //Packet will be sent only one time then will turn connection=0
 
 //MACRO FOR ENABLE DISABLE disable_retriesCounter
 #define modbusM_enable_retriesForever(packet) packet.enable_retriesCounter=0    
 #define modbusM_disable_retriesForever(packet) packet.enable_retriesCounter=1//by default
 
 

#define COIL_OFF 0x0000 // Function 5 OFF request is 0x0000
#define COIL_ON 0xFF00 // Function 5 ON request is 0xFF00
#define READ_COIL_STATUS 1 // Reads the ON/OFF status of discrete outputs (0X references, coils) in the slave.
#define READ_INPUT_STATUS 2 // Reads the ON/OFF status of discrete inputs (1X references) in the slave.
#define READ_HOLDING_REGISTERS 3 // Reads the binary contents of holding registers (4X references) in the slave.
#define READ_INPUT_REGISTERS 4 // Reads the binary contents of input registers (3X references) in the slave. Not writable.
#define FORCE_SINGLE_COIL 5 // Forces a single coil (0X reference) to either ON (0xFF00) or OFF (0x0000).
#define PRESET_SINGLE_REGISTER 6 // Presets a value into a single holding register (4X reference).
#define FORCE_MULTIPLE_COILS 15 // Forces each coil (0X reference) in a sequence of coils to either ON or OFF.
#define PRESET_MULTIPLE_REGISTERS 16 // Presets values into a sequence of holding registers (4X references).

typedef struct
{
  // specific packet info
  unsigned char id;
  unsigned char function;
  unsigned int address;
	
	// For functions 1 & 2 data is the number of points
	// For function 5 data is either ON (oxFF00) or OFF (0x0000)
	// For function 6 data is exactly that, one register's data
  // For functions 3, 4 & 16 data is the number of registers
  // For function 15 data is the number of coils
  unsigned int data; 
	
	unsigned int local_start_address;
  
  // modbus information counters
  unsigned int requests;
  unsigned int successful_requests;
	unsigned int failed_requests;
	unsigned int exception_errors;
  unsigned int retries;
  unsigned char enable_retriesCounter;
  	
  // connection status of packet
  unsigned char connection; 
  
}modbusM_Packet;

// function definitions
void modbusM_update();

void modbusM_construct(modbusM_Packet *_packet, 
											unsigned char id, 
											unsigned char function, 
											unsigned int address, 
											unsigned int data,
											unsigned _local_start_address);

#if defined (ESP8266)				//ESP8266 added by Juan Luis, not tested yet							
void modbusM_configure(HardwareSerial* SerialPort,
											long baud, 
											SerialConfig byteFormat,
											long _timeout, 
											long _polling, 
											unsigned char _retry_count, 
											unsigned char _TxEnablePin,
											modbusM_Packet* _packets, 
											unsigned int _total_no_of_packets,
											unsigned int* _register_array);
#else											
void modbusM_configure(HardwareSerial* SerialPort,
											long baud, 
											unsigned char byteFormat,
											long _timeout, 
											long _polling, 
											unsigned char _retry_count, 
											unsigned char _TxEnablePin,
											modbusM_Packet* _packets, 
											unsigned int _total_no_of_packets,
											unsigned int* _register_array);
#endif
#endif