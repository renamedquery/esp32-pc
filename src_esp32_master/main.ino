#include <ESP32Lib.h>
#include <Ressources/CodePage437_9x16.h>

VGA3Bit vga;

const int PIN_R = 14;
const int PIN_G = 19;
const int PIN_B = 27;
const int PIN_VSYNC = 33;
const int PIN_HSYNC = 32;

char pin_info[64] = "";

void setup() {

    sprintf(pin_info, "PIN_R=%d PIN_G=%d PIN_B=%d PIN_HSYNC=%d PIN_VSYNC=%d", PIN_R, PIN_G, PIN_B, PIN_HSYNC, PIN_VSYNC);

    vga.init(vga.MODE500x480, PIN_R, PIN_G, PIN_B, PIN_HSYNC, PIN_VSYNC);
    vga.setFont(CodePage437_9x16);
    vga.setTextColor(vga.RGB(255, 255, 255), vga.RGB(0, 0, 0));

    vga.println("ESP-32S DEVELOPMENT BOARD");
    vga.println("520KB RAM BUILT IN/0KB EXTERNAL");
    vga.println("80MHZ CLOCK SPEED");
    vga.println("500X400 PIXELS TOTAL");
    vga.println(pin_info);
    vga.println("");
    vga.println("USING ESP32LIB VGA DRIVER WRITTEN BY BITLUNI");
    vga.println("MAIN PROGRAM WRITTEN BY KATZNBOYZ");
    vga.println("");
    vga.println("DONE BOOTING");
    vga.println("");
}

void loop() {

}