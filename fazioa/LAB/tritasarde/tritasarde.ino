//#define DEBUG

//TRITASARDE
//@author Tonino Fazio - 2016
//@email: fazioa@gmail.com

//Potenziometri consigliati: 10Kohms

//COSTANTI
const long MINTIMESECONDS = 3;
const long MAXTIMESECONDS = 240;


#define MotionPotentiometerPin 14 //A0
#define PausePotentiometerPin  15 //A1
#define MotionLedPin 3
#define PauseLedPin 4
#define releOut 5
#define ManualPlayInputPin 6

unsigned long previousSeconds = 0;
unsigned long currentSeconds = 0;

long MotionIntervalSeconds; //in seconds
long PauseIntervalSeconds; //in seconds
int tempVal;
boolean bMotionState = true;
boolean bManualPlay = false;

void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
#endif
  pinMode(PauseLedPin, OUTPUT);
  pinMode(MotionLedPin, OUTPUT);
  pinMode(releOut, OUTPUT);
  pinMode(ManualPlayInputPin, INPUT_PULLUP);

  pinMode(MotionPotentiometerPin, INPUT);
  pinMode(PausePotentiometerPin, INPUT);
  previousSeconds = millis() / 1000;
}

void stop_motor()
{
  digitalWrite(releOut, LOW);
  digitalWrite(PauseLedPin, HIGH);
  digitalWrite(MotionLedPin, LOW);
}

void start_motor()
{
  digitalWrite(releOut, HIGH);
  digitalWrite(PauseLedPin, LOW);
  digitalWrite(MotionLedPin, HIGH);

}

void loop()
{
  //read new values
  tempVal = analogRead(MotionPotentiometerPin);
  MotionIntervalSeconds = map(tempVal, 0, 1023, MINTIMESECONDS, MAXTIMESECONDS);
#ifdef DEBUG
  Serial.print("Motion ");
  Serial.print(MotionIntervalSeconds);
  Serial.print("  -  ");
#endif

  tempVal = analogRead(PausePotentiometerPin);
  PauseIntervalSeconds = map(tempVal, 0, 1023, MINTIMESECONDS, MAXTIMESECONDS);
#ifdef DEBUG
  Serial.print("Pause ");
  Serial.print(PauseIntervalSeconds);
  Serial.print("  -  ");
  Serial.print("bMotionState ");
  Serial.print(bMotionState);

  Serial.print("  -  ");
  Serial.print("bManualPlay ");
  Serial.print(bManualPlay);
#endif

  bManualPlay = digitalRead(ManualPlayInputPin);
  if (!bManualPlay || bMotionState ) {
    //MOTION
    start_motor();
  } else {
    //PAUSE
    stop_motor();
  }

  currentSeconds = millis() / 1000;

#ifdef DEBUG
  Serial.print("  -  ");
  Serial.print("time ");
  Serial.println(currentSeconds - previousSeconds);
#endif

  if (bMotionState) {
    //MOTION - timer
    if (currentSeconds - previousSeconds > MotionIntervalSeconds) {
      bMotionState = !bMotionState; //inverte stato moto / pausa
      previousSeconds = millis() / 1000;
    }
  } else {
    //PAUSE - timer
    if (currentSeconds - previousSeconds > PauseIntervalSeconds) {
      bMotionState = !bMotionState; //inverte stato moto / pausa
      previousSeconds = millis() / 1000;
    }
  }

}


