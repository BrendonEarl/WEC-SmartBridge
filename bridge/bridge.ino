#include <Time.h>

// ###################################
//            CONSTANTS
// ###################################
#define LIGHTS 0      // light shift registers
#define CS0 1         // shift register 0 chip select
#define CS1 0         // shift register 1 chip select
#define CS2 0         // shift register 2 chip select

#define PROXTRIG 4    // proximity trigger pin
#define PROXN 2       // north prox sensor
#define PROXS 3       // south prox sensor
#define MOTCONT 6     // motor control
#define TOP 7         // top limit switch
#define BOT 8         // bottom limit switch
#define BUZZ 9        // buzzer input

#define CLK 10        // shift register clk

#define QUAKEBTN 11   // quake user button
#define CRASHBTN 12   // crash user button
#define ERRBTN 13     // error test button


// ###################################
//            TYPEDEFS
// ###################################
typedef enum {
  RED,
  ORANGE,
  GREEN
} LightState;

typedef enum mode {
  ERR,
  OK,
  LIFTWAIT,
  LIFTING,
  LIFTED,
  LOWERING,
  QUAKE,
  CRASH
} Mode;

typedef struct {
  Mode mode;
  boolean prox;
  int proxNStart;
  int proxSStart;
} State;

State state;

// ###################################
//           INTERUPT F'NS
// ###################################
void proxEchoN() {
  
}

void proxEchoS() {
  
}


// ###################################
//            HELPER F'NS
// ###################################
boolean shouldRaise() {
  if (digitalRead(PROXS) == HIGH || digitalRead(PROXN) == HIGH)
    return true;
  return false;
}

boolean safeToRaise() {
  return true;
}

boolean isRaised() {
  if (digitalRead(TOP) == HIGH)
    return true;
  return false;
}

boolean shouldLower() {
  return true;
}

boolean safeToLower() {
  return true;
}

boolean isLowered() {
  if (digitalRead(BOT) == HIGH)
    return true;
  return false;
}

void proxPulse(char dir) {
  // Pulse proximity sensor
  if (dir == 'N') {
    digitalWrite(PROXN, LOW);
    delayMicroseconds(2);
    digitalWrite(PROXN, HIGH);
    delayMicroseconds(5);
    digitalWrite(PROXN, LOW);
    state.proxNStart = now();
  }
  if (dir == 'S') {
    digitalWrite(PROXS, LOW);
    delayMicroseconds(2);
    digitalWrite(PROXS, HIGH);
    delayMicroseconds(5);
    digitalWrite(PROXS, LOW);
    state.proxSStart = now();
  }
}


// ###################################
//               SETUP
// ###################################
void setup() {
  pinMode(LIGHTS, OUTPUT);
  pinMode(CS0, OUTPUT);
  pinMode(CS1, OUTPUT);
  pinMode(CS2, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(PROXN, INPUT_PULLUP);
  pinMode(PROXS, INPUT_PULLUP);
  pinMode(MOTCONT, OUTPUT);
  pinMode(TOP, INPUT);
  pinMode(BUZZ, OUTPUT);
  pinMode(QUAKEBTN, INPUT);
  pinMode(CRASHBTN, INPUT);
  pinMode(ERRBTN, INPUT);

  attachInterrupt(digitalPinToInterrupt(PROXN), proxEchoN, RISING);
  attachInterrupt(digitalPinToInterrupt(PROXS), proxEchoS, RISING);
  
  state.mode = OK;
}

// ###################################
//           STATE MANAGEMENT
// ###################################
void loop() {
  switch (state.mode) {
    case ERR:
      break;
    case OK:
      if (shouldRaise()) state.mode = LIFTWAIT;
      break;
    case LIFTWAIT:
      if (safeToRaise()) state.mode = LIFTING;
      break;
    case LIFTING:
      if (isRaised()) state.mode = LIFTED;
      break;
    case LIFTED:
      if (shouldLower() && safeToLower()) state.mode = LOWERING;
      break;
    case LOWERING:
      if (isLowered()) state.mode = OK;
      break;
    case QUAKE:
      break;
    case CRASH:
      break;
    default:
      Serial.println("Fallen into default state, setting to ERROR");
      state.mode = ERR;
      break;

    // check for button pushes
    if (digitalRead(QUAKEBTN) == HIGH) state.mode = QUAKE;
    if (digitalRead(CRASHBTN) == HIGH) state.mode = CRASH;
    if (digitalRead(ERRBTN) == HIGH) state.mode = ERR;
  }
}
