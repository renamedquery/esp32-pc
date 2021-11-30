// to get this to work, you need to change 

#include <Arduino.h>
#include <SPI.h>

#define SS 35
#define MAX_SPI_RECIEVE_LENGTH 64

const int SLAVE_SELECT_PIN = 27;

char spi_recieve_data[MAX_SPI_RECIEVE_LENGTH] = "";

bool data_to_be_interpreted = false;

void spi_interrupt() {
    
    SPI.transfer(9);

    data_to_be_interpreted = true;
}

void setup() {
    
    Serial.begin(9600);

    attachInterrupt(digitalPinToInterrupt(SLAVE_SELECT_PIN), spi_interrupt, LOW);

    SPI.begin(36, 39, 34, 35);
    SPI.setClockDivider(SPI_CLOCK_DIV8);
}

void loop() {

    if (data_to_be_interpreted) { 
        
        Serial.println(data_to_be_interpreted);

        data_to_be_interpreted = false;
    }
}