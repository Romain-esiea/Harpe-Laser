#include "TimerFreeTone.h"

bool debug = true;
#define DEBUG_PRINTLN(x) if(debug) Serial.println(x)
#define DEBUG_PRINT(x) if(debug) Serial.print(x)

uint8_t tonePin = 9;
uint8_t nbCords = 5;

uint8_t lightInPins[5] = {0, 1, 2, 3, 4};
uint8_t laserPins[5]   = {6, 5, 4, 3, 2};

int thresholds[5];
bool currentlyReadCords[5] = {false};
bool cordeActiveSon[5]     = {false};

unsigned long cordePressStart[5] = {0};
unsigned long cordeStartTime[5]  = {0};
unsigned long cordeEndTime[5]    = {0};

const unsigned long MIN_DURATION = 500;
const unsigned long MAX_DURATION = 2000;

// Fréquences manuelles car on n'inclut plus pitches.h
int notes[5] = {523, 587, 659, 698, 784}; // C5 D5 E5 F5 G5
//int notes[5] = {440, 494, 523, 587, 659};

uint8_t nbCalibration = 30;
int maxOffsets[5] = {33, 90, 55, 120, 200};

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

  DEBUG_PRINTLN("Début du test des sons...");
  for (uint8_t idx = 0; idx < nbCords; idx++)
  {
    DEBUG_PRINT("Test corde "); DEBUG_PRINTLN(idx + 1);
    // TimerFreeTone(pin, fréquence, durée ms, volume 0-10)
    TimerFreeTone(tonePin, notes[idx], 300, 10);
    delay(500);
    DEBUG_PRINT("Fin test corde "); DEBUG_PRINTLN(idx + 1);
  }
  DEBUG_PRINTLN("Fin du test.");
}

void loop()
{
  unsigned long now = millis();

  // Détection des cordes
  for (uint8_t idx = 0; idx < nbCords; idx++)
  {
    int reading = analogRead(lightInPins[idx]);

    if (reading > thresholds[idx] && !currentlyReadCords[idx])
    {
      currentlyReadCords[idx] = true;
      cordePressStart[idx] = now;
    }
    else if (reading <= thresholds[idx] && currentlyReadCords[idx])
    {
      currentlyReadCords[idx] = false;
      unsigned long pressDuration = now - cordePressStart[idx];
      unsigned long playTime = constrain(pressDuration, MIN_DURATION, MAX_DURATION);

      cordeActiveSon[idx] = true;
      cordeStartTime[idx] = now;
      cordeEndTime[idx]   = now + playTime;

      DEBUG_PRINT("Corde "); DEBUG_PRINT(idx + 1);
      DEBUG_PRINT(" declenchee, duree="); DEBUG_PRINTLN(playTime);
    }
  }

  // Expiration des cordes
  for (uint8_t idx = 0; idx < nbCords; idx++)
  {
    if (cordeActiveSon[idx] && now >= cordeEndTime[idx])
    {
      cordeActiveSon[idx] = false;
      DEBUG_PRINT("Fin corde "); DEBUG_PRINTLN(idx + 1);
    }
  }

  // Calcul fréquence moyenne + ratio de fondu moyen
  uint8_t activeCount = 0;
  long sumNotes = 0;
  float sumRatio = 0.0;

  for (uint8_t idx = 0; idx < nbCords; idx++)
  {
    if (cordeActiveSon[idx])
    {
      activeCount++;
      sumNotes += notes[idx];

      unsigned long elapsed = now - cordeStartTime[idx];
      unsigned long total   = cordeEndTime[idx] - cordeStartTime[idx];
      float ratio = 1.0 - (float)elapsed / (float)total;
      sumRatio += constrain(ratio, 0.0, 1.0);
    }
  }

  // Application du son avec fondu via TimerFreeTone
  if (activeCount == 0)
  {
    delay(20);
  }
  else
  {
    int targetFrequency = (int)(sumNotes / activeCount);
    float avgRatio = sumRatio / activeCount;

    // Volume de 1 à 10 (échelle TimerFreeTone)
    int volume = (int)(10.0 * avgRatio);
    volume = constrain(volume, 1, 10);

    // On joue 20ms à ce volume et cette fréquence
    TimerFreeTone(tonePin, targetFrequency, 20, volume);
  }
}
