#define stdio.h
#define stdlib.h

const int PROXN = 1;    // north prox sensor
const int PROXS = 2;    // south prox sensor
const int MOTN = 3;     // north motor sensor
const int MOTS = 4;     // south motor sensor
const int MOTCONT = 5;  // motor control

typedef enum mode {
  ERROR,
  OK,
  LIFTWAIT,
  LIFTING,
  LIFTED,
  LOWERING,
  QUAKE
} Mode;

typedef struct State {
  Mode mode;

};

State state;

boolean shouldRaise() {
  return true;
}

boolean safeToRaise() {
  return true;
}

boolean isRaised() {
  return true;
}

boolean shouldLower() {
  return true;
}

boolean safeToLower() {
  return true;
}

boolean isLowered() {
  return true;
}

void setup() {
  state.mode = OK;
}

void loop() {
  switch (state.mode) {
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
    case ERROR:
      break;
    default:
      Serial.println("Fallen into default state, setting to ERROR");
      state.mode = ERROR;
      break;

    // check for button pushes
  }
}
