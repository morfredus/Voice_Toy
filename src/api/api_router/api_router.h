/**
 * WebRouter — Façade de routage HTTP exposée aux modules métier.
 *
 * Découple les modules de l'implémentation concrète du serveur HTTP
 * (WebServer d'Arduino-ESP32) : un module appelle router.on(...), sans
 * jamais inclure <WebServer.h> ni connaître WebManager.
 */

#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <functional>

class WebRouter {
public:
    explicit WebRouter(WebServer& server) : _server(server) {}

    using Handler = std::function<void()>;

    void on(const String& path, HTTPMethod method, Handler handler) {
        _server.on(path, method, handler);
    }

    void get(const String& path, Handler handler)  { on(path, HTTP_GET, handler); }
    void post(const String& path, Handler handler) { on(path, HTTP_POST, handler); }

    WebServer& raw() { return _server; }

private:
    WebServer& _server;
};
