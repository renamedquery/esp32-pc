#define BLUETOOTH_SERIAL_NAME "ESP32_MASTER_COMPUTE_DEVICE_1"
#define LED_PIN 27

#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

void setup() {
    
    Serial.begin(9600);

    SerialBT.connect(BLUETOOTH_SERIAL_NAME);

    pinMode(LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, LOW);
}

void loop() {

    while (SerialBT.available()) {

        SerialBT.read();

        digitalWrite(LED_PIN, HIGH);
    }

    digitalWrite(LED_PIN, LOW);
}