#include <Arduino.h>

#include "CarrierManager.h"

#include "arduino_secrets.h"


CarrierManager carrier;


void setup() {
    Serial.begin(9600);
    delay(5000);
    carrier.enableEnvironmentSensorUpdates();

    carrier.begin();
}

void loop() {
    carrier.loop();
    delay(1000);
}
