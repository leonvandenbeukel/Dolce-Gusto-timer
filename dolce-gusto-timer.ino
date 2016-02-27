/*
 * Arduino based Dolce Gusto timer
 * 
 * Leon van den Beukel - Feb 2016
 * 
 * https://github.com/leonvandenbeukel/Dolce-Gusto-timer
 * 
 */

#include <Event.h>
#include <Timer.h>
#include <Servo.h>

// Instantiate the timer object
Timer t;                               
Servo myservo;
#define servoPin 10

int pos = 0;
int potPin = 4;
int potVal = 0;

// Define servo positions
#define NEUTRAL 77
#define HOT 180
#define COLD 0

// Pin connected to ST_CP of 74HC595
int latchPin = 8;
// Pin connected to SH_CP of 74HC595
int clockPin = 12;
// Pin connected to DS of 74HC595
int dataPin = 11;

/*
128  level value 
64   7     127   all on
32   6     63  
16   5     31  
8    4     15  
4    3     7 
2    2     3 
1    1     1 
     0     0     all off
*/
int levels[8] = {0,1,3,7,15,31,63,127};     // Values for shift register to indicate level
int times[8]  = {0,8,10,15,21,24,28,38};    // Timer seconds based on Dolce Gusto levels

int level = 0;
double currentTimer = 0;

const int buttonPin = 2;    // the number of the pushbutton pin
const int ledPin = 3;       // the number of the LED pin

// Variables will change:
int ledState = LOW;          // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;   // the last time the output pin was toggled
long debounceDelay = 50;     // the debounce time; increase if the output flickers

int state = LOW;

void setup(void)
{ 
  // Attach and set servo to neutral state
  myservo.attach(servoPin);
  delay(10);  
  myservo.write(NEUTRAL);

  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(potPin, INPUT);

  // Set initial LED state
  digitalWrite(ledPin, ledState);
  
  // Set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
    
  // Set the timer callbacks
  t.every(1000, checkUpdate);
  t.every(200, checkPot);

}

void loop(void)
{
  // Update the timers
  t.update();

  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        ledState = !ledState;
        state = !state;

        // start
        if (state == HIGH) {
          currentTimer = times[level];
        }
      }
    }
  }

  // set the LED:
  digitalWrite(ledPin, ledState);

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;

}

void checkPot() {

  // Check the pot value and map to the level of leds
  potVal = analogRead(potPin);
  level = map(potVal, 0, 1023, 0, 7);

  // Output to the shift register
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, levels[level]);  
  digitalWrite(latchPin, HIGH);
  
}

void checkUpdate() {

  // Check if the servo should enable the Dolce Gusto switch 
  if (state == HIGH && currentTimer > 0) {
    // decrease a period
    currentTimer -= 1;  
    myservo.write(HOT);      
  } else if (state == HIGH && currentTimer == 0) {
    myservo.write(NEUTRAL);  
    state = LOW;
    ledState = LOW;
    digitalWrite(ledPin, ledState);  
  } else {
    myservo.write(NEUTRAL);  
  }
 
}

