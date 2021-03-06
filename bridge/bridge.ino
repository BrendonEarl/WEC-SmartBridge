#include <Servo.h>

// ###################################
//                PINS
// ###################################
#define LIGHTS 1      // light shift registers
#define CS0 1         // shift register 0 chip select
#define CS1 1         // shift register 1 chip select

#define PROXTRIG 4    // proximity trigger pin
#define PROXN 2       // north prox sensor
#define PROXS 0       // south prox sensor
#define MOTPWM 9      // motor speed
#define MOTCONT 0     // motor control
#define TOP 13         // top limit switch
#define BOT 12         // bottom limit switch
#define BUZZ 3        // buzzer output

#define CLK 10        // shift register clk

#define QUAKEBTN 11   // quake user button
#define CRASHBTN 12   // crash user button
#define ERRBTN 13     // error test button

// ###################################
//             CONSTANTS
// ###################################
#define PULSETIME 3   // time for pulse to return
#define TEST 0        // run test routine

// ###################################
//            TYPEDEFS
// ###################################
typedef enum lightState {
  RED,
  FLASH,
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
  MotorState motorDir;
  int motorSpeed;
  boolean prox;
  unsigned long proxNStart;
  unsigned long proxSStart;
  unsigned long lastPulse;
  bool buzz;
  Lights lights;
} State;

// ###################################
//             GLOBALS
// ###################################
State state;
Servo pully;

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
  delay(10000);
  return true;
}

boolean isRaised() {
  if (digitalRead(TOP) == HIGH)
    return true;
  return false;
}

boolean shouldLower() {
  delay(5000);
  return true;
}

boolean safeToLower() {
  delay(5000);
  return true;
}

boolean isLowered() {
  if (digitalRead(BOT) == HIGH)
    return true;
  return false;
}

void proxPulse() {
  if (millis() - state.lastPulse > 500) {
    state.lastPulse = millis();
    // Pulse proximity sensor
    digitalWrite(PROXTRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PROXTRIG, HIGH);
    delayMicroseconds(5);
    digitalWrite(PROXTRIG, LOW);
    state.proxNStart = millis();
  
    digitalWrite(PROXS, LOW);
    delayMicroseconds(2);
    digitalWrite(PROXS, HIGH);
    delayMicroseconds(5);
    digitalWrite(PROXS, LOW);
    state.proxSStart = millis();
  }
}

void moveMotor() {
  int motorPWM = 2;
  if (state.motorDir == UP)
    motorPWM = map(state.motorSpeed, 0, 100, 90, 0);
  if (state.motorDir == DOWN)
    motorPWM = map(state.motorSpeed, 0, 100, 90, 179);
  pully.write(motorPWM);
}

void setBuzz() {
  Serial.println(state.mode);
  Serial.println(state.buzz);
  if (state.buzz == true) {
    if (state.mode == QUAKE)
      analogWrite(BUZZ, 10);
    if (state.mode == ERR)
      analogWrite(BUZZ, 200);
  } else {
    analogWrite(BUZZ, 0);
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
  pinMode(MOTPWM, OUTPUT);
  pinMode(TOP, INPUT);
  pinMode(BUZZ, OUTPUT);
  pinMode(QUAKEBTN, INPUT);
  pinMode(CRASHBTN, INPUT);
  pinMode(ERRBTN, INPUT);

  pully.attach(MOTPWM);

  attachInterrupt(digitalPinToInterrupt(PROXN), proxEchoN, FALLING);
  //attachInterrupt(digitalPinToInterrupt(PROXS), proxEchoS, FALLING);

  state.mode = OK;
  state.motorDir = DOWN;
  state.motorSpeed = 0;
  state.prox = false;
  state.proxNStart = 0;
  state.proxSStart = 0;

#ifdef TEST
  state.mode = ERR;
#endif
}

// ###################################
//           STATE MANAGEMENT
// ###################################
void loop() {
  switch (state.mode) {
    case ERR:
      state.motorDir = DOWN;
      state.motorSpeed = 0;
      state.prox = false;
      state.proxNStart = 0;
      state.proxSStart = 0;
      state.buzz = true;
      setBuzz();
      moveMotor();
      break;
    case OK:
      state.motorDir = DOWN;
      state.motorSpeed = 0;
      state.buzz = false;
      moveMotor();
      setBuzz();
      proxPulse();
//      if (shouldRaise()) state.mode = LIFTWAIT;
      break;
    case LIFTWAIT:
      state.motorDir = UP;
      state.motorSpeed = 0;
      if (safeToRaise()) state.mode = LIFTING;
      break;
    case LIFTING:
    state.motorDir = UP;
      state.motorSpeed = 100;
      moveMotor();
//      if (isRaised()) state.mode = LIFTED;
      break;
    case LIFTED:
      state.motorDir = UP;
      state.motorSpeed = 0;
      moveMotor();
      if (shouldLower() && safeToLower()) state.mode = LOWERING;
      break;
    case LOWERING:
      state.motorDir = DOWN;
      state.motorSpeed = 50;
      moveMotor();
      if (isLowered()) state.mode = OK;
      break;
    case QUAKE:
      state.motorDir = DOWN;
      state.motorSpeed = 0;
      state.prox = false;
      state.proxNStart = 0;
      state.proxSStart = 0;
      state.buzz = true;
      setBuzz();
      break;
    case CRASH:
      state.motorDir = DOWN;
      state.motorSpeed = 0;
      state.prox = false;
      state.proxNStart = 0;
      state.proxSStart = 0;
      break;
    default:
      Serial.println("Fallen into default state, setting to ERROR");
      state.mode = ERR;
      break;
  }

  

  // check for button pushes
//  if (digitalRead(QUAKEBTN) == HIGH) state.mode = QUAKE;
//  if (digitalRead(CRASHBTN) == HIGH) state.mode = CRASH;
//  if (digitalRead(ERRBTN) == HIGH) state.mode = ERR;
}
