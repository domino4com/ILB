#include <ArduinoJson.h>
#include <Wire.h>
#ifndef I2C_SDA
#define I2C_SDA SDA
#endif
#ifndef I2C_SCL
#define I2C_SCL SCL
#endif
#include <ILB.h>

ILB ilb;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.printf("\ILB Test\n");

    Wire.setPins(I2C_SDA, I2C_SCL);
    Wire.begin();

    if (ilb.begin()) {
        Serial.println("ILB sensor initialized successfully.");
    } else {
        Serial.println("Failed to initialize ILB sensor!");
        exit(0);
    }
}

void loop() {
    float UVI, Lux;

    if (ilb.getData(UVI, Lux)) {
        Serial.printf("UVI: %.2f, Lux: %.2f\n", UVI, Lux);
    } else {
        Serial.println("Failed to ILB data.");
    }

    delay(1000);
}