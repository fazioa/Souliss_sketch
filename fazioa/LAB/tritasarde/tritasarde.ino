//TRITASARDE
//@author Tonino Fazio - 2016
//@email: fazioa@gmail.com

//Potenziometri consigliati: 10Kohms

//COSTANTI
const int MINTIMESECONDS=3;
const int MAXTIMESECONDS=600;


#define MotionPotentiometerPin A0
#define PausePotentiometerPin  A1
#define MotionLedPin 3
#define PauseLedPin 4
#define releOut 5

unsigned long previousMillis = 0;
unsigned long currentMillis=0;

int MotionIntervalSeconds; //in seconds
int PauseIntervalSeconds; //in seconds
int tempVal;
boolean bMotionState=true;

void setup()
{
	pinMode(PauseLedPin, OUTPUT);
	pinMode(MotionLedPin, OUTPUT);
	pinMode(PauseLedPin, OUTPUT);
	pinMode(releOut, OUTPUT);

	pinMode(MotionPotentiometerPin, INPUT);
	pinMode(PausePotentiometerPin, INPUT);

}

void stop()
{
	digitalWrite(releOut, LOW);
	digitalWrite(PauseLedPin, HIGH);
	digitalWrite(MotionLedPin, LOW);
}

void start()
{
	digitalWrite(releOut, HIGH);
	digitalWrite(MotionLedPin, HIGH);
	digitalWrite(PauseLedPin, LOW);
}

void loop()
{
	//read new values
	tempVal= analogRead(MotionPotentiometerPin);
	MotionIntervalSeconds= map(tempVal, 0, 1023, MINTIMESECONDS, MAXTIMESECONDS);
	
	tempVal = analogRead(PauseLedPin);
	PauseIntervalSeconds= map(tempVal, 0, 1023, MINTIMESECONDS, MAXTIMESECONDS);
	
	if(bMotionState) {
		//MOTION
		start();
		} else {
		//PAUSE
		stop();
	}

	currentMillis = millis();

	if(bMotionState) {
		//MOTION - timer
		if (currentMillis - previousMillis > MotionIntervalSeconds){
			bMotionState=!bMotionState; //inverte stato moto / pausa
			previousMillis = millis();
		}
		} else {
		//PAUSE - timer
		if (currentMillis - previousMillis > PausePotentiometerPin) {
			bMotionState=!bMotionState; //inverte stato moto / pausa
			previousMillis = millis();
		}
	}
	
}


