#include <Arduino.h>
#include <Bounce2.h>

#include <display.h>

#define BUTTON_PRESS 2
#define SHOCK_PIN 3
#define MENU_BUTTON 5
#define TEST_SHOCK 4

Bounce turnChange = Bounce();
Bounce menuButton = Bounce();

bool currentTurnWhite = true;

long MAX_TIME = 300000;

long whiteTimeMILLS = MAX_TIME;
long blackTimeMILLS = MAX_TIME;

unsigned long whiteLastStart = 0;
unsigned long blackLastStart = 0;

unsigned long lastShockTime = 0;
unsigned long MaxShockTimeSlotLength = 1000;
unsigned long CurrentTimeslot = 500;
unsigned long shockTimeEnd = 0;


// Runs every Xms, to decide how long to enable the shock
int calculateLength(long long timeMilli, unsigned long CurrentTimeslot) {
  float percentProgress = 1-  ((float)timeMilli / (float)MAX_TIME);
  int shockLength = (int)(percentProgress * CurrentTimeslot);

  int normShockLength = max(min(shockLength, (CurrentTimeslot*0.5)), 50);

  float randomChance = random(0,100);

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

  pinMode(TEST_SHOCK, INPUT_PULLUP);

  menuButton.attach(MENU_BUTTON, INPUT_PULLUP);
  menuButton.interval(15);  // interval in ms

  turnChange.attach(BUTTON_PRESS, INPUT_PULLUP);
  turnChange.interval(15);  // interval in ms

  pinMode(SHOCK_PIN, OUTPUT);
  digitalWrite(SHOCK_PIN, LOW);

  printText(0, 3, "5 MIN");

  turnChange.update();
  while (turnChange.read()) {
    // Serial.println("Waiting for button press");

    while(digitalRead(TEST_SHOCK) == LOW) {
      digitalWrite(SHOCK_PIN, HIGH);
    }
    digitalWrite(SHOCK_PIN, LOW);

    menuButton.update();
    if (menuButton.fell()) {
      // cycle thru times, 9 min 59, 5 min, 3 min, 1 min
      if (MAX_TIME == 599000) {
        MAX_TIME = 300000;
        printText(0, 3, "5 MIN");
      } else if (MAX_TIME == 300000) {
        MAX_TIME = 180000;
        printText(0, 3, "3 MIN");
      } else if (MAX_TIME == 180000) {
        MAX_TIME = 60000;
        printText(0, 3, "1 MIN");
      } else if (MAX_TIME == 60000) {
        MAX_TIME = 599000;
        printText(0, 3, "10 MIN");
      }
      Serial.println(MAX_TIME);
    }

    turnChange.update();
  }

  whiteTimeMILLS = MAX_TIME;
  blackTimeMILLS = MAX_TIME;
  

  whiteLastStart = millis();
  blackLastStart = millis();
  lastShockTime = millis();

  Serial.println(whiteTimeMILLS);

  // Initialise the random seed with an unused pin
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

    // Turn change button has been released
    currentTurnWhite = !currentTurnWhite;
    if (currentTurnWhite) {
      Serial.println("Turn change, now white's turn");
      whiteLastStart = millis();
    } else {
      Serial.println("Turn change, now Black's turn");
      blackLastStart = millis();
    }
  }

  if (currentTurnWhite) {
    whiteTimeMILLS = whiteTimeMILLS - (millis() - whiteLastStart);
    whiteLastStart = millis();
  } else {
    blackTimeMILLS = blackTimeMILLS - (millis() - blackLastStart);
    blackLastStart = millis();
  }

  if (whiteTimeMILLS < 0 || blackTimeMILLS < 0 || whiteTimeMILLS > 1500000 || blackTimeMILLS > 1500000) {
    // Out of time
    endGame();
  }

  char outputString[24];
  char formatString[] = "%s %01d:%02d";

  if (currentTurnWhite) {
      sprintf(outputString, formatString, "W", (int)((whiteTimeMILLS / 1000) / 60), (int)((whiteTimeMILLS / 1000) % 60));
  } else {
      sprintf(outputString, formatString, "B", (int)((blackTimeMILLS / 1000) / 60), (int)((blackTimeMILLS / 1000) % 60));
  }
  printText(0, 3, outputString);

  // Time to work out the shock time
  if (millis()-lastShockTime > CurrentTimeslot) {
    CurrentTimeslot = random(69, MaxShockTimeSlotLength);

    // Calculate a new shock timing
    lastShockTime = millis();
    long shockLength = calculateLength(currentTurnWhite ? whiteTimeMILLS : blackTimeMILLS, CurrentTimeslot);
    

    shockTimeEnd = lastShockTime + shockLength;
  } else {
    if (millis() >= shockTimeEnd) {
      digitalWrite(SHOCK_PIN, LOW);
    } else {
      digitalWrite(SHOCK_PIN, HIGH);
    }
  }
}