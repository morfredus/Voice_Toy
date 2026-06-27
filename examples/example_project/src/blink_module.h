/**
 * BlinkModule — module métier minimal pour ce projet d'exemple : fait
 * clignoter la LED embarquée et expose son état via /api/blink.
 */

#pragma once
#include "../../../src/core/module.h"

class BlinkModule : public Module {
public:
    const char* name() const override { return "Blink"; }
    void begin() override;
    void loop() override;
    void registerRoutes(WebRouter& router) override;

private:
    bool _state = false;
    unsigned long _lastToggle = 0;
};
