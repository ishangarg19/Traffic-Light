/**
 * Name: Ishan Garg, Bernie Chen, Jeffery Hu
 * Date: June 20, 2022
 * Teacher: Mr. Wong (TEJ3MP-1A)
 * Purpose: This program runs a T-intersection traffic light system
 *       while constantly checking for the light level for a streetlight
 *      independently of the traffic light system. A servo motor is also
 *      used to create a gate, controlled by an IR emitter and receiver
 *      that checks whether to open the gate depending on an obstruction 
 *      of the signal. A pedestrian light system constantly runs as well; 
 *      the buttons will alter the traffic light timings to the benefit 
 *      of the pedestrian (pedestrian light is between the side traffic
 *      lights, so the button checks the light sequence based on the status
 *      of the side traffic lights).
 *      
 * NOTE: side traffic lights refers to the left/right lights. The middle light
 *       is the one on the street perpendicular to the side lights (in-between
 *       the left and right lights).
 */

#include <Servo.h>

/**
 * Variables to declare servo motor pin and object
 */
Servo gate;
int servoPin = 3;

/**
 * Variables for the IR receiver that manipulates the gate.
 */
int irRecv = A0;
bool next = false;

/**
 * Variable to declare the pin of the button.
 */
int button = 7;

/**
 * Variables to declare the pins of the pedestrian lights.
 */
int pRed = 5;
int pWhite = 6;

/**
 * Variables to declare the pins of the side (left/right) traffic lights.
 */
int sRed = 8;
int sYellow = 9;
int sGreen = 10;

/**
 * Variables to declare the pins of the middle traffic lights.
 */
int mRed = 11;
int mYellow = 12;
int mGreen = 13;

/**
 * Variables to declare the pins of the photoresistor and streetlight.
 */
int pr = A3;
int street = 2;

/**
 * Variables to declare the time durations of each light.
 */
int yDelay = 3000;
int rDelay = 1000; //Total duration of the red light is yDelay + gDelay + rDelay - rDelay represents how long both traffic lights are red
int gDelay = 3000; 

/**
 * Variables to ensure only one button input is registered at a time
 * and to change the green timing in the next cycle if pressed while green.
 */
bool buttonControl = false;
bool changeGreen = false;

/**
 * Variables used to temporarily change the timings of the traffic
 * lights according to the buttons.
 */
int lessenMYellow;
int lessenSYellow;
int extendGreen;
int lessenGreen;
int lessenRed;


/**
 * Variables to store the previous time to allow tracking of time passed.
 */
unsigned long prevMillis = 0;
unsigned long prevMillisGate = 0; //Gate operates independently of the traffic light timer

/**
 * Variable to store which phase "p" of the traffic light system is currently running
 * so that the program uses the correct delay and has the correct lights turned on.
 */
int p = 1;

/**
 * Method to initialize all components.
 */
void setup()
{
  gate.attach(servoPin);
  
  pinMode(irRecv, INPUT);
  
  pinMode(button, INPUT);
  
  pinMode(pRed, OUTPUT);
  pinMode(pWhite, OUTPUT);
  
  pinMode(sRed, OUTPUT);
  pinMode(sYellow, OUTPUT);
  pinMode(sGreen, OUTPUT);
  
  pinMode(mRed, OUTPUT);
  pinMode(mYellow, OUTPUT);
  pinMode(mGreen, OUTPUT);
  
  pinMode(pr, INPUT);
  pinMode(street, OUTPUT);

  gate.write(0); //Initially horizontal
  writeStates(LOW, LOW, HIGH, LOW, LOW, HIGH); //Sets initial light sequence
  digitalWrite(pRed, HIGH);
  digitalWrite(pWhite, LOW);
}

/**
 * Method to run the operations constantly.
 */
void loop()
{
  trafficLight();
  streetLight();
  runGate();
}

/**
 * Method to run the traffic lights by checking if its
 * on the correct part of the phase "p" and if the time required
 * for the phase to begin has passed.
 */
void trafficLight() {
  changeTrafficLights();
  if (p == 1 && wait(rDelay + lessenRed)) {
    writeStates(LOW, LOW, HIGH, LOW, HIGH, LOW); //Sides red, middle green
  }
  if (p == 2 && wait(gDelay + lessenGreen)) {
    writeStates(LOW, LOW, HIGH, HIGH, LOW, LOW); //Sides red, middle yellow
  }
  if (p == 3 && wait(yDelay + lessenMYellow)) {
    writeStates(LOW, LOW, HIGH, LOW, LOW, HIGH); //Sides red, middle red
  }
  if (p == 4 && wait(rDelay + lessenRed)) {
    writeStates(LOW, HIGH, LOW, LOW, LOW, HIGH); //Sides green, middle red
    digitalWrite(pRed, LOW);
    digitalWrite(pWhite, HIGH);
    resetDelays();
  }
  if (p == 5 && wait(gDelay + extendGreen)) { 
    writeStates(HIGH, LOW, LOW, LOW, LOW, HIGH); //Sides yellow, middle red
    digitalWrite(pRed, HIGH);
    digitalWrite(pWhite, LOW);
  }
  if (p == 6 && wait(yDelay + lessenSYellow)) {
    writeStates(LOW, LOW, HIGH, LOW, LOW, HIGH); //Sides red, middle red    
    if(changeGreen) { //Sets the green extension - done here if the button is pressed while green to make it start the next cycle
      extendGreen = gDelay/2; 
    } else {
      extendGreen = 0; 
    }
  }
}

/**
 * method to check if the time specified has passed, then
 * changes the light sequence to run the next phase and returns true.
 */
bool wait(int t) {
  unsigned long timeMillis = millis();
  if((unsigned long)(timeMillis-prevMillis) >= t) {
    prevMillis = timeMillis;
    
    if(p >= 6) {
      p = 1; 
    } else {
      p++;
    }
    
    return true;
  } else return false;
}

/**
 * Method to set traffic light status (on/off per LED).
 */
void writeStates(int sY, int sG, int sR, int mY, int mG, int mR) {
  digitalWrite(sYellow, sY);
  digitalWrite(sGreen, sG);
  digitalWrite(sRed, sR);
  digitalWrite(mYellow, mY);
  digitalWrite(mGreen, mG);
  digitalWrite(mRed, mR);
}

/**
 * Method to check the light level and turn on/off the streetlight.
 * If the light level is below a threshold, then the streetlight will
 * turn on; otherwise it will remain off.
 */
void streetLight() {
  if(analogRead(pr) < 50) {
    digitalWrite(street, HIGH); 
  } else {
    digitalWrite(street, LOW);
  }
}

/**
 * Method to run the gate by checking whether the IR emission
 * signal is broken (i.e., not being received anymore, indicating
 * a car is waiting).
 */
void runGate() {
  unsigned long timeGate = millis();
  if (analogRead(irRecv) > 50 && gate.read() != 60) { //Resets timer if the signal is not broken and the gate is closed; lets the timer start measuring from the time of signal obstruction
    prevMillisGate = timeGate;
  }

  if (next == false && ((unsigned long)(timeGate - prevMillisGate) >= (unsigned long)2000)) { //Opens gate
    prevMillisGate = timeGate;
    gate.write(60);
    next = true;
  } else if (next == true && ((unsigned long)(timeGate - prevMillisGate) >= (unsigned long)1500)) { //Closes gate
    prevMillisGate = timeGate;
    gate.write(0);
    next = false;
  }
}

/**
 * Method  to check for a button press and alter the traffic light
 * timings accordingly.
 * The button uses the side traffic lights as the reference; the
 * side green lights are extended. 
 * 
 * If the side traffic lights are red, then the red requirements will be followed. 
 * If the side lights are yellow, then the yellow requirements will be followed. 
 * If the side lights are green, then the green requirements will be followed.
 * 
 * The status of the middle traffic light doesn't matter - the side traffic lights' status
 * will dictate the timing change.
*/
void changeTrafficLights() {
  if(!buttonControl && digitalRead(button) == HIGH) { //Allows only one button input at a time; resets once that input's alterations are complete
    buttonControl = true;
    if(p == 1 || p == 2 || p == 3 || p == 4){ //Sides are red
      lessenRed = -rDelay/2;
      lessenGreen = -gDelay/2;
      lessenMYellow = -yDelay/2;
      lessenSYellow = -yDelay/2;
      extendGreen = gDelay/2;
    } else if(p == 6) { //Sides are yellow
      lessenSYellow = -yDelay/2;
      changeGreen = true;
    } else if(p == 5) { //Sides are green
      lessenRed = -rDelay/2;
      lessenGreen = -gDelay/2;
      lessenSYellow = -yDelay/2;
      lessenMYellow = -yDelay/2;
      changeGreen = true; //Enables green time extension code - extended later to make it start next cycle
    }
  }
}

/**
 * Method to reset the button's timer alterations.
 * The green extension is reset after running the command
 * in trafficLight(), as it operates slightly differently.
 */
void resetDelays() {
  lessenMYellow = 0;
  lessenSYellow = 0;
  lessenGreen = 0;
  lessenRed = 0;
  changeGreen = false;
  buttonControl = false;
}
