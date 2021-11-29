#include "VGAX.h"
#include "font.h"

#include <stdlib.h>

VGAX VGA;

const bool VGA_BGCOLOR = 1;
const bool VGA_FGCOLOR = 0;

char TEST_STRING[32] = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
char TEST_STRING_SIZEOF_FRAMEBUFFER[32] = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
const char TEST_STRING_INFO[32] PROGMEM = "2KB SRAM / 8MHZ CLK";
const char TEST_STRING_DEVICE_NAME[32] PROGMEM = "ARDUINO UNO R3";
const char TEST_STRING_DEVELOPER_NAME[32] PROGMEM = "KATZNBOYZ 2021";

volatile long int i;

void setup() {

    VGA.begin();
    VGA.fillrect(0, 0, VGAX_WIDTH, VGAX_HEIGHT, VGA_BGCOLOR);
}

void loop() {

    // dont clear the screen - it takes too long
    // only refresh the parts that need clearing (first line)
    //VGA.clear(VGA_BGCOLOR);

    memcpy(vgaxfb_double_buffer, vgaxfb, sizeof(vgaxfb_double_buffer));

    VGA.fillrect(2, 1, VGAX_WIDTH, FNT_NANOFONT_HEIGHT, VGA_BGCOLOR);

    sprintf(TEST_STRING, "FRAMES: %d", i);
    sprintf(TEST_STRING_SIZEOF_FRAMEBUFFER, "FRAMEBUFF BYTES: %d", sizeof(vgaxfb));

    VGA.printSRAM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, TEST_STRING, 2, 1, VGA_FGCOLOR);
    VGA.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, TEST_STRING_INFO, 2, 1 + (7 * 1), VGA_FGCOLOR);
    VGA.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, TEST_STRING_DEVICE_NAME, 2, 1 + (7 * 2), VGA_FGCOLOR);
    VGA.printPROGMEM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, TEST_STRING_DEVELOPER_NAME, 2, 1 + (7 * 3), VGA_FGCOLOR);
    VGA.printSRAM((byte*)fnt_nanofont_data, FNT_NANOFONT_SYMBOLS_COUNT, FNT_NANOFONT_HEIGHT, 3, 1, TEST_STRING_SIZEOF_FRAMEBUFFER, 2, 1 + (7 * 4), VGA_FGCOLOR);

    VGA.delay(17 * 2);

    i++;
}