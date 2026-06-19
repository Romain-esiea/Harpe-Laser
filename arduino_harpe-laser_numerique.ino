#include "pitches.h"

bool debug = true;
#define DEBUG_PRINTLN(x) if(debug) Serial.println(x)
#define DEBUG_PRINT(x) if(debug) Serial.print(x)

uint8_t tonePin = 8;
uint8_t nbCords = 5;

uint8_t lightInPins[5] = {0, 1, 2, 3, 4};
uint8_t laserPins[5]   = {6, 5, 4, 3, 2};

int thresholds[5];
bool currentlyReadCords[5] = {false};
bool cordeActiveSon[5]     = {false};

// Timing
unsigned long cordePressStart[5] = {0};
unsigned long cordeStartTime[5]  = {0};  // quand le son a commencé
unsigned long cordeEndTime[5]    = {0};  // quand le son doit s'arrêter

const unsigned long FADE_TIME    = 500;   // fondu visuel/logique (ms)
const unsigned long MIN_DURATION = 300;  // durée minimum du son (ms)
const unsigned long MAX_DURATION = 2000;  // durée maximum du son (ms)

int notes[5] = {NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5};

uint8_t nbCalibration = 30;
int maxOffsets[5] = {33, 90, 55, 120, 200};

int currentFrequency = -1;  // fréquence actuellement jouée

void calculateThreshold(uint8_t idx)
{
  long ambiant = 0;
  for (uint8_t i = 0; i < nbCalibration; i++)
  {
    ambiant += analogRead(lightInPins[idx]);
    delay(5);
  }
  ambiant /= nbCalibration;
  thresholds[idx] = ambiant + maxOffsets[idx];

  DEBUG_PRINT("Corde "); DEBUG_PRINT(idx);
  DEBUG_PRINT(" | ambiant="); DEBUG_PRINT(ambiant);
  DEBUG_PRINT(" | threshold="); DEBUG_PRINTLN(thresholds[idx]);

}

void calibrate()
{
  for (uint8_t i = 0; i < nbCords; i++)
    digitalWrite(laserPins[i], HIGH);

  delay(200);
  for (uint8_t i = 0; i < nbCords; i++)
    calculateThreshold(i);
}

void setup()
{
  Serial.begin(9600);

  for (uint8_t i = 0; i < nbCords; i++)
    pinMode(laserPins[i], OUTPUT);

  pinMode(tonePin, OUTPUT);

  calibrate();
  DEBUG_PRINTLN("Pret !");

  // Test des sons au démarrage
  DEBUG_PRINTLN("Début du test des sons pour chaque corde...");
  for (uint8_t idx = 0; idx < nbCords; idx++)
  {
    DEBUG_PRINT("Test son pour corde ");
    DEBUG_PRINT(idx + 1);
    DEBUG_PRINTLN(" (appuie simulé)...");
    tone(tonePin, notes[idx], 300);  // Joue la note pendant 300 ms
    delay(500);  // Pause entre les tests
    noTone(tonePin);
    DEBUG_PRINT("Fin du test pour corde ");
    DEBUG_PRINTLN(idx + 1);
  }
  DEBUG_PRINTLN("Fin du test des sons.");
}

void loop()
{
  unsigned long now = millis();
  delay(25);

  for (uint8_t idx = 0; idx < nbCords; idx++)
  {
    int reading = analogRead(lightInPins[idx]);

    if (reading > thresholds[idx] && !currentlyReadCords[idx])
    {
      currentlyReadCords[idx] = true;
      cordeActiveSon[idx] = false;
      cordePressStart[idx] = now; 
    }
    else if (reading <= thresholds[idx] && currentlyReadCords[idx])
    {
      currentlyReadCords[idx] = false;
      unsigned long pressDuration = now - cordePressStart[idx];
      unsigned long playTime = constrain(pressDuration, MIN_DURATION, MAX_DURATION);

      cordeActiveSon[idx] = true;
      cordeStartTime[idx] = now;
      cordeEndTime[idx] = now + playTime;
    }
  }

  // Gestion de la fin de son (respect MIN/MAX)
  for (uint8_t idx = 0; idx < nbCords; idx++)
  {
    if (cordeActiveSon[idx] && now >= cordeEndTime[idx])
    {
      cordeActiveSon[idx] = false;
      DEBUG_PRINT("Fin du son pour corde ");
      DEBUG_PRINTLN(idx + 1);
    }
  }

  // Calcul de la fréquence cible
  uint8_t activeCount = 0;
  long sumNotes = 0;

  for (uint8_t idx = 0; idx < nbCords; idx++)
  {
    if (cordeActiveSon[idx])
    {
      activeCount++;
      sumNotes += notes[idx];
    }
  }

  int targetFrequency = -1;

  if (activeCount == 1)
  {
    for (uint8_t idx = 0; idx < nbCords; idx++)
    {
      if (cordeActiveSon[idx])
      {
        targetFrequency = notes[idx];
        break;
      }
    }
  }
  else if (activeCount > 1)
  {
    targetFrequency = sumNotes / activeCount;  // moyenne simple
  }

  // Application du son sans spam
  if (targetFrequency != currentFrequency)
  {
    currentFrequency = targetFrequency;

    if (currentFrequency < 0)
      noTone(tonePin);
    else
      tone(tonePin, currentFrequency);
  }
}
