#include <Arduino.h>
#include "../../../src/core/app.h"
#include "blink_module.h"

static BlinkModule blinkModule;

void setup() {
    Serial.begin(115200);
    app.modules.add(&blinkModule);
    app.begin();
}

void loop() {
    app.loop();
}
