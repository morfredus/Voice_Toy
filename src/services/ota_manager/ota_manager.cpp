#include "ota_manager.h"
#include <ArduinoOTA.h>
#include <Update.h>
#include "project_config.h"
#include "../log_manager/log_manager.h"

static const char* TAG = "OTA";

OtaManager otaMgr;

void OtaManager::begin(const char* hostname) {
#ifdef ENABLE_OTA
    ArduinoOTA.setHostname(hostname);

    if (!_callbacksRegistered) {
        ArduinoOTA.onStart([]() { LOG_INFO(TAG, "ArduinoOTA: réception du firmware..."); });
        ArduinoOTA.onEnd([]()   { LOG_INFO(TAG, "ArduinoOTA: installation terminée, redémarrage..."); });
        ArduinoOTA.onProgress([](unsigned int p, unsigned int t) {
            LOG_DEBUG(TAG, "Progression : %u%%", t ? (p * 100 / t) : 0);
        });
        ArduinoOTA.onError([](ota_error_t err) {
            LOG_ERROR(TAG, "Erreur OTA [%u]", (unsigned)err);
        });
        _callbacksRegistered = true;
    }

    ArduinoOTA.begin();
    LOG_INFO(TAG, "ArduinoOTA actif (nom réseau : %s)", hostname);
#else
    (void)hostname;
#endif
}

void OtaManager::loop() {
#ifdef ENABLE_OTA
    ArduinoOTA.handle();
#endif
}

void OtaManager::registerRoutes(WebServer& server) {
    server.on("/api/ota/update", HTTP_POST,
        [&server]() {
            server.sendHeader("Connection", "close");
            server.send(200, "application/json",
                        Update.hasError() ? "{\"ok\":false}" : "{\"ok\":true}");
            delay(500);
            ESP.restart();
        },
        [&server]() {
            HTTPUpload& upload = server.upload();
            if (upload.status == UPLOAD_FILE_START) {
                LOG_INFO(TAG, "Web OTA: début de réception — %s", upload.filename.c_str());
                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
            } else if (upload.status == UPLOAD_FILE_WRITE) {
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                    Update.printError(Serial);
            } else if (upload.status == UPLOAD_FILE_END) {
                if (Update.end(true)) {
                    LOG_INFO(TAG, "Web OTA: %u octets écrits — succès", upload.totalSize);
                } else {
                    Update.printError(Serial);
                }
            }
        }
    );
}
