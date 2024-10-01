#include <Keypad.h>

const byte ROWS = 4; // four rows
const byte COLS = 4; // four coloms

// Bind to array
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', '4'},
  {'5', '6', '7', '8'},
  {'9', 'A', 'B', 'C'},
  {'D', 'E', 'F', 'G'}
};

byte colPins[COLS] = {5, 4, 3, 2}; // Pins used for coloms keypad
byte rowPins[ROWS] = {6, 7, 8, 9}; // Pins used for rows keypad

// Initialiseer het Keypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(9600);   // seriele monitor
}

void loop() {

  // Read my pressed button
  char button = customKeypad.getKey();

  if (button) {
    Serial.println(button);
  }
}