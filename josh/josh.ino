#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_7segment matrix = Adafruit_7segment();

void setup() {
  matrix.begin(0x70);
  matrix.setBrightness(0);
//  matrix.blinkRate(HT16K33_BLINK_DISPLAYON);

  matrix.writeDigitRaw(0, B00011110);
  matrix.writeDigitRaw(1, B00111111);
  matrix.writeDigitRaw(3, B01101101);
  matrix.writeDigitRaw(4, B01110110);
  matrix.writeDisplay();
}

void loop() {
  for (int i = 1; i <= 15; i++) {
    matrix.setBrightness(i);
    delay(50);
  }

  for (int i = 15; i >= 0; i--) {
    matrix.setBrightness(i);
    delay(50);
  }
}
