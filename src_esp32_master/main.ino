#define CONFIG_IDF_TARGET_ESP32 1

#include <ESP32Lib.h>
#include <Ressources/CodePage437_9x16.h>
#include <soc/rtc.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <SD.h>
#include <SPI.h>
#include <vector>

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
#define SD_FILESYSTEM_ROOT "/sd/"
#define SLAVE_RESET_PIN 21
#define PIN_R 27
#define PIN_G 26
#define PIN_B 25
#define PIN_VSYNC 33
#define PIN_HSYNC 32

unsigned long boot_start_time = millis();

WiFiClient remote_clients[SLAVE_COUNT];

VGA3BitI vga;

SPIClass *spi = NULL;

String current_sd_path = "/";

String image_dir_in_queue = "";

uint64_t image_last_draw_time_ms = millis();

uint64_t time_between_image_frame_draw_ms = 1000 / 4;

int last_image_frame = 0;

bool kill_current_async_task = false;

// its okay if this overflows, as long as we can %2 it then itll work
int loop_index = 0;

typedef int (*cli_function)(char[MAX_CLI_INPUT_LENGTH]);

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

    scroll_terminal(17);
    vga.println("fbmem - PRINTS THE AMOUNT OF MEMORY USED BY THE FRAMEBUFFER");
    vga.println("fbinfo - PRINTS THE INFORMATION ABOUT THE FRAMEBUFFERS");
    vga.println("         RESOLUTION AND BIT DEPTH");
    vga.println("hwinfo - PRINTS THE INFORMATION ABOUT WHICH PINS ARE ASSIGNED");
    vga.println("         TO WHAT FUNCTION");
    vga.println("lsdev - PRINTS A LIST OF THE CONNECTED SLAVE DEVICES");
    vga.println("net <cmd> - EXECUTES AN ACTION ON THE WIFI INTERFACE");
    vga.println("df - PRINTS THE AVAILABLE DISK DRIVES AND THEIR INFORMATION");
    vga.println("sd <cmd> - EXECUTES A COMMAND IN RELATION TO THE SD CARD SLOT");
    vga.println("mkdir <absolute_path> - CREATES A NEW DIRECTORY");
    vga.println("ls - LISTS ALL FILES/FOLDERS IN THE CURRENT DIRECTORY");
    vga.println("touch <absolute_path> - CREATES A FILE IN THE CURRENT DIRECTORY");
    vga.println("cd <absolute_path> - SETS THE CURRENT DIRECTORY TO THE");
    vga.println("                     SPECIFIED PATH");
    vga.println("readimgseq <absolute_path> - READS A BINARY IMAGE SEQUENCE");
    vga.println("cls - CLEARS THE SCREEN");
    vga.println("screenshot <absolute_path> - WRITES A SCREENSHOT TO THE PATH");

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

    sprintf(sd_card_filesystem_description, "%s    %lluB TOTAL    %lluB USED", SD_FILESYSTEM_ROOT, SD.totalBytes(), SD.usedBytes());

    scroll_terminal(1);

    vga.println(sd_card_filesystem_description);

    return 0;
}

int cli_cmd_mkdir(char full_command[MAX_CLI_INPUT_LENGTH]) {

    String command_string = full_command;

    const int end_of_first_command = 6;

    String dirname = "";

    for (int i = end_of_first_command; i < MAX_CLI_INPUT_LENGTH; i++) {

        if (command_string[i] == ' ') {

            break;

        } else {

            dirname += command_string.substring(i, i + 1);
        }
    }

    if (dirname.isEmpty()) return 1;

    bool mkdir_status = SD.mkdir(current_sd_path + "/" + dirname);

    if (!mkdir_status) return 2;

    char mkdir_output_info[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";

    sprintf(mkdir_output_info, "CREATED DIR %s", (current_sd_path + "/" + dirname).c_str());

    scroll_terminal(1);

    vga.println(mkdir_output_info);

    return 0;
}

/*
    known issues:
    - touching a file starting with a period will crash the OS
    - touching a file with the same name as a directory will fail with error code 2
    
    if you find more issues, please add to this list or raise an issue on the github repo!
*/
int cli_cmd_touch(char full_command[MAX_CLI_INPUT_LENGTH]) {

    String command_string = full_command;

    const int end_of_first_command = 6;

    String filename = "";

    for (int i = end_of_first_command; i < MAX_CLI_INPUT_LENGTH; i++) {

        if (command_string.substring(i, i + 1).isEmpty()) {

            break;

        } else {

            filename += command_string.substring(i, i + 1);
        }
    }

    if (filename.isEmpty()) return 1;

    if (SD.exists(filename)) return 2;

    try {

        File new_file = SD.open(filename, "w");
        new_file.close();
    
    } catch (...) {

        return 3;
    }

    char touch_output_info[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";

    sprintf(touch_output_info, "CREATED FILE %s", (current_sd_path + "/" + filename).c_str());

    scroll_terminal(1);

    vga.println(touch_output_info);

    return 0;
}

int cli_cmd_ls(char full_command[MAX_CLI_INPUT_LENGTH]) {

    File directory_contents = SD.open(current_sd_path);

    while (1) {

        File current_listed_file = directory_contents.openNextFile();

        if (!current_listed_file) break;

        if (current_listed_file.isDirectory()) vga.setTextColor(vga.RGB(0, 0, 255), vga.RGB(0, 0, 0));

        scroll_terminal(1);

        vga.println(current_listed_file.name());

        vga.setTextColor(vga.RGB(255, 255, 255), vga.RGB(0, 0, 0));
    }

    directory_contents.close();

    return 0;
}

int cli_cmd_cd(char full_command[MAX_CLI_INPUT_LENGTH]) {

    String command_string = full_command;

    const int end_of_first_command = 3;

    String directory = "";

    for (int i = end_of_first_command; i < MAX_CLI_INPUT_LENGTH; i++) {

        if (command_string[i] == ' ') {

            break;

        } else {

            directory += command_string.substring(i, i + 1);
        }
    }

    if (directory.isEmpty()) return 1;

    try {

        // this will fail if the path doesnt exist, thus returning the error code 2
        File directory_test_listing = SD.open(directory, "r");
        directory_test_listing.close();

        current_sd_path = directory;

        scroll_terminal(1);

        char directory_move_status[MAX_CLI_OUTPUT_LENGTH_PER_LINE] = "";

        sprintf(directory_move_status, "CD: %s", current_sd_path.c_str());

        vga.println(directory_move_status);

    } catch (...) {

        scroll_terminal(1);

        vga.println("NOT A DIRECTORY");

        return 2;
    }

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

int cli_cmd_read_image_sequence(char full_command[MAX_CLI_INPUT_LENGTH]) {

    String command_string = full_command;

    const int end_of_first_command = 11;

    String directory = "";

    for (int i = end_of_first_command; i < MAX_CLI_INPUT_LENGTH; i++) {

        if (command_string[i] == ' ') {

            break;

        } else {

            directory += command_string.substring(i, i + 1);
        }
    }

    // this command uses the assumption that you are using a filename that is xxxxxxxxx.jpg.txt
    // it will NOT work otherwise!
    image_dir_in_queue = directory;
    last_image_frame = 0;

    return 0;
}

int cli_cmd_screenshot(char full_command[MAX_CLI_INPUT_LENGTH]) {

    String command_string = full_command;

    const int end_of_first_command = 11;

    String filename = "";

    for (int i = end_of_first_command; i < MAX_CLI_INPUT_LENGTH; i++) {

        if (command_string.substring(i, i + 1).isEmpty()) {

            break;

        } else {

            filename += command_string.substring(i, i + 1);
        }
    }

    try {

        File output_file = SD.open(filename, "w");

        output_file.print(SCREEN_WIDTH); output_file.print(";"); output_file.print(SCREEN_HEIGHT); output_file.print('\n');

        for (int y = 0; y < SCREEN_HEIGHT; y++) {

            for (int x = 0; x < SCREEN_WIDTH; x++) {

                output_file.print(vga.frameBuffers[vga.currentFrameBuffer][y][x]); output_file.print(";");
            }

            output_file.print('\n');
        }

        output_file.close();

    } catch (...) {

        return 1;
    }

    return 0;
}

int cli_cmd_cls(char full_command[MAX_CLI_INPUT_LENGTH]) {

    vga.clear();

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
    spi->setFrequency((uint32_t)(get_clock_speed_cpu_mhz()) * 1000 * 1000); // might be unstable at 240Mhz?

    WiFi.mode(WIFI_AP);
    bool softap_start_status = WiFi.softAP(WIFI_NAME, WIFI_PASSWORD, 1, 0, AP_MAX_CONNECTIONS);

    Serial.begin(9600);

    bool sd_start_status = SD.begin(SD_SS, *spi);

    vga.init(vga.MODE640x350, PIN_R, PIN_G, PIN_B, PIN_HSYNC, PIN_VSYNC);
    vga.setFont(CodePage437_9x16);
    vga.setTextColor(vga.RGB(255, 255, 255), vga.RGB(0, 0, 0));
    vga.setFrameBufferCount(2);

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
    vga.print("REPORT ISSUES TO ");
    vga.setTextColor(vga.RGB(0, 0, 255), vga.RGB(0, 0, 0));
    vga.println("https://github.com/katznboyz1/esp32-pc");
    vga.setTextColor(vga.RGB(255, 255, 255), vga.RGB(0, 0, 0));

    vga.setTextColor(vga.RGB(255, 0, 0), vga.RGB(0, 0, 0));
    vga.println("RED COLOR TEST");
    vga.setTextColor(vga.RGB(0, 255, 0), vga.RGB(0, 0, 0));
    vga.println("GREEN COLOR TEST");
    vga.setTextColor(vga.RGB(0, 0, 255), vga.RGB(0, 0, 0));
    vga.println("BLUE COLOR TEST");
    vga.setTextColor(vga.RGB(255, 255, 255), vga.RGB(0, 0, 0));

    unsigned long boot_to_current_time = (millis() - boot_start_time);
    char boot_time_info[MAX_CLI_INPUT_LENGTH] = "";
    sprintf(boot_time_info, "BOOTING TOOK %luMS", boot_to_current_time);
    
    vga.println(boot_time_info);

    vga.println("DONE BOOTING");

    vga.println(line_separator);

    Serial.println("SETUP IS DONE");
}

void loop() {

    uint64_t current_time_ms = millis();

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
        else if (serial_string.substring(0, 3).equals("cls")) {cli_output(&cli_cmd_cls, serial_string_char, vga);} 
        else if (serial_string.substring(0, 2).equals("df")) {cli_output(&cli_cmd_df, serial_string_char, vga);} 
        else if (serial_string.substring(0, 2).equals("sd")) {cli_output(&cli_cmd_sd, serial_string_char, vga);} 
        else if (serial_string.substring(0, 2).equals("ls")) {cli_output(&cli_cmd_ls, serial_string_char, vga);} 
        else if (serial_string.substring(0, 2).equals("cd")) {cli_output(&cli_cmd_cd, serial_string_char, vga);} 
        else if (serial_string.substring(0, 5).equals("mkdir")) {cli_output(&cli_cmd_mkdir, serial_string_char, vga);}
        else if (serial_string.substring(0, 5).equals("touch")) {cli_output(&cli_cmd_touch, serial_string_char, vga);} 
        else if (serial_string.substring(0, 10).equals("readimgseq")) {cli_output(&cli_cmd_read_image_sequence, serial_string_char, vga);} 
        else if (serial_string.substring(0, 10).equals("screenshot")) {cli_output(&cli_cmd_screenshot, serial_string_char, vga);} 
        else if (serial_string.substring(0, 4).equals("help")) {cli_output(&cli_cmd_help, serial_string_char, vga);} 
        else if (serial_string.substring(0, 3).equals("nop")) {cli_output(&cli_cmd_nop, serial_string_char, vga);} 
        else if (serial_string.substring(0, 3).equals("err")) {cli_output(&cli_cmd_err, serial_string_char, vga);} 
        else if (serial_string.substring(0, 6).equals("reboot")) {reset();} 
        else {cli_nocmd();}
    }

    if (!image_dir_in_queue.isEmpty() && !kill_current_async_task) {

        image_last_draw_time_ms = current_time_ms;

        try {

            String current_img = String(last_image_frame);
            char current_img_char[10] = "";
            current_img.toCharArray(current_img_char, 10);

            sprintf(current_img_char, "%09d", last_image_frame);

            current_img = String(current_img_char) + ".jpg.esp32binimg";

            File image_to_draw = SD.open(image_dir_in_queue + '/' + current_img, "rb");

            // we will miss some data, but thats okay
            image_to_draw.setTimeout(time_between_image_frame_draw_ms / 2);

            //String current_img_res = image_to_draw.readStringUntil('\n');

            int image_width, image_height, image_width_rescaled, image_height_rescaled;

            // tempoary values just to test things out
            // TODO: fix this
            image_height = 80; //current_img_res.substring(0, current_img_res.indexOf(";")).toInt();
            image_width = 60; //current_img_res.substring(current_img_res.indexOf(";"), current_img_res.length()).toInt();

            const int image_scale = 2;

            int x_aspect = SCREEN_WIDTH / image_width / image_scale;
            int y_aspect = SCREEN_HEIGHT / image_height / image_scale;

            int x = 0;
            int y = 0;

            int anchor_x = 0;
            int anchor_y = 0;

            while (image_to_draw.available() > 0) {

                x++;

                byte current_pix_value_str = image_to_draw.read();

                if (x >= image_width || current_pix_value_str == 2) {

                    y++;
                    x = 0;
                }

                bool current_pix_val_1bit = !((bool)(current_pix_value_str));

                vga.fillRect((x * x_aspect) + anchor_x, (y * y_aspect) + anchor_y, x_aspect, (y_aspect * 2), vga.RGB(current_pix_val_1bit * 255, current_pix_val_1bit * 255, current_pix_val_1bit * 255));
            }

            image_to_draw.close();

            last_image_frame++;

        } catch (...) {

            image_dir_in_queue = "";
        }
    }

    loop_index++;
}