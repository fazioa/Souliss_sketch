#include "HardwareSerial.h"
#include "SimpleModbusMaster.h"


// SimplemodbusMasterV2rev2

// state machine states
#define modbusM_IDLE 1
#define modbusM_WAITING_FOR_REPLY 2
#define modbusM_WAITING_FOR_TURNAROUND 3





#define modbusM_BUFFER_SIZE 64

unsigned char modbusM_state;
unsigned char modbusM_retry_count;
unsigned char modbusM_TxEnablePin;

// modbusM_frame[] is used to receive and transmit packages. 
// The maximum number of bytes in a modbus packet is 256 bytes
// This is limited to the serial modbusM_buffer of 64 bytes
unsigned char modbusM_frame[modbusM_BUFFER_SIZE]; 
unsigned char modbusM_buffer;
long modbusM_timeout; // timeout interval
long modbusM_polling; // turnaround delay interval
unsigned int modbusM_T1_5; // inter character time out in microseconds
unsigned int modbusM_frameDelay; // frame time out in microseconds
long modbusM_delayStart; // init variable for turnaround and timeout delay
unsigned int modbusM_total_no_of_packets; 
modbusM_Packet* packetArray; // packet starting address
modbusM_Packet* packet; // current packet
unsigned int* modbusM_register_array; // pointer to masters register array
HardwareSerial* modbusM_port;

// function definitions
void modbusM_idle();
void modbusM_constructPacket();
unsigned char modbusM_construct_F15();
unsigned char modbusM_construct_F16();
void modbusM_waiting_for_reply();
void modbusM_processReply();
void modbusM_waiting_for_turnaround();
void modbusM_process_F1_F2();
void modbusM_process_F3_F4();
void modbusM_process_F5_F6_F15_F16();
void modbusM_processError();
void modbusM_processSuccess();
unsigned int modbusM_calculateCRC(unsigned char modbusM_bufferSize);
void modbusM_sendPacket(unsigned char modbusM_bufferSize);

// modbus Master State Machine
void modbusM_update() 
{
	switch (modbusM_state)
	{
		case modbusM_IDLE:
		modbusM_idle();
		break;
		case modbusM_WAITING_FOR_REPLY:
		modbusM_waiting_for_reply();
		break;
		case modbusM_WAITING_FOR_TURNAROUND:
		modbusM_waiting_for_turnaround();
		break;
	}
}

void modbusM_idle()
{
  static unsigned int packet_index;	
	
	unsigned int failed_connections = 0;
	
	unsigned char current_connection;
	
	do
	{		
		if (packet_index == modbusM_total_no_of_packets) // wrap around to the beginning
			packet_index = 0;
				
		// proceed to the next packet
		packet = &packetArray[packet_index];
		
		// get the current connection status
		current_connection = packet->connection;
		
		if (!current_connection)
		{			
			// If all the connection attributes are false return
			// immediately to the main sketch
			if (++failed_connections == modbusM_total_no_of_packets)
				return;
		}
		packet_index++;     
    
	// if a packet has no connection get the next one		
	}while (!current_connection); 
		
	modbusM_constructPacket();
}
  
void modbusM_constructPacket()
{	 
  packet->requests++;
  modbusM_frame[0] = packet->id;
  modbusM_frame[1] = packet->function;
  modbusM_frame[2] = packet->address >> 8; // address Hi
  modbusM_frame[3] = packet->address & 0xFF; // address Lo
	
	// For functions 1 & 2 data is the number of points
	// For function 5 data is either ON (0xFF00) or OFF (0x0000)
	// For function 6 data is exactly that, one register's data
  // For functions 3, 4 & 16 data is the number of registers
  // For function 15 data is the number of coils
	
	// The data attribute needs to be intercepted by F5 & F6 because these requests
	// include their data in the data register and not in the masters array
	if (packet->function == FORCE_SINGLE_COIL || packet->function == PRESET_SINGLE_REGISTER) 
		packet->data = modbusM_register_array[packet->local_start_address]; // get the data
	
	
	modbusM_frame[4] = packet->data >> 8; // MSB
	modbusM_frame[5] = packet->data & 0xFF; // LSB
	
	//Added on SimplemodbusMasterV2r2B1 
	packet->enable_retriesCounter=1;
	
	
	
	unsigned char frameSize;    
	
  // construct the frame according to the modbus function  
  if (packet->function == PRESET_MULTIPLE_REGISTERS) 
		frameSize = modbusM_construct_F16();
	else if (packet->function == FORCE_MULTIPLE_COILS)
		frameSize = modbusM_construct_F15();
	else // else functions 1,2,3,4,5 & 6 is assumed. They all share the exact same request format.
    frameSize = 8; // the request is always 8 bytes in size for the above mentioned functions.
		
	unsigned int crc16 = modbusM_calculateCRC(frameSize - 2);	
  modbusM_frame[frameSize - 2] = crc16 >> 8; // split crc into 2 bytes
  modbusM_frame[frameSize - 1] = crc16 & 0xFF;
  modbusM_sendPacket(frameSize);

	modbusM_state = modbusM_WAITING_FOR_REPLY; // modbusM_state change
	
	// if broadcast is requested (id == 0) for function 5,6,15 and 16 then override 
  // the previous modbusM_state and force a success since the slave wont respond
	if (packet->id == 0)
			modbusM_processSuccess();
}

unsigned char modbusM_construct_F15()
{
	// function 15 coil information is packed LSB first until the first 16 bits are completed
  // It is received the same way..
  unsigned char no_of_registers = packet->data / 16;
  unsigned char no_of_bytes = no_of_registers * 2; 
	
  // if the number of points dont fit in even 2byte amounts (one register) then use another register and pad 
  if (packet->data % 16 > 0) 
  {
    no_of_registers++;
    no_of_bytes++;
  }
	
  modbusM_frame[6] = no_of_bytes;
  unsigned char bytes_processed = 0;
  unsigned char index = 7; // user data starts at index 7
	unsigned int temp;
	
  for (unsigned char i = 0; i < no_of_registers; i++)
  {
    temp = modbusM_register_array[packet->local_start_address + i]; // get the data
    modbusM_frame[index] = temp & 0xFF; 
    bytes_processed++;
     
    if (bytes_processed < no_of_bytes)
    {
      modbusM_frame[index + 1] = temp >> 8;
      bytes_processed++;
      index += 2;
    }
  }
	unsigned char frameSize = (9 + no_of_bytes); // first 7 bytes of the array + 2 bytes CRC + noOfBytes 
	return frameSize;
}

unsigned char modbusM_construct_F16()
{
	unsigned char no_of_bytes = packet->data * 2; 
    
  // first 6 bytes of the array + no_of_bytes + 2 bytes CRC 
  modbusM_frame[6] = no_of_bytes; // number of bytes
  unsigned char index = 7; // user data starts at index 7
	unsigned char no_of_registers = packet->data;
	unsigned int temp;
		
  for (unsigned char i = 0; i < no_of_registers; i++)
  {
    temp = modbusM_register_array[packet->local_start_address + i]; // get the data
    modbusM_frame[index] = temp >> 8;
    index++;
    modbusM_frame[index] = temp & 0xFF;
    index++;
  }
	unsigned char frameSize = (9 + no_of_bytes); // first 7 bytes of the array + 2 bytes CRC + noOfBytes 
	return frameSize;
}

void modbusM_waiting_for_turnaround()
{
  if ((millis() - modbusM_delayStart) > modbusM_polling)
		modbusM_state = modbusM_IDLE;
}

// get the serial data from the modbusM_buffer
void modbusM_waiting_for_reply()
{
	if ((*modbusM_port).available()) // is there something to check?
	{
		unsigned char overflowFlag = 0;
		modbusM_buffer = 0;
		while ((*modbusM_port).available())
		{
			// The maximum number of bytes is limited to the serial modbusM_buffer size 
      // of modbusM_BUFFER_SIZE. If more bytes is received than the modbusM_BUFFER_SIZE the 
      // overflow flag will be set and the serial modbusM_buffer will be read until
      // all the data is cleared from the receive modbusM_buffer, while the slave is 
      // still responding.
			if (overflowFlag) 
				(*modbusM_port).read();
			else
			{
				if (modbusM_buffer == modbusM_BUFFER_SIZE)
					overflowFlag = 1;
			
				modbusM_frame[modbusM_buffer] = (*modbusM_port).read();
				modbusM_buffer++;
			}
			// This is not 100% correct but it will suffice.
			// worst case scenario is if more than one character time expires
			// while reading from the modbusM_buffer then the modbusM_buffer is most likely empty
			// If there are more bytes after such a delay it is not supposed to
			// be received and thus will force a frame_error.
			delayMicroseconds(modbusM_T1_5); // inter character time out
		}
			
		// The minimum modbusM_buffer size from a slave can be an exception response of
    // 5 bytes. If the modbusM_buffer was partially filled set a frame_error.
		// The maximum number of bytes in a modbus packet is 256 bytes.
		// The serial modbusM_buffer limits this to 64 bytes.
	
		if ((modbusM_buffer < 5) || overflowFlag)
			modbusM_processError();       
      
		// modbus over serial line datasheet states that if an unexpected slave 
    // responded the master must do nothing and continue with the time out.
		// This seems silly cause if an incorrect slave responded you would want to
    // have a quick turnaround and poll the right one again. If an unexpected 
    // slave responded it will most likely be a frame error in any event
		else if (modbusM_frame[0] != packet->id) // check id returned
			modbusM_processError();
		else
			modbusM_processReply();
	}
	else if ((millis() - modbusM_delayStart) > modbusM_timeout) // check timeout
	{
		modbusM_processError();
		modbusM_state = modbusM_IDLE; //modbusM_state change, override modbusM_processError() modbusM_state
	}
}

void modbusM_processReply()
{
	// combine the crc Low & High bytes
  unsigned int received_crc = ((modbusM_frame[modbusM_buffer - 2] << 8) | modbusM_frame[modbusM_buffer - 1]); 
  unsigned int calculated_crc = modbusM_calculateCRC(modbusM_buffer - 2);
	
	if (calculated_crc == received_crc) // verify checksum
	{
		// To indicate an exception response a slave will 'OR' 
		// the requested function with 0x80 
		if ((modbusM_frame[1] & 0x80) == 0x80) // extract 0x80
		{
			packet->exception_errors++;
			modbusM_processError();
		}
		else
		{
			switch (modbusM_frame[1]) // check function returned
      {
        case READ_COIL_STATUS:
        case READ_INPUT_STATUS:
        modbusM_process_F1_F2();
        break;
        case READ_INPUT_REGISTERS:
        case READ_HOLDING_REGISTERS:
        modbusM_process_F3_F4();
        break;
				case FORCE_SINGLE_COIL:
				case PRESET_SINGLE_REGISTER:
        case FORCE_MULTIPLE_COILS:
        case PRESET_MULTIPLE_REGISTERS:
        modbusM_process_F5_F6_F15_F16();
        break;
        default: // illegal function returned
        modbusM_processError();
        break;
      }
		}
	} 
	else // checksum failed
	{
		modbusM_processError();
	}
}

void modbusM_process_F1_F2()
{
	// packet->data for function 1 & 2 is actually the number of boolean points
  unsigned char no_of_registers = packet->data / 16;
  unsigned char number_of_bytes = no_of_registers * 2; 
       
  // if the number of points dont fit in even 2byte amounts (one register) then use another register and pad 
  if (packet->data % 16 > 0) 
  {
    no_of_registers++;
    number_of_bytes++;
  }
             
  if (modbusM_frame[2] == number_of_bytes) // check number of bytes returned
  { 
    unsigned char bytes_processed = 0;
    unsigned char index = 3; // start at the 4th element in the frame and combine the Lo byte  
    unsigned int temp;
    for (unsigned char i = 0; i < no_of_registers; i++)
    {
      temp = modbusM_frame[index]; 
      bytes_processed++;
      if (bytes_processed < number_of_bytes)
      {
				temp = (modbusM_frame[index + 1] << 8) | temp;
        bytes_processed++;
        index += 2;
      }
      modbusM_register_array[packet->local_start_address + i] = temp;
    }
    modbusM_processSuccess(); 
  }
  else // incorrect number of bytes returned 
    modbusM_processError();
}

void modbusM_process_F3_F4()
{
	// check number of bytes returned - unsigned int == 2 bytes
  // data for function 3 & 4 is the number of registers
  if (modbusM_frame[2] == (packet->data * 2)) 
  {
    unsigned char index = 3;
    for (unsigned char i = 0; i < packet->data; i++)
    {
      // start at the 4th element in the frame and combine the Lo byte 
      modbusM_register_array[packet->local_start_address + i] = (modbusM_frame[index] << 8) | modbusM_frame[index + 1]; 
      index += 2;
    }
    modbusM_processSuccess(); 
  }
  else // incorrect number of bytes returned  
    modbusM_processError();  
}

void modbusM_process_F5_F6_F15_F16()
{
	// The repsonse of functions 5,6,15 & 16 are just an echo of the query
  unsigned int recieved_address = ((modbusM_frame[2] << 8) | modbusM_frame[3]);
  unsigned int recieved_data = ((modbusM_frame[4] << 8) | modbusM_frame[5]);
		
  if ((recieved_address == packet->address) && (recieved_data == packet->data))
    modbusM_processSuccess();
  else
    modbusM_processError();
}

void modbusM_processError()
{
	packet->retries++;
	packet->failed_requests++;
	
	// if the number of retries have reached the max number of retries 
  // allowable, stop requesting the specific packet. 
  //---> Added on SimplemodbusMasterV2r2B1 : Packet will be stoped for transmission error only if enable_retriesCounter = true
  if (packet->retries == modbusM_retry_count && packet->enable_retriesCounter)  
	{
    packet->connection = 0;
		packet->retries = 0;
	}
	modbusM_state = modbusM_WAITING_FOR_TURNAROUND;
	modbusM_delayStart = millis(); // start the turnaround delay
}

void modbusM_processSuccess()
{
	packet->successful_requests++; // transaction sent successfully
	if(packet->connection==2) packet->connection=0;  //Added on SimplemodbusMasterV2r2B1 if declared packet as ONCE and connection successful then disconnect packet
	packet->retries = 0; // if a request was successful reset the retry counter
	modbusM_state = modbusM_WAITING_FOR_TURNAROUND;
	modbusM_delayStart = millis(); // start the turnaround delay
}
  
#if defined (ESP8266) // added by Juan Luis, not tested yet
	void modbusM_configure(HardwareSerial* SerialPort,
												long baud,
												SerialConfig byteFormat,
												long _timeout, 
												long _polling, 
												unsigned char _modbusM_retry_count, 
												unsigned char _modbusM_TxEnablePin, 
												modbusM_Packet* _packets, 
												unsigned int _modbusM_total_no_of_packets,
												unsigned int* _register_array)
	

#else  
	void modbusM_configure(HardwareSerial* SerialPort,
												long baud,
												unsigned char byteFormat,
												long _timeout, 
												long _polling, 
												unsigned char _modbusM_retry_count, 
												unsigned char _modbusM_TxEnablePin, 
												modbusM_Packet* _packets, 
												unsigned int _modbusM_total_no_of_packets,
												unsigned int* _register_array)
	
#endif
	{ 

	// modbus states that a baud rate higher than 19200 must use a fixed 750 us 
  // for inter character time out and 1.75 ms for a frame delay for baud rates
  // below 19200 the timing is more critical and has to be calculated.
  // E.g. 9600 baud in a 11 bit packet is 9600/11 = 872 characters per second
  // In milliseconds this will be 872 characters per 1000ms. So for 1 character
  // 1000ms/872 characters is 1.14583ms per character and finally modbus states
  // an inter-character must be 1.5T or 1.5 times longer than a character. Thus
  // 1.5T = 1.14583ms * 1.5 = 1.71875ms. A frame delay is 3.5T.
	// Thus the formula is T1.5(us) = (1000ms * 1000(us) * 1.5 * 11bits)/baud
	// 1000ms * 1000(us) * 1.5 * 11bits = 16500000 can be calculated as a constant
	
	if (baud > 19200)
		modbusM_T1_5 = 750; 
	else 
		modbusM_T1_5 = 16500000/baud; // 1T * 1.5 = T1.5
		
	/* The modbus definition of a frame delay is a waiting period of 3.5 character times
		 between packets. This is not quite the same as the modbusM_frameDelay implemented in
		 this library but does benifit from it.
		 The modbusM_frameDelay variable is mainly used to ensure that the last character is 
		 transmitted without truncation. A value of 2 character times is chosen which
		 should suffice without holding the bus line high for too long.*/
		 
	modbusM_frameDelay = modbusM_T1_5 * 2; 
	
	// initialize
	modbusM_state = modbusM_IDLE;
  modbusM_timeout = _timeout;
  modbusM_polling = _polling;
	modbusM_retry_count = _modbusM_retry_count;
	modbusM_TxEnablePin = _modbusM_TxEnablePin;
	modbusM_total_no_of_packets = _modbusM_total_no_of_packets;
	packetArray = _packets;
	modbusM_register_array = _register_array;
	
	modbusM_port = SerialPort;


	(*modbusM_port).begin(baud, byteFormat);

	
	pinMode(modbusM_TxEnablePin, OUTPUT);
  digitalWrite(modbusM_TxEnablePin, LOW);
	
} 
  
 
  
void modbusM_construct(modbusM_Packet *_packet, 
											unsigned char id, 
											unsigned char function, 
											unsigned int address, 
											unsigned int data,
											unsigned int local_start_address)
{
	_packet->id = id;
  _packet->function = function;
  _packet->address = address;
  _packet->data = data;
	_packet->local_start_address = local_start_address;
	_packet->connection = 1;
}



unsigned int modbusM_calculateCRC(unsigned char modbusM_bufferSize) 
{
  unsigned int temp, temp2, flag;
  temp = 0xFFFF;
  for (unsigned char i = 0; i < modbusM_bufferSize; i++)
  {
    temp = temp ^ modbusM_frame[i];
    for (unsigned char j = 1; j <= 8; j++)
    {
      flag = temp & 0x0001;
      temp >>= 1;
      if (flag)
        temp ^= 0xA001;
    }
  }
  // Reverse byte order. 
  temp2 = temp >> 8;
  temp = (temp << 8) | temp2;
  temp &= 0xFFFF;
  // the returned value is already swapped
  // crcLo byte is first & crcHi byte is last
  return temp; 
}

void modbusM_sendPacket(unsigned char modbusM_bufferSize)
{
	digitalWrite(modbusM_TxEnablePin, HIGH);
		
	for (unsigned char i = 0; i < modbusM_bufferSize; i++)
		(*modbusM_port).write(modbusM_frame[i]);
		
	(*modbusM_port).flush();
	
	delayMicroseconds(modbusM_frameDelay);
	
	digitalWrite(modbusM_TxEnablePin, LOW);
		
	modbusM_delayStart = millis(); // start the timeout delay	
}