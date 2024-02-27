
//Arduino marketplace scoreboard
//https://wokwi.com/projects/new/arduino-uno

#include <Keypad.h>

int strikes = 0;
int outs = 0;
int score = 0;
int timer = 30;
int calibratedsensorValue = 0;

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

void calibrateSensor() {
  int sensorValue = analogRead(A0);
  calibratedsensorValue = (sensorValue)-50;  // set the threshold to just below current value
  Serial.print("Setting calbration to ");
  Serial.println(calibratedsensorValue);
  Serial.print("Live reading is ");
  Serial.println(sensorValue);
}

void setup() {

  pinMode(A0, INPUT);  //photoresistor / proximity sensor for pitching
  //add number/letter keybord; I2C 4 digit 7 segment display; LCD screen
  Serial.begin(9600);
  Serial.println("Starting Marketplace Project");
  calibrateSensor();
}


void loop() {
  delay(250);

  //keypad
  char customKey = customKeypad.getKey();
  if (customKey) {
  }

  //photoresistor
  int sensorValue = analogRead(A0);
  // Serial.println(sensorValue);  //pitching sensor

  if (sensorValue <= calibratedsensorValue) {
    Serial.println("timer reset");
    timer = 30;
    delay(300);
  }

  if (customKey == 'B') {
    calibrateSensor();
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


  if (outs == 3) {  //death detection
    outs = 0;
    Serial.println("Game over!");
    Serial.print("Your score was ");
    Serial.println(score);
    delay(9999999);
  }
  if (score == 10) {
    Serial.println("You win! Pick your prize(s)");
  }
  if (score >= 10) {
    Serial.println("You've won! Stop any time you want and collect your prize!");  // Line is buggy
  }
}