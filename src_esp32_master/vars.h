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

#include <ESP32Lib.h>
#include <Ressources/CodePage437_9x16.h>
#include <WiFi.h>
#include <SPI.h>
#include <SD.h>

unsigned long boot_start_time = millis();

WiFiClient remote_clients[SLAVE_COUNT];

VGA3BitI vga;

SPIClass *spi = NULL;

String current_sd_path = "/";

String image_dir_in_queue = "";

uint64_t image_last_draw_time_ms = millis();

uint64_t time_between_image_frame_draw_ms = 1000 / 4;

int last_image_frame = 0;

int frames_per_frame_file = 4;

bool kill_current_async_task = false;

uint64_t loop_index = 0;

typedef int (*cli_function)(char[MAX_CLI_INPUT_LENGTH]);

uint8_t connected_slaves = 0;

char line_separator[64] = "---------------------------------------------------------------";