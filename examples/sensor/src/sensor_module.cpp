#include "sensor_module.h"
#include "../../../src/services/log_manager/log_manager.h"
#include "../../../src/services/config_manager/config_manager.h"
#include "../../../src/api/api_router/api_router.h"
#include <math.h>

void SensorModule::begin() {
    // Charge l'intervalle de lecture depuis la NVS (ConfigManager), avec
    // 5000 ms comme valeur par défaut si la clé n'existe pas encore (premier
    // démarrage). Cette valeur sera relue à l'identique après un redémarrage.
    _intervalMs = config.getInt("sensor_interval_ms", 5000);
    LOG_INFO(name(), "Démarré — intervalle de lecture = %d ms", _intervalMs);
}

float SensorModule::_readSimulatedValue() const {
    // Simule une température ambiante variant doucement entre ~18 et ~26°C,
    // avec un peu de bruit aléatoire pour rendre la courbe moins parfaite.
    // millis() / 10000.0 fait avancer la phase de la sinusoïde lentement.
    float base = 22.0f + 4.0f * sinf(millis() / 10000.0f);
    float noise = (random(-100, 100) / 100.0f) * 0.3f; // +/- 0.3°C de bruit
    return base + noise;
}

void SensorModule::loop() {
    // Ne relit le "capteur" que toutes les _intervalMs millisecondes —
    // jamais de delay() ici, pour ne pas bloquer le reste du framework
    // (WiFi, serveur web, autres modules).
    if (millis() - _lastReadMs < (unsigned long)_intervalMs) return;

    _lastReadMs = millis();
    _lastValue = _readSimulatedValue();

    LOG_INFO(name(), "Nouvelle lecture = %.2f", _lastValue);
}

void SensorModule::registerRoutes(WebRouter& router) {
    // IMPORTANT : on extrait WebServer& depuis router.raw() AVANT de créer
    // les lambdas, et on capture cette référence (&server), jamais le
    // paramètre `router` lui-même. `router` est une référence locale à
    // registerRoutes() (donc temporaire), tandis que `server` désigne
    // l'objet WebServer réel qui vit aussi longtemps que webMgr — capturer
    // `router` par référence dans un std::function stocké à long terme
    // créerait une référence pendante (dangling reference) une fois
    // registerRoutes() terminée.
    WebServer& server = router.raw();

    // Route de lecture : renvoie la dernière valeur lue, en JSON.
    router.get("/api/sensor/value", [this, &server]() {
        String json = "{\"value\":" + String(_lastValue, 2) + "}";
        server.send(200, "application/json", json);
    });

    // Route de configuration : permet de changer l'intervalle de lecture à
    // chaud, ex. GET /api/sensor/interval?ms=2000 — et persiste le choix
    // pour qu'il survive à un redémarrage.
    router.get("/api/sensor/interval", [this, &server]() {
        if (server.hasArg("ms")) {
            int ms = server.arg("ms").toInt();
            if (ms >= 100) { // garde-fou simple contre un intervalle absurde
                _intervalMs = ms;
                config.setInt("sensor_interval_ms", _intervalMs);
            }
        }
        String json = "{\"interval_ms\":" + String(_intervalMs) + "}";
        server.send(200, "application/json", json);
    });
}
