/*
* Ultrasound Sensor Test
*/

int pingPin = 6;

void setup()
{
  Serial.begin(9600);
  pinMode(pingPin,INPUT);
  
}

long duration, inches, cm;
int iState;
void loop(){
   
 //delayMicroseconds(1);
 duration = pulseIn(pingPin, HIGH);
 iState = digitalRead(pingPin);

 // convert the time into a distance
 inches = microsecondsToInches(duration);
 cm = microsecondsToCentimeters(duration);

 Serial.print(iState);
 Serial.print(" state, ");
 Serial.print(duration);
 Serial.print(" microseconds, ");
 Serial.print(inches);
 Serial.print(" in, ");
 Serial.print(cm);
 Serial.print(" cm");
 Serial.println();

 delay(2000);
}
   
long microsecondsToInches(long microseconds)
{
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
 return microseconds / 29 / 2;
}
