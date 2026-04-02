#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

int mq = A0;
int relay1 = 7;   // Heater → MQ135
int relay2 = 8;   // Timer → 1 minute

// Timer variables
unsigned long timerStart = 0;
bool timerStarted = false;
bool timerFinished = false;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // RTC setup
  if (!rtc.begin()) {
    Serial.println("RTC NOT FOUND");
    while (1);
  }

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);

  // Both relays OFF initially (Active LOW relay)
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);

  Serial.println("System Started");
}

// --------------------------
// HEATER CONTROL (Relay1)
// --------------------------
void handleHeater() {
  int air = analogRead(mq);
  Serial.print("MQ135: ");
  Serial.println(air);

  if (air > 400) {
    digitalWrite(relay1, HIGH);  // OFF
    Serial.println("Heater: OFF (C02 increasing)");
  } else {
    digitalWrite(relay1, LOW);   // ON
    Serial.println("Heater: ON (CO2 decreasing)");
  }
}

// --------------------------
// TIMER CONTROL (Relay2)
// --------------------------
void handleTimer() {

  // STEP 1 → Start timer once
  if (!timerStarted) {
    timerStarted = true;
    timerStart = millis();
    digitalWrite(relay2, LOW);     // Relay2 ON
    Serial.println("Timer Socket: ON (Started)");
  }

  // STEP 2 → After 1 minute, turn OFF once
  if (!timerFinished && (millis() - timerStart >= 60000)) {
    digitalWrite(relay2, HIGH);    // Relay OFF
    Serial.println("Timer Socket: OFF (1 min done)");
    timerFinished = true;          // Never run again
  }
}

void loop() {
  DateTime now = rtc.now();

  Serial.print("Time: ");
  Serial.print(now.hour()); Serial.print(":");
  Serial.print(now.minute()); Serial.print(":");
  Serial.println(now.second());

  handleHeater();   // Relay1 controlled by MQ135
  handleTimer();    // Relay2 1-minute timer

  Serial.println("-------------------------");
  delay(1000);
}
