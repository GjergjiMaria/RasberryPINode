int steeringPower = 12;
int steeringBreak = 9;
int steeringAnalog = 3;

int goPin = 13;
int goBreakPin = 8;
int goAnalog = 11;

int theSpeed = 100;
int incomingByte =0;

// ###GEIGER COUNTER PART 
#include <SPI.h>
#define LOG_PERIOD 15000 
#define MAX_PERIOD 60000    //Maximum logging period
unsigned long counts;             //variable for GM Tube events
unsigned long cpm;                 //variable for CPM
unsigned int multiplier;             //variable for calculation CPM in this sketch
unsigned long previousMillis;      //variable for time measurement

void tube_impulse(){     //procedure for capturing events from Geiger Kit

  counts++;

}
 
// ### GEIGER COUNTER END


void runRC(int input){
  switch(input){
    case 1:
      goForward();
      break;
    case 2:   
      goBackward();
      break;
    case 3: 
      goBreak();
      break;
    case 4:
      goStraight();
      break;
    case 5:
     goRight();
     break;
    case 6:
     goLeft();
     break;
    case 7:
     theSpeed = 125;
     break;
    case 8:
     theSpeed = 65;
     break;
    case 9:
     theSpeed = 255;
     break;
    case 0:
     theSpeed = 200;
     break;
  }

} 

void goForward(){
  //Motor A forward @ theSpeed
  ////Serial.println("Going Forward");
  digitalWrite(goPin, LOW); //Establishes forward direction of Channel A
  digitalWrite(goBreakPin, LOW);   //Disengage the Brake for Channel A
  analogWrite(goAnalog, theSpeed);   //Spins the motor on Channel A at full speed
}
void goBackward(){
  //Motor A forward @ theSpeed
  ////Serial.println("Going Backward");
  digitalWrite(goPin, HIGH); //Establishes forward direction of Channel A
  digitalWrite(goBreakPin, LOW);   //Disengage the Brake for Channel A
  analogWrite(goAnalog, theSpeed);   //Spins the motor on Channel A at full speed  
}
void goBreak(){
  // Breaking
  //Serial.println("Going To Break");
  digitalWrite(goPin, LOW); //Establishes forward direction of Channel A
  digitalWrite(goBreakPin, HIGH);   //Disengage the Brake for Channel A
  analogWrite(goAnalog, 0);   //Spins the motor on Channel A at full speed
}
void goStraight(){
  //Serial.println("Going Straight");
  digitalWrite(steeringPower, HIGH);  //Establishes backward direction of Channel B
  digitalWrite(steeringBreak, HIGH);   //Disengage the Brake for Channel B
  analogWrite(steeringPower, 0);    //Spins the motor on Channel B at half speed
}

void goLeft(){
  // Left Turn
  //Serial.println("Going Left");
  digitalWrite(steeringPower, HIGH);  //Establishes backward direction of Channel B
  digitalWrite(steeringBreak, LOW);   //Disengage the Brake for Channel B
  analogWrite(steeringAnalog, theSpeed);    //Spins the motor on Channel B at half speed
  
}
void goRight(){
  // Left Right
  //Serial.println("Going Right");
  digitalWrite(steeringPower, LOW);  //Establishes backward direction of Channel B
  digitalWrite(steeringBreak, LOW);   //Disengage the Brake for Channel B
  analogWrite(steeringAnalog, theSpeed);    //Spins the motor on Channel B at half speed  
  
}

void setup() {
  
  Serial.begin(9600);
  Serial.println("Connected");
  //Setup Channel A
  
  pinMode(goPin, OUTPUT); //Initiates Motor Channel A pin
  pinMode(goBreakPin, OUTPUT); //Initiates Motor Channel A pin
  pinMode(steeringBreak, OUTPUT); //Initiates Motor Channel A pin
  pinMode(steeringPower, OUTPUT); //Initiates Motor Channel A pin
  pinMode(4, OUTPUT); // Initiate LED
  /*
  pinMode(steeringAnalog, OUTPUT);
  pinMode(goAnalog, OUTPUT);
  */
// ###Geiger counter code
  counts = 0;

  cpm = 0;

  multiplier = MAX_PERIOD / LOG_PERIOD;
  Serial.begin(9600);   // start serial monitor

 // uncommennt if you have time-out problem to connect with Radiation Logger
 //  delay(2000);
 //  Serial.write('0'); // sending zero to avoid connection time out with radiation logger
 //  delay(2000);
 //  Serial.write('0');// sending zero to avoid connection time out with radiation logger 
  pinMode(2, INPUT); // set pin INT0 input for capturing GM Tube events

  digitalWrite(2, HIGH);// turn on internal pullup resistors, solder C-INT on the PCB
  attachInterrupt(0, tube_impulse, FALLING);  //define external interrupts
//### GEIGER CODE END 
	digitalWrite(4,HIGH);
	delay(1000);
        digitalWrite(4,LOW);
}

void loop() {

        // send data only when you receive data:
        digitalWrite(4,LOW);
        if (Serial.available() > 0) {
                // read the incoming byte:
                digitalWrite(4,HIGH);
                delay(1000);
                incomingByte = Serial.read();
                runRC(incomingByte-48);
        }
// ### GEIGER CODE START
        unsigned long currentMillis = millis();

        if(currentMillis - previousMillis > LOG_PERIOD){
        	previousMillis = currentMillis;
		cpm = counts * multiplier;
		//Serial.print("The Geiger Counter Reads:"); 
		Serial.println(cpm); // send cpm data to Radiation Logger
		//Serial.write(''); // send null character to separate next data
   		counts = 0;
	}
// ### GEIGER CODE ENDS 
}
