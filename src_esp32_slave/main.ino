#define WIFI_NAME "ESP32_MASTER_COMPUTE_DEVICE_1"
#define WIFI_PASSWORD "YOUR PASSWORD GOES HERE"
#define LED_PIN_RED 27
#define LED_PIN_GREEN 26

#include <WiFi.h>

void setup() {
    
    Serial.begin(9600);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    delay(100);

    WiFi.begin(WIFI_NAME, WIFI_PASSWORD);

    long long int current_time = millis();
    long long int max_allowed_wait_time_ms = 10 * 1000; // 10s

    pinMode(LED_PIN_RED, OUTPUT);
    pinMode(LED_PIN_GREEN, OUTPUT);

    digitalWrite(LED_PIN_RED, HIGH);
    digitalWrite(LED_PIN_GREEN, LOW);

    while (WiFi.status() != WL_CONNECTED) {

        delay(10*5);

        auto wifi_status = WiFi.status();

        if (wifi_status == WL_NO_SSID_AVAIL || wifi_status == WL_CONNECT_FAILED || wifi_status == WL_CONNECTION_LOST || millis() >= max_allowed_wait_time_ms) {

            Serial.println(wifi_status);

            break;
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
 
        digitalWrite(LED_PIN_GREEN, HIGH);

        delay(10*30);
    }

    digitalWrite(LED_PIN_RED, LOW);
    digitalWrite(LED_PIN_GREEN, LOW);
}

void loop() {

    digitalWrite(LED_PIN_RED, LOW);
}