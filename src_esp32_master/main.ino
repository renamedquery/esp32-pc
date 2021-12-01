#define CONFIG_IDF_TARGET_ESP32 1

#include <ESP32Lib.h>
#include <Ressources/CodePage437_9x16.h>
#include <soc/rtc.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <SD.h>
#include <SPI.h>

#define MAX_CLI_INPUT_LENGTH 64
#define MAX_CLI_OUTPUT_LENGTH 128
#define MAX_CLI_OUTPUT_LENGTH_PER_LINE 64
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 350
#define SCREEN_SIZE (SCREEN_WIDTH*SCREEN_HEIGHT)
#define CLI_LINE_HEIGHT 16
#define SCREEN_BITDEPTH 3
#define MAX_BYTES (520*8*1000)
#define SLAVE_COUNT 5
#define WIFI_NAME "ESP32_MASTER_COMPUTE_DEVICE_1"
#define WIFI_PASSWORD "YOUR PASSWORD GOES HERE"
#define AP_MAX_CONNECTIONS 5
#define SD_SCLK 18
#define SD_MISO 19
#define SD_MOSI 23
#define SD_SS 5
#define SD_FILESYSTEM_ROOT "/sd"
#define SLAVE_RESET_PIN 21

WiFiClient remote_clients[SLAVE_COUNT];

VGA3BitI vga;

SPIClass *spi = NULL;

// its okay if this overflows, as long as we can %2 it then itll work
int loop_index = 0;

typedef int (*cli_function)(char[MAX_CLI_INPUT_LENGTH]);

const int PIN_R = 27;
const int PIN_G = 26;
const int PIN_B = 25;
const int PIN_VSYNC = 33;
const int PIN_HSYNC = 32;

uint8_t connected_slaves = 0;

char line_separator[64] = "---------------------------------------------------------------";

void scroll_terminal(int lines) {

    for (int i = 0; i < lines; i++) vga.scroll(CLI_LINE_HEIGHT, vga.RGB(0, 0, 0));
}

void (*reset)(void) = 0;

byte get_clock_speed_cpu_mhz() {

    return rtc_clk_cpu_freq_value(rtc_clk_cpu_freq_get()) / 1000 / 1000;
}

// ----------------------------------------------------------------------------------------------------------
// terminal functions below
// should return 0 upon success

int cli_cmd_fbmem(char full_command[MAX_CLI_INPUT_LENGTH]) {

    char memused_line1[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";
    char memused_line2[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";

    sprintf(memused_line1, "    FRAMEBUFFER 1: %d BYTES", (SCREEN_SIZE * sizeof(unsigned long)));
    sprintf(memused_line2, "    FRAMEBUFFER 2: %d BYTES", (SCREEN_SIZE * sizeof(unsigned long)));
    
    scroll_terminal(3);

    vga.println("USED FRAMEBUFFER MEMORY:");
    vga.println(memused_line1);
    vga.println(memused_line2);

    return 0;
}

int cli_cmd_fbinfo(char full_command[MAX_CLI_INPUT_LENGTH]) {

    char fbinfo_line1[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";
    char fbinfo_line2[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";
    char fbinfo_line3[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";
    char fbinfo_line4[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";

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

    return 0;
}

int cli_cmd_hwinfo(char full_command[MAX_CLI_INPUT_LENGTH]) {

    char hwinfo_line1[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";
    char hwinfo_line2[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";
    char hwinfo_line3[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";
    char hwinfo_line4[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";
    char hwinfo_line5[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";
    char hwinfo_line6[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";
    char hwinfo_line7[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";
    
    sprintf(hwinfo_line1, "    RED COLOR PIN: %d", PIN_R);
    sprintf(hwinfo_line2, "    GREEN COLOR PIN: %d", PIN_G);
    sprintf(hwinfo_line3, "    BLUE COLOR PIN: %d", PIN_B);
    sprintf(hwinfo_line4, "    VERTICAL SYNC PIN: %d", PIN_VSYNC);
    sprintf(hwinfo_line5, "    HORIZONTAL SYNC PIN: %d", PIN_HSYNC);
    sprintf(hwinfo_line6, "    MAX BYTES: %d", MAX_BYTES);
    sprintf(hwinfo_line7, "    CURRENT CLOCK SPEED: %dMHz", get_clock_speed_cpu_mhz());

    scroll_terminal(8);

    vga.println("HARDWARE INFORMATION");
    vga.println(hwinfo_line1);
    vga.println(hwinfo_line2);
    vga.println(hwinfo_line3);
    vga.println(hwinfo_line4);
    vga.println(hwinfo_line5);
    vga.println(hwinfo_line6);
    vga.println(hwinfo_line7);

    return 0;
}

int cli_cmd_help(char full_command[MAX_CLI_INPUT_LENGTH]) {

    scroll_terminal(9);
    vga.println("fbmem - PRINTS THE AMOUNT OF MEMORY USED BY THE FRAMEBUFFER");
    vga.println("fbinfo - PRINTS THE INFORMATION ABOUT THE FRAMEBUFFERS");
    vga.println("         RESOLUTION AND BIT DEPTH");
    vga.println("hwinfo - PRINTS THE INFORMATION ABOUT WHICH PINS ARE ASSIGNED");
    vga.println("         TO WHAT FUNCTION");
    vga.println("lsdev - PRINTS A LIST OF THE CONNECTED SLAVE DEVICES");
    vga.println("net <cmd> - EXECUTES AN ACTION ON THE WIFI INTERFACE");
    vga.println("df - PRINTS THE AVAILABLE DISK DRIVES AND THEIR INFORMATION");
    vga.println("sd <cmd> - EXECUTES A COMMAND IN RELATION TO THE SD CARD SLOT");

    return 0;
}

int cli_cmd_lsdev(char full_command[MAX_CLI_INPUT_LENGTH]) {

    char amount_of_slave_devices_text[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";

    connected_slaves = WiFi.softAPgetStationNum();

    sprintf(amount_of_slave_devices_text, "LISTING %d CONNECTED SLAVE DEVICES", connected_slaves);

    scroll_terminal(1);

    vga.println(amount_of_slave_devices_text);

    wifi_sta_list_t wifi_station_list;
    tcpip_adapter_sta_list_t adapter_station_list;

    // zero the bytes
    memset(&wifi_station_list, 0, sizeof(wifi_station_list));
    memset(&adapter_station_list, 0, sizeof(adapter_station_list));

    esp_wifi_ap_get_sta_list(&wifi_station_list);
    tcpip_adapter_get_sta_list(&wifi_station_list, &adapter_station_list);

    for (int i = 0; i < connected_slaves; i++) {

        tcpip_adapter_sta_info_t station = adapter_station_list.sta[i];

        char station_info[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";

        sprintf(station_info, "DEV_%d     %d:%d:%d:%d:%d:%d    %s", i, station.mac[0], station.mac[1], station.mac[2], station.mac[3], station.mac[4], station.mac[5], ip4addr_ntoa(&(station.ip)));

        scroll_terminal(1);

        vga.println(station_info);
    }

    return 0;
}

int cli_cmd_serial(char full_command[MAX_CLI_INPUT_LENGTH]) {

    String command_string = full_command;

    const int end_of_first_command = 4;

    if (command_string.substring(end_of_first_command, end_of_first_command + 4).equals("name")) {

        scroll_terminal(4);

        vga.println(WIFI_NAME);
        vga.println(WiFi.softAPIP().toString().c_str());
        vga.println(WiFi.softAPIPv6().toString().c_str());
        vga.println(WiFi.macAddress().c_str());

    } else if (command_string.substring(end_of_first_command, end_of_first_command + 4).equals("test")) {

        for (int i = 0; i < connected_slaves; i++) {

            if (remote_clients[i].connected()) {

                remote_clients[i].println("TEST DATA FROM MASTER DEVICE");
            }
        }

        scroll_terminal(1);

        vga.println("SENT TEST STRING TO SLAVE DEVICES VIA WIFI");

    } else if (command_string.substring(end_of_first_command, end_of_first_command + 4).equals("stop")) {

        WiFi.softAPdisconnect(true);

        scroll_terminal(1);

        vga.println("DISABLED WIFI SERIAL INTERFACE");

    } else if (command_string.substring(end_of_first_command, end_of_first_command + 5).equals("start")) {

        WiFi.softAP(WIFI_NAME, WIFI_PASSWORD, 1, 0, AP_MAX_CONNECTIONS);

        scroll_terminal(1);

        vga.println("STARTED WIFI SERIAL INTERFACE");

    } else if (command_string.substring(end_of_first_command, end_of_first_command + 4).equals("list")) {

        cli_cmd_lsdev(full_command);

    } else if (command_string.substring(end_of_first_command, end_of_first_command + 7).equals("restart")) {

        digitalWrite(SLAVE_RESET_PIN, LOW);

        delay(100);

        digitalWrite(SLAVE_RESET_PIN, HIGH);

        scroll_terminal(1);

        vga.println("RESET ALL CONNECTED SLAVE DEVICES");

    } else {

        scroll_terminal(7);

        vga.println("UNKNOWN NET COMMAND. VALID COMMANDS ARE:");
        vga.println("net name - PRINTS THE NAME OF THE WIFI INTERFACE");
        vga.println("net test - SENDS OUT A TEST MESSAGE TO ALL WIFI SERIAL DEVICES");
        vga.println("net stop - STOPS THE WIFI SERIAL INTERFACE");
        vga.println("net start - STARTS THE WIFI SERIAL INTERFACE");
        vga.println("net list - LISTS ALL CONNECTED SLAVE DEVICES");
        vga.println("net restart - RESTARTS ALL CONNECTED SLAVE DEVICES");
    }

    return 0;
}

int cli_cmd_df(char full_command[MAX_CLI_INPUT_LENGTH]) {

    char sd_card_filesystem_description[MAX_CLI_OUTPUT_LENGTH_PER_LINE];

    sprintf(sd_card_filesystem_description, "%s    %dB TOTAL    %dB USED", SD_FILESYSTEM_ROOT, SD.totalBytes(), SD.usedBytes());

    scroll_terminal(1);

    vga.println(sd_card_filesystem_description);

    return 0;
}

int cli_cmd_sd(char full_command[MAX_CLI_INPUT_LENGTH]) {

    String command_string = full_command;

    const int end_of_first_command = 3;

    if (command_string.substring(end_of_first_command, end_of_first_command + 5).equals("eject")) {

        SD.end();

        char sd_ejection_msg[MAX_CLI_OUTPUT_LENGTH_PER_LINE];

        sprintf(sd_ejection_msg, "EJECTED SD CARD %s", SD_FILESYSTEM_ROOT);

        scroll_terminal(1);

        vga.println(sd_ejection_msg);

    } else if (command_string.substring(end_of_first_command, end_of_first_command + 4).equals("init")) {
 
        bool sd_start_status = SD.begin(SD_SS, *spi);

        scroll_terminal(1);

        vga.println(sd_start_status ? "SUCCESSFULLY INITIALIZED SD CARD" : "FAILED TO INITIALIZE SD CARD");

    } else {

        scroll_terminal(3);
        vga.println("UNKNOWN SD COMMAND. VALID COMMANDS ARE:");
        vga.println("sd eject - EJECTS THE SD CARD");
        vga.println("sd init - INITIALIZES THE SD CARD");
    }

    return 0;
}

int cli_cmd_nop(char full_command[MAX_CLI_INPUT_LENGTH]) {

    // do nothing
    // no operation -> nop
    // this can be used to write comments
    // ex: `nop this is a comment`

    return 0;
}

int cli_cmd_err(char full_command[MAX_CLI_INPUT_LENGTH]) {

    // for testing, and throwing errors in scripts

    throw nullptr;

    return 0;
}

void cli_output(cli_function function, char full_command[MAX_CLI_INPUT_LENGTH], VGA3BitI &vga_output) {

    try {

        int return_value = function(full_command);

        char return_value_char[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";

        sprintf(return_value_char, "%d", return_value);

        scroll_terminal(1);

        if (return_value != 0) {vga_output.setTextColor(vga_output.RGB(255, 0, 0), vga_output.RGB(0, 0, 0));}
        else {vga_output.setTextColor(vga_output.RGB(0, 255, 0), vga_output.RGB(0, 0, 0));}

        vga_output.println(return_value_char);

        vga_output.setTextColor(vga_output.RGB(255, 255, 255), vga_output.RGB(0, 0, 0));

    } catch (...) {

        scroll_terminal(1);

        vga_output.setTextColor(vga_output.RGB(255, 0, 0), vga_output.RGB(0, 0, 0));

        vga_output.println("ERROR WHILE RUNNING COMMAND. EXIT STATUS CAN NOT BE DETERMINED.");

        vga_output.setTextColor(vga_output.RGB(255, 255, 255), vga_output.RGB(0, 0, 0));
    }
}

void cli_nocmd() {

    scroll_terminal(1);

    vga.setTextColor(vga.RGB(255, 0, 0), vga.RGB(0, 0, 0));

    vga.println("UNKNOWN COMMAND");

    vga.setTextColor(vga.RGB(255, 255, 255), vga.RGB(0, 0, 0));
}

// ----------------------------------------------------------------------------------------------------------
// arduino functions below

void setup() {

    spi = new SPIClass(VSPI);
    spi->begin(SD_SCLK, SD_MISO, SD_MOSI, SD_SS);

    WiFi.mode(WIFI_AP);
    bool softap_start_status = WiFi.softAP(WIFI_NAME, WIFI_PASSWORD, 1, 0, AP_MAX_CONNECTIONS);

    Serial.begin(9600);

    bool sd_start_status = SD.begin(SD_SS, *spi);

    vga.init(vga.MODE640x350, PIN_R, PIN_G, PIN_B, PIN_HSYNC, PIN_VSYNC);
    vga.setFont(CodePage437_9x16);
    vga.setTextColor(vga.RGB(255, 255, 255), vga.RGB(0, 0, 0));
    vga.setFrameBufferCount(1);

    char pin_info[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";
    sprintf(pin_info, "PIN_R=%d\nPIN_G=%d\nPIN_B=%d\nPIN_HSYNC=%d\nPIN_VSYNC=%d", PIN_R, PIN_G, PIN_B, PIN_HSYNC, PIN_VSYNC);

    char clock_speed[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";
    sprintf(clock_speed, "%dMHz CLOCK SPEED", get_clock_speed_cpu_mhz());

    char softap_start_info[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";
    sprintf(softap_start_info, "SOFTAP STATUS %d", (byte)softap_start_status);

    char sd_start_info[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";
    sprintf(sd_start_info, "SD STATUS %d", (byte)sd_start_status);

    pinMode(SLAVE_RESET_PIN, OUTPUT);
    digitalWrite(SLAVE_RESET_PIN, HIGH);

    vga.println("ESP-32S DEVELOPMENT BOARD");
    vga.println("520KB RAM BUILT IN/0KB EXTERNAL");
    vga.println(clock_speed);
    vga.println("640X350 PIXELS TOTAL");
    vga.println("3 BIT VGA");
    vga.println(softap_start_info);
    vga.println(pin_info);
    vga.println(sd_start_info);

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

    vga.println(line_separator);

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

        if (serial_string.substring(0, 5).equals("fbmem")) {cli_output(&cli_cmd_fbmem, serial_string_char, vga);}
        else if (serial_string.substring(0, 6).equals("fbinfo")) {cli_output(&cli_cmd_fbinfo, serial_string_char, vga);} 
        else if (serial_string.substring(0, 6).equals("hwinfo")) {cli_output(&cli_cmd_hwinfo, serial_string_char, vga);} 
        else if (serial_string.substring(0, 5).equals("lsdev")) {cli_output(&cli_cmd_lsdev, serial_string_char, vga);} 
        else if (serial_string.substring(0, 3).equals("net")) {cli_output(&cli_cmd_serial, serial_string_char, vga);} 
        else if (serial_string.substring(0, 2).equals("df")) {cli_output(&cli_cmd_df, serial_string_char, vga);} 
        else if (serial_string.substring(0, 2).equals("sd")) {cli_output(&cli_cmd_sd, serial_string_char, vga);} 
        else if (serial_string.substring(0, 4).equals("help")) {cli_output(&cli_cmd_help, serial_string_char, vga);} 
        else if (serial_string.substring(0, 3).equals("nop")) {cli_output(&cli_cmd_nop, serial_string_char, vga);} 
        else if (serial_string.substring(0, 3).equals("err")) {cli_output(&cli_cmd_err, serial_string_char, vga);} 
        else if (serial_string.substring(0, 6).equals("reboot")) {reset();} 
        else {cli_nocmd();}
    }

    loop_index++;
}