#define CONFIG_IDF_TARGET_ESP32 1

#include "cli.h"

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
    vga.clear();

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

            int current_embedded_frame = 0;

            while (image_to_draw.available() > 0) {

                byte current_pix_value_str = image_to_draw.read();
                byte current_pix_run_length = image_to_draw.read();

                bool current_pix_val_1bit = !((bool)(current_pix_value_str));

                unsigned char pix_clr = vga.RGB(current_pix_val_1bit * 255, current_pix_val_1bit * 255, current_pix_val_1bit * 255);

                for (int i = 0; i < current_pix_run_length; i++) {

                    if (x >= image_width) {

                        y++;
                        x = 0;
                    }

                    if (y >= image_height) {

                        y = 0;
                        x = 0;
                        current_embedded_frame++;
                    }

                    if (vga.frameBuffers[vga.currentFrameBuffer][y][x] != pix_clr) vga.fillRect(((x) * x_aspect) + anchor_x, ((y * 2) * y_aspect) + anchor_y, x_aspect, (y_aspect * 2), pix_clr);

                    x++;
                }

                x++;
            }

            image_to_draw.close();

            last_image_frame++;

        } catch (...) {

            image_dir_in_queue = "";
        }
    }

    loop_index++;
}