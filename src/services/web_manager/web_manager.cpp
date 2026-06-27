#include "web_manager.h"
#include "../ota_manager/ota_manager.h"
#include "../system_info/system_info.h"
#include "../log_manager/log_manager.h"
#include "../storage_manager/storage_manager.h"
#include "project_config.h"
#include <LittleFS.h>

static const char* TAG = "Web";

WebManager webMgr;

static String _contentType(const String& path) {
    if (path.endsWith(".html")) return "text/html";
    if (path.endsWith(".css"))  return "text/css";
    if (path.endsWith(".js"))   return "application/javascript";
    if (path.endsWith(".json")) return "application/json";
    if (path.endsWith(".png"))  return "image/png";
    if (path.endsWith(".svg"))  return "image/svg+xml";
    return "text/plain";
}

WebRouter WebManager::router() { return WebRouter(_server); }

void WebManager::begin(uint16_t port) {
    if (_started) return;
    _started = true;

    _registerCoreRoutes();
    otaMgr.registerRoutes(_server);
    storage.begin();

    _server.onNotFound([this]() { _handleStaticFile(); });

    _server.begin();
    LOG_INFO(TAG, "Serveur web démarré sur le port %u", (unsigned)port);
}

void WebManager::loop() {
    _server.handleClient();
}

void WebManager::_registerCoreRoutes() {
    // --- Page d'accueil et pages statiques connues servies depuis LittleFS ---
    _server.on("/", HTTP_GET, [this]() {
        if (storage.exists("/index.html")) {
            _server.send(200, "text/html", storage.readFile("/index.html"));
        } else {
            _server.send(200, "text/html", String("<h1>") + PROJECT_NAME + "</h1><p>Aucune page d'accueil (data/index.html absent).</p>");
        }
    });

    _server.on("/logs", HTTP_GET, [this]() {
        if (storage.exists("/logs.html")) _server.send(200, "text/html", storage.readFile("/logs.html"));
        else _server.send(404, "text/plain", "logs.html absent de data/");
    });
    _server.on("/files", HTTP_GET, [this]() {
        if (storage.exists("/files.html")) _server.send(200, "text/html", storage.readFile("/files.html"));
        else _server.send(404, "text/plain", "files.html absent de data/");
    });
    _server.on("/system", HTTP_GET, [this]() {
        if (storage.exists("/system.html")) _server.send(200, "text/html", storage.readFile("/system.html"));
        else _server.send(404, "text/plain", "system.html absent de data/");
    });
    _server.on("/ota", HTTP_GET, [this]() {
        if (storage.exists("/ota.html")) _server.send(200, "text/html", storage.readFile("/ota.html"));
        else _server.send(404, "text/plain", "ota.html absent de data/");
    });

    // --- API logs ---
    _server.on("/api/logs", HTTP_GET, [this]() {
        _server.sendHeader("Cache-Control", "no-cache");
        _server.send(200, "application/json", logMgr.toJson());
    });
    _server.on("/api/logs", HTTP_DELETE, [this]() {
        logMgr.clear();
        _server.send(200, "application/json", "{\"status\":\"ok\"}");
    });

    // --- API système ---
    _server.on("/api/system", HTTP_GET, [this]() {
        _server.sendHeader("Cache-Control", "no-cache");
        _server.send(200, "application/json", systemInfo.getJson());
    });

    // --- API fichiers (LittleFS) ---
    _server.on("/api/files/list", HTTP_GET, [this]() {
        String path = _server.hasArg("path") ? _server.arg("path") : "/";
        auto files = storage.listFiles(path);
        String json = "[";
        for (size_t i = 0; i < files.size(); i++) {
            if (i) json += ",";
            json += "{\"name\":\"" + files[i].name + "\",\"size\":" + files[i].size +
                     ",\"isDir\":" + (files[i].isDir ? "true" : "false") + "}";
        }
        json += "]";
        _server.send(200, "application/json", json);
    });

    _server.on("/api/files/download", HTTP_GET, [this]() {
        String path = _server.arg("path");
        if (path.isEmpty() || path.indexOf("..") != -1 || !storage.exists(path)) {
            _server.send(404, "text/plain", "Fichier introuvable");
            return;
        }
        File f = LittleFS.open(path, "r");
        _server.streamFile(f, _contentType(path));
        f.close();
    });

    _server.on("/api/files/delete", HTTP_DELETE, [this]() {
        String path = _server.arg("path");
        if (path.isEmpty() || path == "/" || path.indexOf("..") != -1) {
            _server.send(403, "application/json", "{\"error\":\"chemin invalide\"}");
            return;
        }
        bool ok = storage.deleteFile(path);
        _server.send(ok ? 200 : 404, "application/json",
                     ok ? "{\"status\":\"ok\"}" : "{\"error\":\"introuvable\"}");
    });

    _server.on("/api/files/upload", HTTP_POST,
        [this]() { _server.send(200, "application/json", "{\"status\":\"ok\"}"); },
        [this]() {
            static File uploadFile;
            HTTPUpload& upload = _server.upload();
            if (upload.status == UPLOAD_FILE_START) {
                String filename = upload.filename;
                if (!filename.startsWith("/")) filename = "/" + filename;
                if (filename.indexOf("..") != -1) return;
                uploadFile = LittleFS.open(filename, "w");
            } else if (upload.status == UPLOAD_FILE_WRITE) {
                if (uploadFile) uploadFile.write(upload.buf, upload.currentSize);
            } else if (upload.status == UPLOAD_FILE_END) {
                if (uploadFile) uploadFile.close();
            }
        });
}

void WebManager::_handleStaticFile() {
    String path = _server.uri();
    if (path.indexOf("..") != -1) {
        _server.send(403, "text/plain", "Forbidden");
        return;
    }
    if (storage.exists(path)) {
        File f = LittleFS.open(path, "r");
        _server.streamFile(f, _contentType(path));
        f.close();
        return;
    }
    _server.send(404, "text/plain", "Not Found");
}
