/**
 * ExampleModule — Module métier minimal de démonstration.
 *
 * Montre comment un module utilise les services du framework (LogManager,
 * ConfigManager) et ajoute sa propre route HTTP via WebRouter, sans jamais
 * connaître WebServer ni WebManager directement.
 */

#pragma once
#include "../../core/module.h"

class ExampleModule : public Module {
public:
    const char* name() const override { return "Example"; }
    void begin() override;
    void loop() override;
    void registerRoutes(WebRouter& router) override;

private:
    unsigned long _lastTick = 0;
    int _counter = 0;
};
