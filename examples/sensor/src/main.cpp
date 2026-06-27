/**
 * main.cpp — Point d'entrée. Ne contient que l'enregistrement des modules
 * métier du projet ; toute la logique d'infrastructure vit dans App/services.
 */

#include <Arduino.h>
#include "../../../src/core/app.h"
#include "sensor_module.h"

static SensorModule sensorModule;

void setup() {
    Serial.begin(115200);

    app.modules.add(&sensorModule);
    app.begin();
}

void loop() {
    app.loop();
}
