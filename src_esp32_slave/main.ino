#define WIFI_NAME "ESP32_MASTER_COMPUTE_DEVICE_1"
#define WIFI_PASSWORD "YOUR PASSWORD GOES HERE"
#define LED_PIN 27

#include <WiFi.h>

void setup() {
    
    Serial.begin(9600);

    pinMode(LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, LOW);
}

void loop() {

    digitalWrite(LED_PIN, LOW);
}