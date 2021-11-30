#include <ESP32Lib.h>
#include <SoftwareSerial.h> // https://github.com/plerup/espsoftwareserial

#define MAX_SERIAL_RECIEVE_LENGTH 64
#define LED_PIN 27
#define SER_RX_PIN 26
#define SER_TX_PIN 25

const int SLAVE_SELECT_PIN = 35;

char serial_recieve_data[MAX_SERIAL_RECIEVE_LENGTH] = "";

SoftwareSerial ser;

void setup() {
    
    Serial.begin(9600);

    pinMode(LED_PIN, OUTPUT);

    pinMode(SER_RX_PIN, INPUT);
    pinMode(SER_TX_PIN, OUTPUT);

    digitalWrite(LED_PIN, HIGH);

    delay(1000);

    digitalWrite(LED_PIN, LOW);

    ser.begin(9600, SWSERIAL_8N1, SER_RX_PIN, SER_TX_PIN, false);

    Serial.println("SLAVE DEVICE FINISHED INITIALIZATION");
}

void loop() {

    int available_serial = ser.available();

    if (available_serial > 0) {

        digitalWrite(LED_PIN, HIGH);

        delay(1000);

        ser.flush();

        digitalWrite(LED_PIN, LOW);
    }
}