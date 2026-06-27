/**
 * main.cpp — Point d'entrée. Ne contient que l'enregistrement des modules
 * métier du projet ; toute la logique d'infrastructure vit dans App/services.
 */

#include <Arduino.h>
#include "core/app.h"
#include "project_config.h"
#include "modules/voice_toy/voice_toy_module.h"
#ifdef ENABLE_BOOT_LOG
#include "modules/boot_log/boot_log.h"
#endif

static VoiceToyModule voiceToyModule;

void setup() {
    Serial.begin(115200);

    app.modules.add(&voiceToyModule);
#ifdef ENABLE_BOOT_LOG
    app.modules.add(&bootLogModule);
#endif
    app.begin();
}

void loop() {
    app.loop();
}
