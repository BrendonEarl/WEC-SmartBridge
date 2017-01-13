#define stdio.h
#define stdlib.h

const int PROXN = 1;    // north prox sensor
const int PROXS = 2;    // south prox sensor
const int MOTN = 3;     // north motor sensor
const int MOTS = 4;     // south motor sensor

typedef enum Bool {
  FALSE,
  TRUE
};

typedef enum mode {
  ERROR,
  OK,
  LIFTING,
  LIFTED,
  LOWERING,
  QUAKE
} Mode;

typedef struct State {
  Mode mode;

};

State state;

void setup() {
  state.mode = OK;
}

void loop() {
  switch (state.mode) {
    case OK:
      break;
    case LIFTING:
      break;
    case LIFTED:
      break;
    case LOWERING:
      break;
    case QUAKE:
      break;
    case ERROR:
      break;
    default:
      Serial.println("Fallen into default state, setting to ERROR");
      state.mode = ERROR;
      break;
  }
}
