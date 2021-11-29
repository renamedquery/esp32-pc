#include <ESP32Lib.h>
#include <Ressources/CodePage437_9x16.h>

VGA3Bit vga;

const int PIN_R = 25;
const int PIN_G = 26;
const int PIN_B = 27;
const int PIN_VSYNC = 33;
const int PIN_HSYNC = 32;

char pin_info[64] = "";
char line_separator[64] = "---------------------------------------------------------------";

void setup() {

    Serial.begin(9600);

    vga.init(vga.MODE640x400, PIN_R, PIN_G, PIN_B, PIN_HSYNC, PIN_VSYNC);
    vga.setFont(CodePage437_9x16);
    vga.setTextColor(vga.RGB(255, 255, 255), vga.RGB(0, 0, 0));

    sprintf(pin_info, "PIN_R=%d\nPIN_G=%d\nPIN_B=%d\nPIN_HSYNC=%d\nPIN_VSYNC=%d", PIN_R, PIN_G, PIN_B, PIN_HSYNC, PIN_VSYNC);

    vga.println("ESP-32S DEVELOPMENT BOARD");
    vga.println("520KB RAM BUILT IN/0KB EXTERNAL");
    vga.println("80MHZ CLOCK SPEED");
    vga.println("640X400 PIXELS TOTAL");
    vga.println("3 BIT VGA");
    vga.println("");

    vga.println(pin_info);
    vga.println("");

    vga.println("USING ESP32LIB VGA DRIVER WRITTEN BY BITLUNI");
    vga.println("MAIN PROGRAM WRITTEN BY KATZNBOYZ");
    vga.println("");

    vga.setTextColor(vga.RGB(255, 0, 0), vga.RGB(0, 0, 0));
    vga.println("RED COLOR TEST");
    vga.setTextColor(vga.RGB(0, 255, 0), vga.RGB(0, 0, 0));
    vga.println("GREEN COLOR TEST");
    vga.setTextColor(vga.RGB(0, 0, 255), vga.RGB(0, 0, 0));
    vga.println("BLUE COLOR TEST");
    vga.setTextColor(vga.RGB(255, 255, 255), vga.RGB(0, 0, 0));
    vga.println("");

    vga.println("DONE BOOTING");
    vga.println("");

    vga.println(line_separator);
    vga.println("");

    // so that the console input is on the bottom of the screen
    vga.println("");
    vga.print(">");

    Serial.println("SETUP IS DONE");
}

void loop() {

    while (Serial.available()) {

        String serial_string = Serial.readString();
        serial_string.replace('\n', ' ');

        char serial_string_char[64];
        serial_string.toCharArray(serial_string_char, serial_string.length() + 1);

        vga.print(serial_string_char);
        vga.print("\n");
        vga.scroll(16, vga.RGB(0, 0, 0));
        vga.print(">");
    }
}