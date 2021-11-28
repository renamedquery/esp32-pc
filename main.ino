// code is for an arduino at the moment since the esp32s have been delayed in shipping

#include <Arduino.h>
#include <stdlib.h>

struct _PINS {

    const byte R = 7;
    const byte G = 6;
    const byte B = 5;

    const byte VSYNC = 9;
    const byte HSYNC = 3;
};

const _PINS PINS;

void setup() {

    pinMode(PINS.R, OUTPUT);
    pinMode(PINS.G, OUTPUT);
    pinMode(PINS.B, OUTPUT);

    pinMode(PINS.VSYNC, OUTPUT);
    pinMode(PINS.HSYNC, OUTPUT);
}

void loop() {}