/**
 * main.cpp — Point d'entrée. Ne contient que l'enregistrement des modules
 * métier du projet ; toute la logique d'infrastructure vit dans App/services.
 */

#include <Arduino.h>
#include "../../../src/core/app.h"
#include "minimal_module.h"

static MinimalModule minimalModule;

void setup() {
    Serial.begin(115200);

    // Enregistrer le module avant app.begin() : App appelle ensuite
    // automatiquement registerRoutes() puis begin() sur chaque module ajouté.
    app.modules.add(&minimalModule);
    app.begin();
}

void loop() {
    // App::loop() appelle déjà loop() sur tous les services (WiFi, web...)
    // puis sur tous les modules enregistrés (ici, MinimalModule::loop()).
    app.loop();
}
