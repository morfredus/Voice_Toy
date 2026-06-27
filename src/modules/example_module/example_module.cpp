#include "example_module.h"
#include "../../services/log_manager/log_manager.h"
#include "../../services/config_manager/config_manager.h"
#include "../../api/api_router/api_router.h"

void ExampleModule::begin() {
    _counter = config.getInt("example_counter", 0);
    LOG_INFO(name(), "Démarré — compteur initial = %d", _counter);
}

void ExampleModule::loop() {
    if (millis() - _lastTick < 10000) return;
    _lastTick = millis();
    _counter++;
    config.setInt("example_counter", _counter);
    LOG_DEBUG(name(), "Tick — compteur = %d", _counter);
}

void ExampleModule::registerRoutes(WebRouter& router) {
    WebServer& server = router.raw();
    router.get("/api/example/counter", [this, &server]() {
        String json = "{\"counter\":" + String(_counter) + "}";
        server.send(200, "application/json", json);
    });
}
