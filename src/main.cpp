#include <Arduino.h>
#include <display.h>

#define BUTTON_PRESS 2
#define SHOCK_PIN 3

#include <Bounce2.h>

// INSTANTIATE A Bounce OBJECT.
Bounce turnChange = Bounce();

bool currentTurnWhite = true;

unsigned long MAX_TIME = 300000;

unsigned long whiteTimeMILLS = MAX_TIME;
unsigned long blackTimeMILLS = MAX_TIME;

unsigned long whiteLastStart = 0;
unsigned long blackLastStart = 0;

unsigned long lastShockTime = 0;
unsigned long ShockTimeSlotLength = 500;
unsigned long shockTimeEnd = 0;


// Runs every 500ms, to decide how long to enable the shock
int calculateLength(unsigned long timeMilli) {
  float percentProgress = 1-  ((float)timeMilli / (float)MAX_TIME);
  int shockLength = (int)(percentProgress * ShockTimeSlotLength);

  int normShockLength = max(min(shockLength, (ShockTimeSlotLength*0.5)), 50);

  float randomChance = random(0,100);
  Serial.print(randomChance);
  Serial.print(" ");
  Serial.println(percentProgress*100);

  if (randomChance < percentProgress*100) {
    return normShockLength;
  } else {
    return 0;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  displaySetup();
  turnChange.attach(BUTTON_PRESS, INPUT_PULLUP);
  turnChange.interval(15);  // interval in ms

  pinMode(SHOCK_PIN, OUTPUT);

  whiteLastStart = millis();
  blackLastStart = millis();
  lastShockTime = millis();

  Serial.println(whiteTimeMILLS);

  randomSeed(analogRead(0));
}
void endGame() {
  Serial.println("Game Over");
  digitalWrite(SHOCK_PIN, LOW);
  while (true) {
    if (currentTurnWhite) {
      printText(0, 3, "B WON");
    } else {
      printText(0, 3, "W WON");
    }
  }
}

void loop() {
  turnChange.update();
  if (turnChange.fell()) {
    currentTurnWhite = !currentTurnWhite;
    if (currentTurnWhite) {
      whiteLastStart = millis();
    } else {
      blackLastStart = millis();
    }
  }

  char outputString[24];

  if (currentTurnWhite) {
    whiteTimeMILLS = whiteTimeMILLS - (millis() - whiteLastStart);
    whiteLastStart = millis();

  } else {
    blackTimeMILLS = blackTimeMILLS - (millis() - blackLastStart);
    blackLastStart = millis();

  }

  if (currentTurnWhite) {
    if (whiteTimeMILLS > 100) {
      sprintf(outputString, "%s %01d:%02d", "W", (int)((whiteTimeMILLS / 1000) / 60), (int)((whiteTimeMILLS / 1000) % 60));
    } else {
      endGame();
    }
    // printText(0, 3, "W");
  } else {
    if (whiteTimeMILLS > 100) {
      sprintf(outputString, "%s %01d:%02d", "B", (int)((blackTimeMILLS / 1000) / 60), (int)((blackTimeMILLS / 1000) % 60));
    } else {
      endGame();
    }
  }
  printText(0, 3, outputString);

  // Time to work out the shock
  if (millis()-lastShockTime > ShockTimeSlotLength) {
    lastShockTime = millis();
    unsigned long shockLength = calculateLength(currentTurnWhite ? whiteTimeMILLS : blackTimeMILLS);

    shockTimeEnd = lastShockTime + shockLength;
  } else {
    if (millis() >= shockTimeEnd) {
      digitalWrite(SHOCK_PIN, LOW);
    } else {
      digitalWrite(SHOCK_PIN, HIGH);
    }
  }
}