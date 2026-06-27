/**
 * WebManager — Serveur HTTP du framework (généralisé depuis MeteoHub).
 *
 * Fournit automatiquement, sans code métier :
 *   GET  /            — page d'accueil (data/index.html)
 *   GET  /logs        — page de consultation des logs
 *   GET  /api/logs     — tampon de logs en JSON
 *   GET  /files        — gestionnaire de fichiers LittleFS
 *   GET  /api/files/list|download, POST /api/files/upload, DELETE /api/files/delete
 *   GET  /system       — informations système
 *   GET  /api/system    — JSON (SystemInfo::getJson)
 *   GET  /ota           — page de mise à jour firmware
 *   POST /api/ota/update — délègue à OtaManager
 *
 * Tout fichier statique présent dans data/ (LittleFS) et non repris ci-dessus
 * est servi tel quel par le handler générique. Les modules métier ajoutent
 * leurs propres routes via WebRouter (registerRoutes), sans connaître
 * l'implémentation de WebManager.
 */

#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include "../../api/api_router/api_router.h"
#include "i_web_server.h"

// Actuellement basé sur WebServer.
// L\'architecture prépare une future implémentation Async.
class WebManager {
public:
    void begin(uint16_t port = 80);
    void loop();

    WebRouter router();

private:
    void _registerCoreRoutes();
    void _handleStaticFile();   // sert data/<path> depuis LittleFS, fallback 404

    WebServer _server{80};
    bool _started = false;
};

extern WebManager webMgr;
