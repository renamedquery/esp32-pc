#define WIFI_NAME "ESP32_MASTER_COMPUTE_DEVICE_1"
#define WIFI_PASSWORD "YOUR PASSWORD GOES HERE"
#define LED_PIN 27

#include <WiFi.h>

void setup() {
    
    Serial.begin(9600);

    WiFi.begin(WIFI_NAME, WIFI_PASSWORD);

    pinMode(LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, HIGH);

    while (WiFi.status() != WL_CONNECTED) {

        delay(10);

        auto wifi_status = WiFi.status();

        if (wifi_status == WL_NO_SSID_AVAIL || wifi_status == WL_CONNECT_FAILED || wifi_status == WL_CONNECTION_LOST || wifi_status == WL_DISCONNECTED) {

            Serial.println(wifi_status);

            break;
        }
    }

    digitalWrite(LED_PIN, LOW);
}

void loop() {

    digitalWrite(LED_PIN, LOW);
}