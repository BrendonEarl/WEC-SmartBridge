// ###################################
//                PINS
// ###################################
#define LIGHTS 0      // light shift registers
#define CS0 1         // shift register 0 chip select
#define CS1 0         // shift register 1 chip select
#define CS2 0         // shift register 2 chip select

#define PROXTRIG 4    // proximity trigger pin
#define PROXN 2       // north prox sensor
#define PROXS 3       // south prox sensor
#define MOTPWM 5      // motor speed
#define MOTCONT 6     // motor control
#define TOP 7         // top limit switch
#define BOT 8         // bottom limit switch
#define BUZZ 9        // buzzer input

#define CLK 10        // shift register clk

#define QUAKEBTN 11   // quake user button
#define CRASHBTN 12   // crash user button
#define ERRBTN 13     // error test button

// ###################################
//             CONSTANTS
// ###################################
#define PULSETIME 50    // time for pulse to return
#define TEST 1          // run test routine


// ###################################
//            TYPEDEFS
// ###################################
typedef enum lightState {
  RED,
  ORANGE,
  GREEN
} LightState;

typedef struct {
  LightState N0;
  LightState N1;
  LightState N2;
  LightState N3;
  LightState N4;
  LightState N5;
  LightState S0;
  LightState S1;
  LightState S2;
  LightState S3;
  LightState S4;
  LightState S5;
} Lights;

Lights lights;

typedef enum {
  DOWN,
  UP
} MotorState;

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
  MotorState motordir;
  int motorSpeed;
  boolean prox;
  int proxNStart;
  int proxSStart;
} State;

State state;

// ###################################
//           INTERUPT F'NS
// ###################################
void proxEchoN() {
  Serial.print("ProxN time: ");
  if (state.proxNStart != 0)
    if (state.proxNStart - millis() < PULSETIME) {
      state.prox = true;
      state.proxNStart = 0;
      state.proxSStart = 0;
    } else {
      state.prox = false;
      state.proxNStart = 0;
    }
}

void proxEchoS() {
  Serial.print("ProxS time: ");
  Serial.println(state.proxSStart - millis());
  if (state.proxSStart != 0) {
    if (state.proxSStart - millis() < PULSETIME) {
      state.prox = true;
      state.proxNStart = 0;
      state.proxSStart = 0;
    } else {
      state.prox = false;
      state.proxSStart = 0;
    }
  }
}


// ###################################
//            HELPER F'NS
// ###################################
boolean shouldRaise() {
  if (state.prox == true)
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
    state.proxNStart = millis();
  }
  if (dir == 'S') {
    digitalWrite(PROXS, LOW);
    delayMicroseconds(2);
    digitalWrite(PROXS, HIGH);
    delayMicroseconds(5);
    digitalWrite(PROXS, LOW);
    state.proxSStart = millis();
  }
}


// ###################################
//               SETUP
// ###################################
void setup() {
  Serial.begin(9600);

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
  state.motordir = DOWN;
  state.motorSpeed = 0;
  state.prox = false;
  state.proxNStart = 0;
  state.proxSStart = 0;
}

// ###################################
//           STATE MANAGEMENT
// ###################################
void loop() {
#ifdef TEST
  state.mode = OK;
  shouldRaise();
#endif
#ifndef TEST
  switch (state.mode) {
    case ERR:
      break;
    case OK:
      checkProx();
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
#endif
}
