//ENCODER
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 int encoder0Pos = 220;
 int encoder0PinALast = LOW;
 int n = LOW;
 float encoderValue=22.0;
 
void tickEncoder() {
  n = digitalRead(encoder0PinA);
  if ((encoder0PinALast == LOW) && (n == HIGH)) {
    if (digitalRead(encoder0PinB) == LOW) {
      //dbackLED=0;
      encoder0Pos--;
    } else {
      encoder0Pos++;
    }
    encoderValue = encoder0Pos / 10.0;
    encoder0PinALast = n;
  }
}

float getEncoderValue() {
  return encoderValue;
}


