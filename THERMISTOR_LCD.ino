#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);   // change to 0x3F if needed

// Cannula dislodge detector using thermistor (1% threshold, latched alert + recalibration + power LED)

const int analogPin = A0;
const int alarmPin  = 8;
const int dislodgedLedPin = 11;
const int calLedPin = 12;
const int powerLed  = 13;

// Calibration
const unsigned long CAL_DURATION_MS = 5000;
const unsigned long SAMPLE_INTERVAL_MS = 50;

// Detection
const float DROP_FRACTION = 0.995;
const unsigned long LOOP_INTERVAL_MS = 500;
float baseline = 0.0;

// Button
const int buttonPin = 7;
const unsigned long DEBOUNCE_MS = 50;
int lastButtonReading = HIGH;
int buttonState = HIGH;
unsigned long lastDebounceTime = 0;
bool calibrated = false;

// Dislodgement logic
const int DISLODGED_REQUIRED = 10;
int dislodgedCount = 0;
bool alarmLatched = false;

void setup() {
  Serial.begin(9600);

  // LCD setup
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Cannula Monitor");
  delay(1000);
  lcd.clear();

  pinMode(alarmPin, OUTPUT);
  pinMode(dislodgedLedPin, OUTPUT);
  pinMode(calLedPin, OUTPUT);
  pinMode(powerLed, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  digitalWrite(alarmPin, LOW);
  digitalWrite(dislodgedLedPin, LOW);
  digitalWrite(calLedPin, LOW);
  digitalWrite(powerLed, HIGH);

  Serial.println("\n=== Thermistor Cannula Monitor ===");
  Serial.println("Power LED ON: System powered.");
  Serial.println("Press the button (A7/D7) to calibrate for 5 seconds...");
  Serial.println("If dislodged, alarm latches until recalibration.");
  }

  void loop() {
  // --- Button Handling ---
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonReading) {
  lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
  if (reading != buttonState) {
  buttonState = reading;

    if (buttonState == LOW) {  // button pressed
      Serial.println("\\nButton pressed: stopping alarm + recalibrating...");
      stopAlarmOutput();
      alarmLatched = false;
      dislodgedReset();

      calibrated = false;
      Serial.print("Calibration period (ms): ");
      Serial.println(CAL_DURATION_MS);
      calibrateBaseline();
      calibrated = true;

      Serial.println("Calibration complete. Monitoring restarted.\\n");
    }
  }

  }
  lastButtonReading = reading;

  if (!calibrated) return;

  // --- Monitoring Loop ---
  static unsigned long lastLoop = 0;
  unsigned long now = millis();
  if (now - lastLoop < LOOP_INTERVAL_MS) return;
  lastLoop = now;

  int raw = analogRead(analogPin);
  float lowerLimit = baseline * DROP_FRACTION;

  // Serial Output
  Serial.print("Reading: ");
  Serial.print(raw);
  Serial.print("  (count=");
  Serial.print(dislodgedCount);
  Serial.print("/");
  Serial.print(DISLODGED_REQUIRED);
  Serial.print(")");

  // --- Dislodgement Logic ---
  if (!alarmLatched) {
  if (raw < lowerLimit) {
  if (dislodgedCount < DISLODGED_REQUIRED) dislodgedCount++;
  } else {
  dislodgedCount = 0;
  }

if (dislodgedCount >= DISLODGED_REQUIRED) {
  alarmLatched = true;
  digitalWrite(alarmPin, HIGH);
  digitalWrite(dislodgedLedPin, HIGH);
  Serial.println("  --> Cannula dislodged! (Alarm latched)");
} else {
  Serial.println("  --> Cannula normal.");
}

} else {
digitalWrite(alarmPin, HIGH);
digitalWrite(dislodgedLedPin, HIGH);
Serial.println("  --> Alarm latched: press button to recalibrate.");
}

// --- LCD DISPLAY (RAW THERMISTOR VALUE + DISLODGED ALERT) ---
lcd.clear();

// First line: Thermistor value
lcd.setCursor(0, 0);
lcd.print("Thermistor: ");
lcd.print(raw);

// Second line: status
lcd.setCursor(0, 1);

if (alarmLatched) {
  lcd.print("DISLODGED!");
} else {
  lcd.print("Count ");
  lcd.print(dislodgedCount);
  lcd.print("/");
  lcd.print(DISLODGED_REQUIRED);
}

}

void dislodgedReset() {
Serial.println("Resetting counters...");
stopAlarmOutput();
digitalWrite(dislodgedLedPin, LOW);
dislodgedCount = 0;
}

void stopAlarmOutput() {
#if defined(ARDUINO_ARCH_AVR)
noTone(alarmPin);
#endif
digitalWrite(alarmPin, LOW);
}

void calibrateBaseline() {
  unsigned long start = millis();
  unsigned long nextSample = start;
  unsigned long samples = 0;
  unsigned long sum = 0;

  Serial.println("Calibrating baseline for 5 seconds...");

  // LCD message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibrating...");
  lcd.setCursor(0, 1);
  lcd.print("Please wait");

  const unsigned long CAL_FLASH_MS = 250;
  unsigned long lastFlash = millis();
  bool calLedState = false;

  digitalWrite(calLedPin, LOW);

  while (millis() - start < CAL_DURATION_MS) {
    unsigned long now = millis();

    if (now >= nextSample) {
      sum += analogRead(analogPin);
      samples++;
      nextSample += SAMPLE_INTERVAL_MS;
      if (samples % 10 == 0) Serial.print(".");
    }

    if (now - lastFlash >= CAL_FLASH_MS) {
      calLedState = !calLedState;
      digitalWrite(calLedPin, calLedState ? HIGH : LOW);
      lastFlash = now;
    }

    delay(1);
  }

  digitalWrite(calLedPin, LOW);

  if (samples == 0) samples = 1;
  baseline = (float)sum / (float)samples;

  Serial.println();
  Serial.print("Calibrated baseline: ");
  Serial.println(baseline);
  Serial.print("1% threshold: ");
  Serial.println(baseline * DROP_FRACTION);
}
