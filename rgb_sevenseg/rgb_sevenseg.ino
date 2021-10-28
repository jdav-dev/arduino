#include "Adafruit_TLC5947.h"

// How many boards do you have chained?
#define NUM_TLC5974 1

#define data   11
#define clock   13
#define latch   10
#define oe  7  // set to -1 to not use the enable pin (its optional)

Adafruit_TLC5947 tlc = Adafruit_TLC5947(NUM_TLC5974, clock, data, latch);

uint8_t BLACK[] = {0, 0, 0};
uint8_t BLUE[] = {0, 40, 85};
uint8_t GOLD[] = {234, 170, 0};
uint8_t RED[] = {255, 0, 0};

const int SPEAKER=9;

const int NOTE_B2=123;

void setup() {
  Serial.begin(9600);
  
  Serial.println("TLC5974 test");
  tlc.begin();
  
  if (oe >= 0) {
    pinMode(oe, OUTPUT);
    digitalWrite(oe, LOW);
  }
  
  uint8_t a = 1;

  for (uint8_t i = 16; i >= 1; i--) {
    byte mask = valueMask(i-1);
    writeMask(mask, 1);
    delay(1000);
  }

  setSegment(0, RED, a);
  setSegment(1, RED, a);
  setSegment(2, RED, a);
  setSegment(3, RED, a);
  setSegment(4, RED, a);
  setSegment(5, RED, a);
  setSegment(6, BLACK, a);
  tlc.write();

  tone(SPEAKER, NOTE_B2, 1000);
}

void loop() {
//  brightCycle();

//  for (uint8_t i = 16; i >= 1; i--) {
//    byte mask = valueMask(i-1);
//    writeMask(mask, 1);
//    delay(1000);
//  }
}

void setSegment(uint8_t segment, uint8_t color[3], uint8_t a) {
  setSegment(segment, color[0], color[1], color[2], a);
}

void setSegment(uint8_t segment, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  uint16_t mappedA = map(a, 0, 255, 0, 4095);

  uint16_t mappedR = map(r, 0, 255, 0, mappedA);
  uint16_t mappedG = map(g, 0, 255, 0, mappedA);
  uint16_t mappedB = map(b, 0, 255, 0, mappedA);

  tlc.setLED(segment, mappedR, mappedG, mappedB);
}

byte valueMask(uint8_t value) {
  switch (value) {
    case 0: return B00111111;
    case 1: return B00000110;
    case 2: return B01011011;
    case 3: return B01001111;
    case 4: return B01100110;
    case 5: return B01101101;
    case 6: return B01111101;
    case 7: return B00100111;
    case 8: return B01111111;
    case 9: return B01101111;
    case 10: return B01110111;
    case 11: return B01111100;
    case 12: return B00111001;
    case 13: return B01011110;
    case 14: return B01111001;
    case 15: return B01110001;
    default: return B00000000;
  }
}

void writeMask(byte value, uint8_t a) {
  for (uint8_t i = 0, mask = 1; i < 7; i++, mask = mask << 1) {
    if (value & mask) {
      setSegment(i, GOLD, a);
    } else {
      setSegment(i, BLUE, a);
    }
  }

  tlc.write();
}

void brightCycle() {
  uint16_t r = 234;
  uint16_t g = 170;
  uint16_t b = 0;

  uint16_t r2 = 0;
  uint16_t g2 = 40;
  uint16_t b2 = 85;
      
  for (uint8_t a = 0; a < 255; a++) {
    setSegment(0, r2, g2, b2, a);
    setSegment(1, r, g, b, a);
    setSegment(2, r, g, b, a);
    setSegment(3, r2, g2, b2, a);
    setSegment(4, r2, g2, b2, a);
    setSegment(5, r, g, b, a);
    setSegment(6, r, g, b, a);
    tlc.write();
  }

  for (uint8_t a = 255; a > 0; a--) {
    setSegment(0, r2, g2, b2, a);
    setSegment(1, r, g, b, a);
    setSegment(2, r, g, b, a);
    setSegment(3, r2, g2, b2, a);
    setSegment(4, r2, g2, b2, a);
    setSegment(5, r, g, b, a);
    setSegment(6, r, g, b, a);
    tlc.write();
  }
}
