
//Arduino marketplace scoreboard
//https://wokwi.com/projects/new/arduino-uno

#include <Keypad.h>

bool isPaused = false;

unsigned long timerStart = 0; // Stores the last time the timer was started
unsigned long timerDuration = 30000; // Timer duration in milliseconds (30 seconds)
bool timerRunning = false; // Indicates whether the timer is currently running

int strikes = 0;
int outs = 0;
int score = 0;
int calibratedsensorValueP = 0;
int calibratedsensorValueO = 0;

const byte ROWS = 4;  //four rows
const byte COLS = 4;  //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte rowPins[ROWS] = { 2, 3, 4, 5 };  //connect to the row pinouts of the keypad
byte colPins[COLS] = { 6, 7, 8, 9 };  //connect to the column pinouts of the keypad
//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void calibrateSensorP() {
  int pitchingsensorValue = analogRead(A0);
  calibratedsensorValueP = pitchingsensorValue - 50;  // set the threshold to just below current value
  Serial.print("Setting pitching calbration to ");
  Serial.println(calibratedsensorValueP);
  Serial.print("Live reading is ");
  Serial.println(pitchingsensorValue);
}

void calibrateSensorO() {
  int outsensorValue = analogRead(A1);
  calibratedsensorValueO = outsensorValue - 50;  // set the threshold to just below current value
  Serial.print("Setting outs calbration to ");
  Serial.println(calibratedsensorValueO);
  Serial.print("Live reading is ");
  Serial.println(outsensorValue);
}

void enterPauseState() {
  isPaused = true; // Enter paused state
  while (isPaused) {
    // Check for a condition to exit pause, e.g., a specific key press
    char customKey = customKeypad.getKey();
    if (customKey == 'R') { // Assuming 'R' is the resume key
      isPaused = false; // Exit paused state
    }
    // Optionally, include a small delay to prevent the loop from running too fast
    delay(100);
  }
}

void startTimer() {
  if (!timerRunning) {
    timerStart = millis(); // Update start time
    timerRunning = true; // Mark the timer as running
    Serial.println("Timer started");
    printTimerClean();
  }
}

void pauseTimer() {
  timerRunning = false; // Mark the timer as not running
  Serial.println("Timer paused");
}

void resetTimer() {
  timerRunning = false; // Ensure the timer is not running
  timerStart = millis(); // Reset the start time to now, for consistency
  Serial.println("Timer reset");
}

unsigned long getCurrentTime() {
  if (timerRunning) {
    // Calculate the time elapsed since the timer started
    return timerDuration - (millis() - timerStart);
  } else {
    // If the timer is paused or stopped, return the time until it was paused/stopped
    return timerDuration; // Adjust this logic as needed
  }
}

void setup() { delay(100);
  pinMode(A1, INPUT);
  pinMode(A0, INPUT);  //photoresistor / proximity sensor for pitching
  //add I2C 4 digit 7 segment display; LCD screen
  Serial.begin(9600);
  calibrateSensorP();
  calibrateSensorO();
}

int lastTimerAmount = 0;
void printTimerClean()
{
  int timerAmount = getCurrentTime() / 1000;
  if (timerAmount != lastTimerAmount) {
    Serial.println(timerAmount);
    lastTimerAmount = timerAmount;
  }
}

void loop() {
  printTimerClean();

  //keypad
  char customKey = customKeypad.getKey();
  if (customKey) {
  }

  // photoresistor pitching
  int pitchingsensorValue = analogRead(A0);

  if (pitchingsensorValue <= calibratedsensorValueP) {
    Serial.println("timer reset");
    resetTimer();
  delay(50);
  startTimer();
    delay(1000);
    //add func timer reset + timer start
  }

  if (customKey == 'B') {
    calibrateSensorP();
    calibrateSensorO();
  }
  //photoresitor outs
  int outsensorValue = analogRead(A1);

  if (outsensorValue <= calibratedsensorValueO) {
    Serial.println("adding out");
    delay(1000);
    pauseTimer();
    outs = outs + 1;
  }
  //reset all
  if (customKey == 'A') {
    outs = 0;
    score = 0;
    strikes = 0;
    resetTimer();
  }
//timer = 0 + out
if (getCurrentTime() == 0) {
  outs = outs + 1;
  resetTimer();
}
  //strike outs score plus
  if (customKey == '1') {
    score = score + 1;
    Serial.println("----");

    Serial.print("Score ");
    Serial.println(score);

    Serial.print("Strikes ");
    Serial.println(strikes);

    Serial.print("Outs ");
    Serial.println(outs);

    delay(750);
  }

  if (customKey == '4') {  //add strikes
    strikes = strikes + 1;
    Serial.println("----");

    Serial.println(score);
    Serial.print("Score ");

    Serial.println(strikes);
    Serial.print("Strikes ");

    Serial.print(outs);
    Serial.println("Outs ");

    delay(750);
  }

  if (customKey == '7') {  //add outs
    outs = outs + 1;
    Serial.println("----");

    Serial.println(score);
    Serial.print("Score ");

    Serial.println(strikes);
    Serial.print("Strikes ");

    Serial.print(outs);
    Serial.println("Outs ");

    delay(750);
  }

  //score strike outs minus
  if (customKey == '2') {  //subtract score
    score = score - 1;
    Serial.println("----");

    Serial.println(score);
    Serial.print("Score ");

    Serial.println(strikes);
    Serial.print("Strikes ");

    Serial.print(outs);
    Serial.println("Outs ");

    delay(750);
  }

  if (customKey == '5') {  //subtract strikes
    strikes = strikes - 1;
    Serial.println("----");

    Serial.println(score);
    Serial.print("Score ");

    Serial.println(strikes);
    Serial.print("Strikes ");

    Serial.print(outs);
    Serial.println("Outs ");

    delay(750);
  }

  if (customKey == '8') {  //subtract outs
    outs = outs - 1;
    Serial.println("----");

    Serial.println(score);
    Serial.print("Score ");

    Serial.println(strikes);
    Serial.print("Strikes ");

    Serial.print(outs);
    Serial.println("Outs ");

    delay(750);
  }

  if (strikes == 3) {  //3 strikes = 1 out
    strikes = 0;
    outs = outs + 1;
    Serial.println("----");

    Serial.println(score);
    Serial.print("Score ");

    Serial.println(strikes);
    Serial.print("Strikes ");

    Serial.print(outs);
    Serial.println("Outs ");
  }

  if (outs == 3) {  // Game over condition
  Serial.println("Game over!");
  Serial.print("Your score was ");
  Serial.println(score);
  enterPauseState(); // Pause the program instead of using delay
}

  if (score == 10) {
    Serial.println("You win! Pick your prize(s)");
  }
  if (score >= 10) {
    Serial.println("You've won! Stop any time you want and collect your prize!");  // Line is buggy
  }
}