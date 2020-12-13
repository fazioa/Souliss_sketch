// MQ-7 wiring 
#define analogMQ7 A0      // Signal  
#define ledPin LED_BUILTIN         // Device internal LED       
int MQ7sensorValue = 0;   // value read from the sensor 

void setup() {
 Serial.begin(9600);
  
  analogWrite(analogMQ7, HIGH); // HIGH = 255 
  delay(20000); 
  analogWrite(analogMQ7, 71.4); // 255x1400/5000 
   delay(20000); 
}

void loop() {
   analogWrite(analogMQ7, HIGH);  
   delay(50); // Getting an analog read apparently takes 100uSec 
   MQ7sensorValue = analogRead(analogMQ7);  
   Serial.print("MQ-7 PPM: ");                        
   Serial.println(MQ7sensorValue); 


// D) interpretation 
   // Detecting range: 20ppm-2000ppm carbon monoxide 
   // air quality-cases: < 200 perfect, 200 - 800 normal, > 800 - 1800 high, > 1800 abnormal 
   if (MQ7sensorValue <= 200)  
   { 
       Serial.println("Air-Quality: CO perfect"); 
   } 
   else if ((MQ7sensorValue > 200) || (MQ7sensorValue <= 800)) // || = or 
   { 
       Serial.println("Air-Quality: CO normal"); 
   } 
   else if ((MQ7sensorValue > 800) || (MQ7sensorValue <= 1800)) 
   { 
       Serial.println("Air-Quality: CO high"); 
   } 
   else if (MQ7sensorValue > 1800)  
   { 
       digitalWrite(ledPin, HIGH); // optical information in case of emergency 
       Serial.println("Air-Quality: ALARM CO very high"); 
       delay(3000); 
       digitalWrite(ledPin, LOW); 
   } 
   else 
   { 
       Serial.println("MQ-7 - cant read any value - check the sensor!"); 
   }  
}
