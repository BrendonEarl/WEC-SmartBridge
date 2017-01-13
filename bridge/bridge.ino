// ###################################
//                PINS
// ###################################
#define LIGHTS 0      // light shift registers
#define CS0 1         // shift register 0 chip select
#define CS1 0         // shift register 1 chip select

#define PROXTRIG 4    // proximity trigger pin
#define PROXN 2       // north prox sensor
#define PROXS 0       // south prox sensor
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
#define PULSETIME 5    // time for pulse to return
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
  unsigned long proxNStart;
  unsigned long proxSStart;
} State;

State state;

// ###################################
//           INTERUPT F'NS
// ###################################
void proxEchoN() {
  if (state.proxNStart != 0) {
    Serial.print("ProxN time: ");
    Serial.println(millis() - state.proxNStart);
    Serial.print(millis());
    Serial.print(" ");
    Serial.println(state.proxNStart);
    if (millis() - state.proxNStart < PULSETIME) {
      state.prox = true;
      state.proxNStart = 0;
      state.proxSStart = 0;
    } else {
      state.prox = false;
      state.proxNStart = 0;
    }
  }
}

void proxEchoS() {
  if (state.proxSStart != 0) {
    Serial.print("ProxS time: ");
    Serial.println(millis() - state.proxSStart);
    if (millis() - state.proxSStart < PULSETIME) {
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
    digitalWrite(PROXTRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PROXTRIG, HIGH);
    delayMicroseconds(5);
    digitalWrite(PROXTRIG, LOW);
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

void moveMotor() {
  if (state.motorDir == UP) {
    
  }
  if (state.motorDir == DOWN) {
    
  }
}


// ###################################
//               SETUP
// ###################################
void setup() {
  Serial.begin(9600);
  Serial.println("Serial initialized");

  pinMode(LIGHTS, OUTPUT);
  pinMode(CS0, OUTPUT);
  pinMode(CS1, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(PROXTRIG, OUTPUT);
  pinMode(PROXN, INPUT);
  pinMode(PROXS, INPUT);
  pinMode(MOTCONT, OUTPUT);
  pinMode(TOP, INPUT);
  pinMode(BUZZ, OUTPUT);
  pinMode(QUAKEBTN, INPUT);
  pinMode(CRASHBTN, INPUT);
  pinMode(ERRBTN, INPUT);

  attachInterrupt(digitalPinToInterrupt(PROXN), proxEchoN, FALLING);
//  attachInterrupt(digitalPinToInterrupt(PROXS), proxEchoS, FALLING);
  
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
  state.mode = LIFTING;
#endif
#ifndef TEST
  switch (state.mode) {
    case ERR:
      state.motordir = DOWN;
      state.motorSpeed = 0;
      state.prox = false;
      state.proxNStart = 0;
      state.proxSStart = 0;
      break;
    case OK:
      state.motordir = DOWN;
      state.motorSpeed = 0;
      checkProx();
      if (shouldRaise()) state.mode = LIFTWAIT;
      break;
    case LIFTWAIT:
      state.motordir = UP;
      state.motorSpeed = 0;
      if (safeToRaise()) state.mode = LIFTING;
      break;
    case LIFTING:
      state.motordir = UP;
      state.motorSpeed = 50;
      if (isRaised()) state.mode = LIFTED;
      break;
    case LIFTED:
      state.motordir = UP;
      state.motorSpeed = 0;
      if (shouldLower() && safeToLower()) state.mode = LOWERING;
      break;
    case LOWERING:
      state.motordir = DOWN;
      state.motorSpeed = 50;
      if (isLowered()) state.mode = OK;
      break;
    case QUAKE:
      state.motordir = DOWN;
      state.motorSpeed = 0;
      state.prox = false;
      state.proxNStart = 0;
      state.proxSStart = 0;
      break;
    case CRASH:
      state.motordir = DOWN;
      state.motorSpeed = 0;
      state.prox = false;
      state.proxNStart = 0;
      state.proxSStart = 0;
      break;
    default:
      Serial.println("Fallen into default state, setting to ERROR");
      state.mode = ERR;
      break;

    moveMotor();

    // check for button pushes
    if (digitalRead(QUAKEBTN) == HIGH) state.mode = QUAKE;
    if (digitalRead(CRASHBTN) == HIGH) state.mode = CRASH;
    if (digitalRead(ERRBTN) == HIGH) state.mode = ERR;
  }
#endif
}
