#include <SPI.h>

#define MAX_SPI_RECIEVE_LENGTH 64

const int SLAVE_SELECT_PIN = 27;

char spi_recieve_data[MAX_SPI_RECIEVE_LENGTH] = "";

void setup() {

    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {


}