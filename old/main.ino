#include "VGAX.h"
#include "font.h"

#include <stdlib.h>

VGAX VGA;

const byte VGA_BGCOLOR = 11;
const byte VGA_FGCOLOR = 00;

char TEST_STRING[32] = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
const char TEST_STRING_INFO[32] PROGMEM = "32KB SRAM | 8MHZ CLK";
const char TEST_STRING_DEVICE_NAME[32] PROGMEM = "ARDUINO UNO R3";
const char TEST_STRING_DEVELOPER_NAME[32] PROGMEM = "KATZNBOYZ 2021";

volatile long int i;

void setup() {

    VGA.begin();
    VGA.clear(VGA_BGCOLOR);
}

void loop() {

    // dont clear the screen - it takes too long
    // only refresh the parts that need clearing (first line)
    //VGA.clear(VGA_BGCOLOR);
    VGA.fillrect(12, (VGAX_HEIGHT / 2) - 4 - 8, VGAX_WIDTH - 12, FNT_NANOFONT_HEIGHT, VGA_BGCOLOR);

    sprintf(TEST_STRING, "%d FRAMES", i);

    VGA.printSRAM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, TEST_STRING, 12, (VGAX_HEIGHT / 2) - 4 - 8, VGA_FGCOLOR);
    VGA.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, TEST_STRING_INFO, 12, (VGAX_HEIGHT / 2) - 4, VGA_FGCOLOR);
    VGA.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, TEST_STRING_DEVICE_NAME, 12, (VGAX_HEIGHT / 2) + 4, VGA_FGCOLOR);
    VGA.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, TEST_STRING_DEVELOPER_NAME, 12, (VGAX_HEIGHT / 2) + 4 + 8, VGA_FGCOLOR);

    VGA.delay(17 * 2);

    i++;
}