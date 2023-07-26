#include <Wire.h>
#include <ILB.h>

ILB ilb;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.printf("\nIGA Test\n");

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