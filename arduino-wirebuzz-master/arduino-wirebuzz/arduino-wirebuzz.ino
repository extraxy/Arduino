// Xmas Song Lib
#include "pitches.h"
// Start of melody information Define is now BUZZ_PIN NOT BUZZER
#define BUZZ_PIN 6

int melody[] = {
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_G5, NOTE_C5, NOTE_D5,
  NOTE_E5,
  NOTE_F5, NOTE_F5, NOTE_F5, NOTE_F5,
  NOTE_F5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_D5, NOTE_D5, NOTE_E5,
  NOTE_D5, NOTE_G5 
  };

int durations[] = {
  8, 8, 4,
  8, 8, 4,
  8, 8, 8, 8,
  2,
  8, 8, 8, 8,
  8, 8, 8, 16, 16,
  8, 8, 8, 8,
  4, 4
};


// CONSTANTS
// The "start zone" which players must touch to start the game each time
const byte startPin = 8;
// Touching any part of the wire itself causes a failure
const byte failPin = 9;
// The "win zone" at the end of the wire
const byte endPin = 10;
// A piezo buzzer chirps to signify success/failure
const byte buzzerPin = 6;
//release on win 
const byte relayPin = 13;
//This is my led
const byte ledPin = 7;
// GLOBALS
// Keep track of the current states of the game
enum GameState {FAILED, IN_PROGRESS, SUCCESS};
GameState gameState = GameState::FAILED;

void setup() {
  pinMode(BUZZ_PIN, OUTPUT);
  // Set the pins to the correct mode
  pinMode(startPin, INPUT_PULLUP);
  pinMode(failPin, INPUT_PULLUP);
  pinMode(endPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);

  // Begin a serial connection for debugging
  Serial.begin(9600);
}

void loop() {
  switch(gameState) {
   
    case GameState::IN_PROGRESS:
      if (!digitalRead(endPin)) {
        gameState = GameState::SUCCESS;
        Serial.println("Congratulations!");
        // TEMP REMOVE OLD SOUND
        //tone(buzzerPin, 440, 50);
        //delay(60);
        //tone(buzzerPin, 587, 250);
        digitalWrite(ledPin, HIGH);
        delay(500);
        digitalWrite(ledPin, LOW);
        delay(500);
        digitalWrite(ledPin, HIGH);
        delay(500);
        digitalWrite(ledPin, LOW);
        delay(500);

        int size = sizeof(durations) / sizeof(int);

        for (int note = 0; note < size; note++) {
          //to calculate the note duration, take one second divided by the note type.
          //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
          int duration = 1000 / durations[note];
          tone(BUZZ_PIN, melody[note], duration);

          //to distinguish the notes, set a minimum time between them.
          //the note's duration + 30% seems to work well:
          int pauseBetweenNotes = duration * 1.30;
          delay(pauseBetweenNotes);
      
          //stop the tone playing:
          noTone(BUZZ_PIN);
          
        }
        
      } else if (!digitalRead(failPin)) {
        gameState = GameState::FAILED;
        Serial.println("FAILED");
        tone(buzzerPin, 440, 200);
        delay(200);
        tone(buzzerPin, 415, 200);
        delay(200);
        tone(buzzerPin, 392, 200);
        delay(200);
        tone(buzzerPin, 370, 400);
        digitalWrite(ledPin, HIGH);
        
      }
      break;

    case GameState::FAILED:
    case GameState::SUCCESS:
      //Serial.println("SUCCES!");
      digitalWrite(relayPin, LOW);
    
      if(!digitalRead(startPin)) {
        gameState = GameState::IN_PROGRESS;
        digitalWrite(ledPin, LOW);
        digitalWrite(relayPin, HIGH);
        Serial.println("New Game Started");
        tone(buzzerPin, 440, 100);
        delay(120);
        tone(buzzerPin, 554, 100);
        delay(120);
        tone(buzzerPin, 659, 200);
      }
    break;    
  }
}
