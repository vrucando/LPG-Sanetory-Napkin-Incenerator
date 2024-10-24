///////////////////--------ADD LIBRARIES-------------////////////////
#include <Servo.h>

///////////////////--------PIN SETUP-------------////////////////
const int pbuttonPin = 27;   // Button connected to pin 27
const int ledPin = 28;       // LED connected to pin 28
const int BzzPin = 26;       // Buzzer connected to pin 26
const int IgnPin = 2;        // Ignition pin
const int servoPin = 4;      // Servo pin

///////////////////--------VARIABLE SETUP-------////////////////
int lastButtonState = LOW;    // Previous button state
bool pushed = false;          // Button toggle state
bool timerActive = true;      // Is the timer running?

///////////////////--------DEBOUNCE SETUP-------////////////////
unsigned long lastDebounceTime = 0;  // Last time the button state changed
const unsigned long debounceDelay = 50;  // Debounce delay time (50 ms)

///////////////////--------TIMER SETUP-------////////////////
unsigned long startTime = 0;               // When the countdown started
const unsigned long countdownTime = 1 * 60 * 1000;  // 1 minute in milliseconds

///////////////////--------BLINKING SETUP-------////////////////
unsigned long lastBlinkTime = 0;           // Last time the LED blinked
const unsigned long blinkInterval = 1000;  // Blink interval (1 second)
bool blinkState = LOW;                     // Current blink state

///////////////////--------SERVO SETUP-------////////////////
Servo myservo;               // Servo object
const unsigned long onTime_S = 12000;  // Servo stays in position 1 for 12 seconds
const unsigned long offTime_S = 4000;  // Servo stays in position 2 for 4 seconds
unsigned long previousMillis_S = 0;    // Last time the servo state was updated
int interval_S = onTime_S;             // Initial interval time
bool S_state = false;                  // Servo state: false = pos2, true = pos1
int pos1 = 0;                          // Servo position 1
int pos2 = 180;                        // Servo position 2

///////////////////--------SETUP FUNCTION-------////////////////
void setup() {
  Serial.begin(9600);
  pinMode(pbuttonPin, INPUT_PULLUP); // Use internal pull-up resistor
  pinMode(ledPin, OUTPUT);           // Set LED pin as output
  pinMode(BzzPin, OUTPUT);           // Set Buzzer pin as output
  pinMode(IgnPin, OUTPUT);           // Set Ignition pin as output
  digitalWrite(ledPin, HIGH);        // Initialize LED to off
  digitalWrite(BzzPin, LOW);         // Initialize Buzzer to off
  myservo.attach(servoPin);          // Attach the servo to pin 3
  myservo.write(pos2);               // Initialize servo to position 2 (off)
  Serial.println("Press the button ....");
}

///////////////////--------SERVO CONTROL FUNCTION-------////////////////
void servoread() {
  unsigned long currentMillis = millis();  // Get current time in milliseconds

  // Servo control only when the timer is active
  if (timerActive) {
    if (currentMillis - previousMillis_S >= interval_S) {
      // Update interval based on the current servo state
      if (S_state) {
        interval_S = offTime_S;  // If at pos1, set time to stay off
      } else {
        interval_S = onTime_S;   // If at pos2, set time to stay on
      }

      // Toggle the servo state and move it
      S_state = !S_state;
      if (S_state) {
        myservo.write(pos1);  // Move to position 1
        Serial.println("Servo at Position 1");
      } else {
        myservo.write(pos2);  // Move to position 2
        Serial.println("Servo at Position 2");
      }

      // Update the last time the servo state was changed
      previousMillis_S = currentMillis;
    }
  } else {
    // If the timer is not active, move the servo to position 2
    myservo.write(pos2);  // Servo goes to the resting position
  }
}

///////////////////--------TIMER FUNCTION-------////////////////
void timer() {
  unsigned long currentTime = millis();  // Get the current time
  unsigned long elapsedTime = currentTime - startTime;  // Calculate elapsed time

  // Calculate remaining time
  unsigned long remainingTime = countdownTime > elapsedTime ? countdownTime - elapsedTime : 0;

  // Convert remaining time into minutes and seconds
  int minutes = remainingTime / 60000;
  int seconds = (remainingTime % 60000) / 1000;

  // Print the time in "min:sec" format
  Serial.print("Time remaining: ");
  Serial.print(minutes);
  Serial.print(":");
  if (seconds < 10) {
    Serial.print("0");  // Leading zero for single-digit seconds
  }
  Serial.println(seconds);

  // Turn off the LED and stop the timer when time runs out
  if (remainingTime == 0) {
    Serial.println("Time up! Timer finished. LED turned off.");
    digitalWrite(ledPin, HIGH);   // Turn off the LED
    digitalWrite(BzzPin, LOW);    // Turn off the buzzer
    timerActive = false;          // Stop the timer
    return;
  }

  // Blink the LED and buzzer every 1 second
  if (currentTime - lastBlinkTime >= blinkInterval) {
    blinkState = !blinkState;                // Toggle blink state
    digitalWrite(ledPin, blinkState);        // Update LED to blink
    digitalWrite(BzzPin, blinkState);        // Update buzzer to beep
    lastBlinkTime = currentTime;             // Reset blink timer
  }
}

///////////////////--------MAIN LOOP FUNCTION-------////////////////
void loop() {
  int reading = digitalRead(pbuttonPin);  // Read the button state

  // Check if button state has changed (debounce logic)
  if (reading != lastButtonState) {
    lastDebounceTime = millis();  // Reset debounce timer
  }

  // If debounce delay has passed, check the button state
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == HIGH && !pushed) {
      pushed = true;  // Button is pressed
      Serial.println("Button pressed!");

      if (!timerActive) {
        startTime = millis();       // Start timer
        lastBlinkTime = millis();   // Reset blink timer
      }
      timerActive = !timerActive;   // Toggle timer state
      Serial.println(timerActive ? "SYSTEM ON" : "SYSTEM OFF");
    }

    // Reset push state when the button is released
    if (reading == LOW) {
      pushed = false;
    }
  }

  // Timer logic
  if (timerActive) {
    timer();        // Handle countdown, blinking, and buzzer when active
    servoread();    // Control the servo based on timer state
  } else {
    myservo.write(pos2);  // Move the servo to position 2 when the timer is off
  }
  digitalWrite(IgnPin, timerActive);        // Update LED to blink
  lastButtonState = reading;  // Save the current button state
}
