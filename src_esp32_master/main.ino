#include <ESP32Lib.h>
#include <Ressources/CodePage437_9x16.h>

#define MAX_CLI_INPUT_LENGTH 64
#define MAX_CLI_OUTPUT_LENGTH 128
#define MAX_CLI_OUTPUT_LENGTH_PER_LINE 64
#define SCREEN_WIDTH 680
#define SCREEN_HEIGHT 480
#define SCREEN_SIZE (SCREEN_WIDTH*SCREEN_HEIGHT)
#define CLI_LINE_HEIGHT 16
#define SCREEN_BITDEPTH 3
#define HELP_MENU_LENGTH 3

VGA3Bit vga;

const int PIN_R = 25;
const int PIN_G = 26;
const int PIN_B = 27;
const int PIN_VSYNC = 33;
const int PIN_HSYNC = 32;

char help_menu[HELP_MENU_LENGTH][MAX_CLI_OUTPUT_LENGTH] = {
    "fbmem - PRINTS THE AMOUNT OF MEMORY USED BY THE FRAMEBUFFER",
    "fbinfo - PRINTS THE INFORMATION ABOUT THE FRAMEBUFFERS RESOLUTION AND BIT DEPTH",
    "hwinfo - PRINTS THE INFORMATION ABOUT WHICH PINS ARE ASSIGNED TO WHAT FUNCTION"
};

char pin_info[64] = "";
char line_separator[64] = "---------------------------------------------------------------";

void scroll_terminal(int lines) {

    for (int i = 0; i < lines; i++) vga.scroll(CLI_LINE_HEIGHT, vga.RGB(0, 0, 0));
}

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

    Serial.println("SETUP IS DONE");
}

void loop() {

    while (Serial.available()) {

        String serial_string = Serial.readString();
        serial_string.replace('\n', ' ');

        char serial_string_char[MAX_CLI_INPUT_LENGTH];
        serial_string.substring(0, MAX_CLI_INPUT_LENGTH - 1).toCharArray(serial_string_char, serial_string.length() + 1);

        vga.print(">");
        vga.print(serial_string_char);
        vga.print("\n");
        scroll_terminal(1);

        if (serial_string.substring(0, 5).equals("fbmem")) {

            char memused_line1[MAX_CLI_OUTPUT_LENGTH] = "";
            char memused_line2[MAX_CLI_OUTPUT_LENGTH] = "";

            sprintf(memused_line1, "    FRAMEBUFFER 1: %d BYTES", (SCREEN_SIZE * sizeof(unsigned long)));
            sprintf(memused_line2, "    FRAMEBUFFER 2: %d BYTES", (SCREEN_SIZE * sizeof(unsigned long)));
            
            scroll_terminal(3);

            vga.println("USED FRAMEBUFFER MEMORY:");
            vga.println(memused_line1);
            vga.println(memused_line2);
        
        } else if (serial_string.substring(0, 6).equals("fbinfo")) {

            char fbinfo_line1[MAX_CLI_OUTPUT_LENGTH] = "";
            char fbinfo_line2[MAX_CLI_OUTPUT_LENGTH] = "";
            char fbinfo_line3[MAX_CLI_OUTPUT_LENGTH] = "";
            char fbinfo_line4[MAX_CLI_OUTPUT_LENGTH] = "";

            sprintf(fbinfo_line1, "    SCREEN WIDTH: %d PIXELS", SCREEN_WIDTH);
            sprintf(fbinfo_line2, "    SCREEN HEIGHT: %d PIXELS", SCREEN_HEIGHT);
            sprintf(fbinfo_line3, "    SCREEN SIZE: %d PIXELS", SCREEN_SIZE);
            sprintf(fbinfo_line4, "    SCREEN COLORS: %d BIT", SCREEN_BITDEPTH);

            scroll_terminal(5);

            vga.println("FRAMEBUFFER INFORMATION:");
            vga.println(fbinfo_line1);
            vga.println(fbinfo_line2);
            vga.println(fbinfo_line3);
            vga.println(fbinfo_line4);

        } else if (serial_string.substring(0, 6).equals("hwinfo")) {

            char hwinfo_line1[MAX_CLI_OUTPUT_LENGTH] = "";
            char hwinfo_line2[MAX_CLI_OUTPUT_LENGTH] = "";
            char hwinfo_line3[MAX_CLI_OUTPUT_LENGTH] = "";
            char hwinfo_line4[MAX_CLI_OUTPUT_LENGTH] = "";
            char hwinfo_line5[MAX_CLI_OUTPUT_LENGTH] = "";
            
            sprintf(hwinfo_line1, "    RED COLOR PIN: %d", PIN_R);
            sprintf(hwinfo_line2, "    GREEN COLOR PIN: %d", PIN_G);
            sprintf(hwinfo_line3, "    BLUE COLOR PIN: %d", PIN_B);
            sprintf(hwinfo_line4, "    VERTICAL SYNC PIN: %d", PIN_VSYNC);
            sprintf(hwinfo_line5, "    HORIZONTAL SYNC PIN: %d", PIN_HSYNC);

            scroll_terminal(6);

            vga.println("HARDWARE INFORMATION");
            vga.println(hwinfo_line1);
            vga.println(hwinfo_line2);
            vga.println(hwinfo_line3);
            vga.println(hwinfo_line4);
            vga.println(hwinfo_line5);

        } else if (serial_string.substring(0, 4).equals("help")) {

            for (int i = 0; i < HELP_MENU_LENGTH; i++) {

                String help_menu_string = help_menu[i];

                for (int j = 0; j < MAX_CLI_OUTPUT_LENGTH; j += MAX_CLI_OUTPUT_LENGTH_PER_LINE) {

                    if (!(help_menu_string.substring(j, j + MAX_CLI_OUTPUT_LENGTH_PER_LINE).isEmpty())) {

                        char help_menu_line[MAX_CLI_OUTPUT_LENGTH_PER_LINE];
                        help_menu_string.substring(j, j + MAX_CLI_OUTPUT_LENGTH_PER_LINE).toCharArray(help_menu_line, MAX_CLI_OUTPUT_LENGTH_PER_LINE + 1);

                        scroll_terminal(1);
                        vga.println(help_menu_line);
                    }
                }
            }

        } else {

            scroll_terminal(1);
            vga.println("UNKNOWN COMMAND");
        }
    }
}