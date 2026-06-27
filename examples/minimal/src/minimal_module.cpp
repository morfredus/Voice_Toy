#include "minimal_module.h"
#include "../../../src/services/log_manager/log_manager.h"

void MinimalModule::begin() {
    // LOG_INFO(tag, format, ...) fonctionne comme printf et envoie le
    // message à la fois sur le port série et dans le tampon JSON consultable
    // via /api/logs (et visible sur la page web /logs).
    LOG_INFO(name(), "MinimalModule démarré — exemple le plus simple du framework");
}

void MinimalModule::loop() {
    // Pattern classique pour exécuter une action périodique sans bloquer :
    // comparer millis() à la dernière exécution plutôt que d'utiliser
    // delay() (qui geler toute la boucle, y compris le WiFi et le serveur web).
    const unsigned long intervalMs = 5000;
    if (millis() - _lastTick < intervalMs) return;

    _lastTick = millis();
    _tickCount++;

    // LOG_DEBUG n'est compilé que si LOG_LEVEL >= 4 (voir project_config.h) —
    // pratique pour des messages très fréquents qu'on ne veut pas voir en
    // temps normal, sans avoir à les supprimer du code.
    LOG_DEBUG(name(), "Tick #%lu (toutes les %lu ms)", _tickCount, intervalMs);
}
