#include "VGAX.h"

VGAX VGA;

const byte VGA_BGCOLOR = 00;
const byte VGA_FGCOLOR = 11;

void setup() {

    VGA.begin();
    VGA.clear(VGA_BGCOLOR);
}

void loop() {

    for (register byte y = 0; y < VGAX_HEIGHT; y++) {

        for (register byte x = 0; x < VGAX_WIDTH; x++) {

            if (((x % (VGAX_WIDTH / 4)) == 0) || ((y % (VGAX_HEIGHT / 4)) == 0)) {

                VGA.putpixel(x, y, VGA_FGCOLOR);
            }
        }
    }

    VGA.delay(17);
}