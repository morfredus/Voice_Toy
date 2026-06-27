#include "blink_module.h"
#include "../../../src/services/log_manager/log_manager.h"
#include "../../../src/api/api_router/api_router.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

void BlinkModule::begin() {
    pinMode(LED_BUILTIN, OUTPUT);
    LOG_INFO(name(), "Démarré");
}

void BlinkModule::loop() {
    if (millis() - _lastToggle < 1000) return;
    _lastToggle = millis();
    _state = !_state;
    digitalWrite(LED_BUILTIN, _state);
}

void BlinkModule::registerRoutes(WebRouter& router) {
    WebServer& server = router.raw();
    router.get("/api/blink", [this, &server]() {
        server.send(200, "application/json", _state ? "{\"on\":true}" : "{\"on\":false}");
    });
}
