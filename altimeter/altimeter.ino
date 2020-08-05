#include <Adafruit_BMP3XX.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_Sensor.h>
#include <Encoder.h>
#include <SPI.h>
#include <Wire.h>

#define MINUS_SIGN B01000000

Adafruit_BMP3XX bmp;
Adafruit_7segment matrix = Adafruit_7segment();

Encoder knob(2, 4);
long knobPosition = 0;

#define PIN_ENCODER_SWITCH 7
int buttonState;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

bool editingSettings = true;
int menuIndex = 0;
int brightness = 0;
float seaLevelPressureHpa = 1013.25;

const long EARTH_CORE_ALTITUDE_FEET = -20903520;
const float FEET_PER_METER = 3.28084;
long altitudeFeet = EARTH_CORE_ALTITUDE_FEET;
unsigned long lastAltitudeDebounceTime = 0;
unsigned long altitudeDebounceDelay = 1000;

void setup() {
//  Serial.begin(9600);

  pinMode(PIN_ENCODER_SWITCH, INPUT_PULLUP);
  digitalPinToInterrupt(2);
  digitalPinToInterrupt(4);

  if (!bmp.begin()) {
    // Could not find a valid BMP3 sensor, check wiring!
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    while (true);
  }

  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);

  matrix.begin(0x70);
  displayJosh();
  matrix.setBrightness(brightness);
}

void displayJosh() {
  matrix.setBrightness(0);
  matrix.writeDigitRaw(0, B00011110);
  matrix.writeDigitRaw(1, B00111111);
  matrix.writeDigitRaw(3, B01101101);
  matrix.writeDigitRaw(4, B01110110);
  matrix.writeDisplay();

  for (int i = 1; i <= 15; i++) {
    matrix.setBrightness(i);
    delay(50);
  }

  for (int i = 15; i >= 0; i--) {
    matrix.setBrightness(i);
    delay(50);
  }
}

void loop() {
  if (! bmp.performReading()) {
    matrix.printError();
    matrix.writeDisplay();
    return;
  }
  
  int knobDiff = readKnob();
  bool buttonPressed = debounceButton();

  if (editingSettings) {
    displaySettings(knobDiff, buttonPressed);
  } else {
    displayInformation(knobDiff, buttonPressed);
  }
}

int readKnob() {
  long newPosition = knob.read() / 4;
  int diff = newPosition - knobPosition;
  knobPosition = newPosition;
  return diff;
}

bool debounceButton() {
  bool buttonPressed = false;
  
  int reading = digitalRead(PIN_ENCODER_SWITCH);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay && reading != buttonState) {
    buttonState = reading;

    if (buttonState == LOW) {
      buttonPressed = true;
    }
  }

  lastButtonState = reading;
  return buttonPressed;
}

void displaySettings(int knobDiff, bool buttonPressed) {
  matrix.clear();
  matrix.blinkRate(HT16K33_BLINK_DISPLAYON);

  if (menuIndex == 0) {
    printBrightnessSetting(knobDiff);
  } else if (menuIndex == 1) {
    printPressureWholeNumberSetting(knobDiff);
  } else {
    printPressureDecimalNumberSetting(knobDiff);
  }
  
  matrix.writeDisplay();

  if (buttonPressed) {
    menuIndex++;
    if (menuIndex > 2) {
      editingSettings = false;
      menuIndex = 0;
    }
  }
}

void printBrightnessSetting(int knobDiff) {
  brightness = brightness + knobDiff;
  if (brightness < 0) {
    brightness = 0;
  } else if (brightness > 15) {
    brightness = 15;
  }
  
  matrix.setBrightness(brightness);
  
  matrix.print(brightness);
  matrix.writeDigitNum(1, 0xB);
  matrix.drawColon(true);
}

void printPressureWholeNumberSetting(int knobDiff) {
  seaLevelPressureHpa = seaLevelPressureHpa + knobDiff;
  int seaLevelPressureHpaInt = seaLevelPressureHpa;
  matrix.println(seaLevelPressureHpaInt);
}

void printPressureDecimalNumberSetting(int knobDiff) {
  seaLevelPressureHpa = seaLevelPressureHpa + (knobDiff * 0.01);
  int seaLevelPressureHpaInt = (int) (seaLevelPressureHpa + 0.005);
  
  float seaLevelPressureHpaAfterDecimal = seaLevelPressureHpa - seaLevelPressureHpaInt;
  matrix.print(seaLevelPressureHpaAfterDecimal);

  matrix.writeDigitNum(0, (seaLevelPressureHpaInt / 10) % 10);
  matrix.writeDigitNum(1, seaLevelPressureHpaInt % 10, true);

  if ((int) (seaLevelPressureHpaAfterDecimal * 100) == 0) {
    matrix.writeDigitNum(3, 0);
  }
}

void displayInformation(int knobDiff, bool buttonPressed) {
  if (buttonPressed) {
    editingSettings = true;
    menuIndex = 0;
    return;
  }
  
  menuIndex = (menuIndex + knobDiff + 2) % 2;
  
  matrix.clear();
  matrix.blinkRate(HT16K33_BLINK_OFF);
  
  if (menuIndex == 0) {
    printAltitude(bmp.readAltitude(seaLevelPressureHpa));
  } else {
    printTemperature(bmp.temperature);
  }

  matrix.writeDisplay();
}

void printAltitude(float altitudeMeters) {
  long reading = convertToFeet(altitudeMeters);

  if (altitudeFeet <= EARTH_CORE_ALTITUDE_FEET) {
    altitudeFeet = reading;
  }

  if (reading == altitudeFeet) {
    lastAltitudeDebounceTime = millis();
  }
  
  if ((millis() - lastAltitudeDebounceTime) > altitudeDebounceDelay) {
    altitudeFeet = reading;
  }

  matrix.println(altitudeFeet);
}

long convertToFeet(float altitudeMeters) {
  return round(altitudeMeters * FEET_PER_METER);
}

void printTemperature(float degreesCelcius) {
  int degreesFahrenheit = convertToFahrenheit(degreesCelcius);

  if (degreesFahrenheit <= -100) {
    matrix.printError();
    matrix.writeDisplay();
    return;
  } else if (degreesFahrenheit <= -10) {
    matrix.writeDigitRaw(0, MINUS_SIGN);
  } else if (degreesFahrenheit < 0) {
    matrix.writeDigitRaw(1, MINUS_SIGN);
  }

  int absoluteDegreesFahrenheit = abs(degreesFahrenheit);

  int hundredsDigit = absoluteDegreesFahrenheit / 100;
  if (hundredsDigit > 0) {
    matrix.writeDigitNum(0, hundredsDigit);
  }

  int tensDigit = absoluteDegreesFahrenheit / 10;
  if (tensDigit > 0 || hundredsDigit > 0) {
    matrix.writeDigitNum(1, tensDigit);
  }
  
  matrix.writeDigitNum(3, absoluteDegreesFahrenheit % 10, true);
  matrix.writeDigitNum(4, 0xF);
}

int convertToFahrenheit(float degreesCelcius) {
  return round(degreesCelcius * 1.8 + 32);
}
