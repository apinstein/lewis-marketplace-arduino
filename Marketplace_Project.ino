
//Arduino marketplace scoreboard
//https://wokwi.com/projects/new/arduino-uno

#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define FIRST_BASE_TOGGLE '3'
#define SECOND_BASE_TOGGLE '6'
#define THIRD_BASE_TOGGLE '9'
#define ADD_SCORE '1'
#define SUBTRACT_SCORE '2'
#define ADD_STRIKES '4'
#define SUBTRACT_STRIKES '5'
#define ADD_OUTS '7'
#define SUBTRACT_OUTS '8'
#define RESET_ALL 'A'
#define RECALIBRATE_SENSORS 'B'
#define RESET_PAUSE_TIMER 'C'

// Set the LCD address to 0x27 for a 16 chars and 2 line display
// Adjust the address as needed, which can be found using an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long lcdGOdsp = 0;  // time when game ended
bool isPaused = false;
unsigned long timerStart = 0;         // Stores the last time the timer was started
unsigned long timerDuration = 30000;  // Timer duration in milliseconds (30 seconds)
bool timerRunning = false;            // Indicates whether the timer is currently running
bool timerJustStarted = false;        // Flag to handle initial timer start
bool firstBase = false;
bool secondBase = false;
bool thirdBase = false;

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

//TIMER FUNCTIONS START
void startTimer() {
  if (!timerRunning) {
    timerStart = millis();  // Update start time
    timerRunning = true;    // Mark the timer as running
    Serial.println("Timer started");
    printTimerClean();
  }
}

void pauseTimer() {
  timerRunning = false;  // Mark the timer as not running
  // Serial.println("Timer paused");
}

void resetTimer() {
  timerRunning = false;   // Ensure the timer is not running
  timerStart = millis();  // Reset the start time to now, for consistency
  Serial.println("Timer reset");
}

unsigned long getCurrentTime() {
  if (timerRunning) {
    // Calculate the time elapsed since the timer started
    return timerDuration - (millis() - timerStart);
  } else {
    // If the timer is paused or stopped, return the time until it was paused/stopped
    return timerDuration;  // Adjust this logic as needed
  }
}


void displayScore() {  //LCD DISPLAY LOGIC
  char lcdDisplay[2][17] = { // 17 is b/c 16 chars + NULL string terminator
    "",
    ""
  };
  // structure lcdDisplay[0] and lcdDisplay[1] as desried
  snprintf(lcdDisplay[0], sizeof(lcdDisplay[0]), "Strikes %d|Outs %d", strikes, outs);
  snprintf(lcdDisplay[1], sizeof(lcdDisplay[1]), " _ _ _ | Bases       ");

  lcdDisplay[1][1] = firstBase ? '1' : '_';
  lcdDisplay[1][3] = secondBase ? '2' : '_';
  lcdDisplay[1][5] = thirdBase ? '3' : '_';

  if (lcdGOdsp) {  // Game over condition
    if (millis() - lcdGOdsp < 3000) {
      // display "game over" for a few seconds
      snprintf(lcdDisplay[0], sizeof(lcdDisplay[0]),  "Game Over!         ");
      snprintf(lcdDisplay[1], sizeof(lcdDisplay[1]), "Your score is %d", score);
    } else if (score >= 10) {
      snprintf(lcdDisplay[0], sizeof(lcdDisplay[0]), "Congratulations!");
      snprintf(lcdDisplay[1], sizeof(lcdDisplay[1]), "You get a prize!");
    } else {  // lost
      snprintf(lcdDisplay[0], sizeof(lcdDisplay[0]), "No prize! ");
      snprintf(lcdDisplay[1], sizeof(lcdDisplay[1]), "Not enough runs!");
    }
  }

  // send line 1 to display
  lcd.setCursor(0, 0);
  lcd.print(lcdDisplay[0]);

  // send line 2 to display
  lcd.setCursor(0, 1);
  lcd.print(lcdDisplay[1]);
}

void plotValues() {  //DEBUG PHOTORESISTORS
  char plotterData[100];
  // pitching sensor data
  int pitchingsensorValue = analogRead(A0);

  // out sensor data
  int outsensorValue = analogRead(A1);
  snprintf(plotterData, sizeof(plotterData), "%d,%d", pitchingsensorValue, outsensorValue);
  Serial.println(plotterData);
}

int lastTimerAmount = 0;
void printTimerClean() {
  int timerAmount = getCurrentTime() / 1000;
  if (timerAmount != lastTimerAmount) {
    lastTimerAmount = timerAmount;
  }
}

void markGameOver() {
  lcdGOdsp = millis();  // log time of game over
}

void addOut() {
  return;
  if (outs < 3) {
    outs += 1;

    // game over?
    if (outs == 3) {
      markGameOver();
    }
  }
}


void setup() {  //VOID SETUP START
  delay(1000);  // Wait a bit before initializing serial to reduce startup noise
  pinMode(A1, INPUT);
  pinMode(A0, INPUT);  // Photoresistor / proximity sensor for pitching
  Serial.begin(9600);
  lcd.init();       // Initialize the LCD
  lcd.backlight();  // Turn on the backlight
  while (!Serial)
    ;                                // Wait for Serial port to connect - necessary for Leonardo, Micro, etc.
  Serial.println("Setup complete");  // First message to indicate setup is done
  calibrateSensorP();
  calibrateSensorO();
  displayScore();
}

void loop() {  //VOID LOOP START
  printTimerClean();
  displayScore();
  //keypad
  char customKey = customKeypad.getKey();
  if (customKey) {
  }

  //keypad c = pause timer
  if (customKey == 'RESET_PAUSE_TIMER') {
    resetTimer();
    pauseTimer();
  }
  // photoresistor pitching
  int pitchingsensorValue = analogRead(A0);

  if (pitchingsensorValue <= calibratedsensorValueP) {
    startTimer();
    strikes += 1;
    if (getCurrentTime <= 10) {
      timerDuration = 30000;
    }
    // delay(1000);
  }

  //bases on keypad
  if (customKey == FIRST_BASE_TOGGLE) {
    firstBase = !firstBase;
  }

  if (customKey == SECOND_BASE_TOGGLE) {
    secondBase = !secondBase;
  }
  if (customKey == THIRD_BASE_TOGGLE) {
    thirdBase = !thirdBase;
  }


  if (customKey == RECALIBRATE_SENSORS) {
    calibrateSensorP();
    calibrateSensorO();
  }
  //photoresitor outs
  int outsensorValue = analogRead(A1);

  if (outsensorValue <= calibratedsensorValueO) {
    pauseTimer();
    timerDuration += 5000;
    addOut();
    // delay(1000);
  }
  //reset all
  if (customKey == RESET_ALL) {
    outs = 0;
    score = 0;
    strikes = 0;
    resetTimer();
    lcdGOdsp = 0;  // reset game over event time
    Serial.println("Resetting all");
  }
  //timer = 0 + out
  if (getCurrentTime() == 0) {
    strikes = strikes + 1;
    resetTimer();
  }
  //strike outs score plus
  if (customKey == ADD_SCORE) {
    score = score + 1;
    if (score == 10) {
      markGameOver();
    }
  }

  if (customKey == ADD_STRIKES) {  //add strikes
    strikes = strikes + 1;
  }

  if (customKey == ADD_OUTS) {  //add outs
    addOut();
  }

  //score strike outs minus
  if (customKey == SUBTRACT_SCORE) {  //subtract score
    score = score - 1;
  }

  if (customKey == SUBTRACT_STRIKES) {  //subtract strikes
    strikes = strikes - 1;
  }

  if (customKey == SUBTRACT_OUTS) {  //subtract outs
    outs = outs - 1;
  }

  if (strikes == 3) {  //3 strikes = 1 out
    strikes = 0;
    addOut();
  }

  plotValues();
}