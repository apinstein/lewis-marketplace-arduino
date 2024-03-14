
//Arduino marketplace scoreboard
//https://wokwi.com/projects/new/arduino-uno
#include <TM1637Display.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <avr/io.h>
#include <avr/interrupt.h>

// PINS
#define PROX_SENSOR_PIN 2
#define SEVEN_SEG_CLOCK_PIN 12
#define SEVEN_SEG_DIO_PIN 13

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
#define TOGGLE_TIMER 'C'

// How long does a new clock last for a turn
#define TURN_DURATION 30

// Set the LCD address to 0x27 for a 16 chars and 2 line display
// Adjust the address as needed, which can be found using an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long lcdGOdsp = 0;  // time when game ended
bool isPaused = false;

int turnSecsRemaining = TURN_DURATION;
unsigned long turnLastSecondCountdown = 0;  // millis() of last time we reduced turnSecsRemaining
bool timerRunning = false;                  // Indicates whether the timer is currently running
bool timerButtonToggle = false;

bool firstBase = false;
bool secondBase = false;
bool thirdBase = false;

int strikes = 0;
int outs = 0;
int score = 0;
int calibratedOutSensorValue = 0;

// Create a display object of type TM1637Display
TM1637Display display = TM1637Display(SEVEN_SEG_CLOCK_PIN, SEVEN_SEG_DIO_PIN);

// Create an array that turns all segments ON
const uint8_t allON[] = { 0xff, 0xff, 0xff, 0xff };

// Create an array that turns all segments OFF
const uint8_t allOFF[] = { 0x00, 0x00, 0x00, 0x00 };

const byte ROWS = 4;  //four rows
const byte COLS = 4;  //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte rowPins[ROWS] = { 3, 4, 5, 6 };   //connect to the row pinouts of the keypad
byte colPins[COLS] = { 7, 8, 9, 10 };  //connect to the column pinouts of the keypad
//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

volatile bool timerFlag = false;
void setupOneSecondTimerInterrupt() {
  cli();  // Disable global interrupts

  // Configure Timer1 for a 1-second interrupt
  TCCR1A = 0;  // Set entire TCCR1A register to 0
  TCCR1B = 0;  // Same for TCCR1B

  // Set compare match register to desired timer count:
  // 16MHz Clock / 256 prescaler / 1Hz (1 second)
  OCR1A = 62499;  // = (16*10^6) / (256*1) - 1 (must be <65536)

  // Turn on CTC mode (Clear Timer on Compare Match)
  TCCR1B |= (1 << WGM12);

  // Set CS12 bit for 256 prescaler
  TCCR1B |= (1 << CS12);

  // Enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();  // Enable global interrupts
}

ISR(TIMER1_COMPA_vect) {
  timerFlag = true;
}

void checkTimerDecrement() {
  if (timerFlag) {
    timerFlag = false;
    // Serial.println("1 second passed");

    if (timerRunning) {
      turnSecsRemaining--;
      printTimerClean();
    }
  }
}

volatile bool needToHandleBallDetected = false;
void ballDetected() {
  needToHandleBallDetected = true;
}

unsigned long lastBallDetected = 0;
unsigned long ballDebounceThresholdMS = 750;
void handleBallDetected() {
  // Code to execute when the ball is detected

  // when did we last see the ball?
  unsigned long currentBallDetected = millis();

  if (currentBallDetected - lastBallDetected > ballDebounceThresholdMS) {
    // record current ball detection as last for tracking new debounce cycle
    lastBallDetected = currentBallDetected;
    startTimer();
  }
}


void calibrateSensor() {
  int outsensorValue = analogRead(A1);
  calibratedOutSensorValue = outsensorValue - 50;  // set the threshold to just below current value
  Serial.print("Setting outs calbration to ");
  Serial.println(calibratedOutSensorValue);
  Serial.print("Live reading is ");
  Serial.println(outsensorValue);
}

//TIMER FUNCTIONS START
void startTimer() {
  if (!timerRunning) {
    timerRunning = true;  // Mark the timer as running
    Serial.println("Timer started");
    printTimerClean();
  }
}

void pauseTimer() {
  timerRunning = false;  // Mark the timer as not running
  // Serial.println("Timer paused");
}

void resetTimer() {
  turnSecsRemaining = TURN_DURATION;
  Serial.println("Timer reset");
}

unsigned long getCurrentTime() {
  return turnSecsRemaining;
}


void displayScore() {        //LCD DISPLAY LOGIC
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
      snprintf(lcdDisplay[0], sizeof(lcdDisplay[0]), "Game Over!         ");
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
  int timerAmount = turnSecsRemaining;
  if (timerAmount != lastTimerAmount) {
    lastTimerAmount = timerAmount;

    // Serial.println("Updating Timer Display");
    int seven_seg_complete = turnSecsRemaining * 100 + score;
    // Serial.println(score);pauseTim
    Serial.println(seven_seg_complete);
    display.showNumberDecEx(seven_seg_complete);  // enable colon
    //0b01000000
    // Serial.println(timerAmount);
    delay(100);
  }
}

void markGameOver() {
  lcdGOdsp = millis();  // log time of game over
}

void addOut() {
  outs += 1;
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
  setupOneSecondTimerInterrupt();
  delay(1000);  // Wait a bit before initializing serial to reduce startup noise
  pinMode(A1, INPUT);
  pinMode(A0, INPUT);  // Photoresistor / proximity sensor for pitching
  Serial.begin(9600);
  lcd.init();                // Initialize the LCD
  lcd.backlight();           // Turn on the backlight
  display.setBrightness(5);  // Turn on the 7 seg display
  display.clear();
  while (!Serial)
    ;  // Wait for Serial port to connect - necessary for Leonardo, Micro, etc.
  pinMode(PROX_SENSOR_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(PROX_SENSOR_PIN), ballDetected, CHANGE);
  Serial.println("Setup complete");  // First message to indicate setup is done
  calibrateSensor();
  displayScore();
}

void loop() {  //VOID LOOP START

  if (needToHandleBallDetected) {
    handleBallDetected();
    needToHandleBallDetected = false;
  }

  checkTimerDecrement();
  printTimerClean();
  displayScore();
  //keypad

  char customKey = customKeypad.getKey();

  //keypad c = pause timer
  if (customKey == TOGGLE_TIMER) {
    Serial.println("toggle timer");
    timerButtonToggle = !timerButtonToggle;
    if (timerButtonToggle == true) {
    Serial.println("pause timer");
      pauseTimer();

    } else {
          Serial.println("unpause timer");

      startTimer();
    }
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
    calibrateSensor();
  }
  //photoresitor outs
  int outsensorValue = analogRead(A1);

  if (outsensorValue <= calibratedOutSensorValue) {
    pauseTimer();
    turnSecsRemaining += 3;
    addOut();
    delay(1000);
  }
  //reset all
  if (customKey == RESET_ALL) {
    outs = 0;
    score = 0;
    strikes = 0;
    resetTimer();
    pauseTimer();
    lcdGOdsp = 0;  // reset game over event time
    Serial.println("Resetting all");
  }
  //timer = 0 + out
  if (getCurrentTime() == 0) {
    strikes = strikes + 2;
    resetTimer();
    pauseTimer();
  }
  //strike outs score plus
  if (customKey == ADD_SCORE) {
    Serial.println("blach");
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
}