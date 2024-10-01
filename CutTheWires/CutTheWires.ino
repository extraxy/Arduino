/**
 * "Cut the Wires" defusable bomb puzzle
 *
 * Players must cut the correct wire(s) in order to stop a ticking clock before it reaches 0.
 * If the bomb explodes, it publishes a message to an MQTT broker which can trigger appropriate
 * lighting/sound effects
 */

// INCLUDES
// Include the platform-specific WiFi library
#if defined(ESP8266) // ESP8266
  #include <ESP8266WiFi.h>
#elif defined(ESP32) || defined(__AVR__) // ESP32 or Arduino
  #include <WiFi.h>
#endif
// PubSubClient used for MQTT message brokering over either ethernet or Wi-Fi
// See from https://github.com/knolleary/pubsubclient
#include <PubSubClient.h>
// 4-digit 7-segment LED Display library, download from https://github.com/avishorp/TM1637
#include "src/TM1637Display.h"

// CONSTANTS
const byte numWires = 4;
// Note that Wemos D1 mini has PULLDOWN resistor on D8, but no PULLUP
const int wirePins[numWires] = {D7, D6, D5, D1};
// Amount of time (in ms) to be deducted when an incorrect wire is cut
const unsigned long timePenalty = 300000;
// SSID of the network to join
const char* ssid = "ZiggoB63E55B";
// Wi-Fi password if required
const char* password = "sj3wj3xsyKjx";
// The IP address of the MQTT server
const IPAddress server(192,168,178,242);
// The port on which the MQTT server is running
const int port = 1883;
// Unique ID used to identify this device on the MQTT network
char* deviceID = "100";

// GLOBALS
// Create a display with the specified CLK/DIO lines
TM1637Display display(D2, D3);
int lastState[numWires];
WiFiClient networkClient;
// Create a propcontrol object based on the chosen network
PubSubClient pc(server, port, networkClient);
// What is the order in which wires need to be cut
// 0 indicates the wire should not be cut!
int wiresToCut[numWires] = {1, 0, 0, 0};
byte wiresCutCounter = 1;
// Keep track of the current state of the device
enum State {Inactive, Active, Defused, Exploded};
State state = State::Inactive;
// This is the timestamp at which the bomb will detonate
// It is calculated by adding on the specified numnber of minutes in the game time
// to the value of millis() when the code initialised.
unsigned long detonationTime;
// The game length (in minutes)
int gameDuration = 10;

void Activate(){
  state = State::Active;
  // Set the detonation time to the appropriate time in the future
  detonationTime = millis() + (unsigned long)gameDuration*60*1000;
  Serial.println(F("Bomb activated!"));
}

void Deactivate() {
  Serial.println(F("Bomb defused"));
  state = State::Inactive;
}

void Detonate() {
  state = State::Exploded;
  Serial.println("BOOM!");
  pc.publish("ToHost", "BOOM");
}


void setup() {
  // Start the serial connection (used for debugging only)
  Serial.begin(9600);
  // Print some useful debug output - the filename and compilation time
  Serial.println(__FILE__);
  Serial.println("Compiled: " __DATE__ ", " __TIME__);

  #if defined(ESP8266)
    WiFi.mode(WIFI_STA);
  #endif
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(F("done."));
  Serial.print(F("IP Address: "));
  Serial.println(WiFi.localIP());

  // Initialize LED and set to off
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialise wire pins
  for(int i=0; i<numWires; i++){
    pinMode(wirePins[i], INPUT_PULLUP);
    lastState[i] = digitalRead(wirePins[i]);
  }
 
  // Set the detontation time to the appropriate time in the future
  detonationTime = millis() + (unsigned long)gameDuration*60*1000;

  // Specify the callback that processes any messages received on an MQTT topic to which we are subscribed
  pc.setCallback([](char* topic, byte* payload, unsigned int length) {
    // Copy the payload received to a character string
    char msg[200];
    memcpy(msg, payload, length);
    msg[length] = '\0';
    Serial.print(F("Received over MQTT:"));
    Serial.println(msg);
     
    // Perform the appropriate action depending on the topic in which the message was published
    if(strstr(topic, "RESET") != NULL) {
      Serial.print(F("Activating"));
      Activate();
    }
    if(strcmp(msg, "SOLVE") == 0) {
      Serial.print(F("Deactivating"));
      Deactivate();
    }
  });

  // Turn on the countdown display
  display.setBrightness(0x0c, true);
  uint8_t status[] = {
  SEG_A | SEG_D,
  SEG_A | SEG_D,
  SEG_A | SEG_D,
  SEG_A | SEG_D
  };
  // Print the initial state of the wires
  for(int i=0; i<numWires; i++){
    Serial.print("Wire ");
    Serial.print(i);
    Serial.print(" pin(");
    Serial.print(wirePins[i]);
    Serial.print(") :");
    Serial.println(digitalRead(wirePins[i])? "Unconnected" : "Connected");
    if(!digitalRead(wirePins[i])) {
      status[i] |= SEG_F | SEG_E | SEG_B | SEG_C;  // Turns = into [=]
    }
  }
  display.setSegments(status);
  delay(2000);

  // Arm the bomb!
  Activate();
};


void loop() {
  // Ensure there's a connection to the MQTT server
  while(!pc.connected()) {
    if(pc.connect(deviceID)) {
      // Subscribe to topics meant for this device
      char topic[32];
      snprintf(topic, 32, "ToDevice/%s/#", deviceID);
      pc.subscribe(topic);
      Serial.println(F("Connected to MQTT server"));
    } 
    else {
      Serial.print(F("Could not connect to MQTT server, rc="));
      Serial.println(pc.state());
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }    
  // Call the main MQTT loop to check for and publish messages
  pc.loop();

  // First, see if any of the wires have been recently cut
  for(int i=0; i<numWires; i++){
    // If the previous reading was LOW, but now it's HIGH, that means this wire must just have been cut
    //Serial.println(lastState[i]); 
    if(digitalRead(wirePins[i]) == HIGH && lastState[i] == LOW) {
      Serial.print("Wire ");
      Serial.print(i);
      Serial.println(" cut");
      digitalWrite(LED_BUILTIN, HIGH);
      delay(5);
      digitalWrite(LED_BUILTIN, LOW);
      delay(5);
      lastState[i] = HIGH;

      // Was this the correct wire to cut?
      if(wiresCutCounter == wiresToCut[i]){
        // Go on to the next counter
        wiresCutCounter++;
      }
      // Incorrect wire cut
      else {
        // Subtract the penalty from the time remaining
        detonationTime -= timePenalty;
      }
    }
    // If the previous reading was LOW, but now it's HIGH, that means this wire must just have been cut
    else if(digitalRead(wirePins[i]) == LOW && lastState[i] == HIGH) {
      Serial.print("Wire ");
      Serial.print(i);
      Serial.println(" reconnected");
      lastState[i] = LOW;
      //Activate();
    }
  }

  // Now, test the current state of all wires against the solution state
  // First, assume that the correct wires have all been cut
  bool allWiresCut = true;
  // Then, loop over the wires array
  for(int i=0; i<numWires; i++){
    // Every wire that has a number > 0 in the wiresToCut array should be cut (at some point), after which they will read HIGH.
    // So if any of them still read LOW, that means that there is at least one wire still to be cut
    if(wiresToCut[i] !=0 && lastState[i] == LOW) {
      allWiresCut = false;
    }
  }

  // What to do next depends on the current state of the device
  if(state == State::Active) {
    // Retrieve the current timestamp
    unsigned long currentTime = millis();
    if(currentTime > detonationTime) {
      Detonate();
    }
    else if(allWiresCut == true) {
      Deactivate();
    }
    else {
      // Update clock display
      unsigned long timeRemaining = detonationTime - currentTime;
      int seconds = (timeRemaining / 1000)%60;
      int minutes = timeRemaining / 60000;
      // Update the seconds in the last two places
      display.showNumberDecEx(seconds, 0, true, 2, 2);
      // Display the minutes in the first two places, with colon
      display.showNumberDecEx(minutes, 0b01000000 , true, 2, 0);
    }
  }
}
