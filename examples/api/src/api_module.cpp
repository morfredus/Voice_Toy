#include "api_module.h"
#include "../../../src/services/log_manager/log_manager.h"
#include "../../../src/api/api_router/api_router.h"
#include <ArduinoJson.h>

void ApiModule::begin() {
    LOG_INFO(name(), "Démarré — routes disponibles : "
             "/api/hello, /api/greet?name=..., POST /api/echo");
}

void ApiModule::registerRoutes(WebRouter& router) {
    // IMPORTANT — pattern de capture : on extrait la référence WebServer&
    // une seule fois ici, AVANT de définir les lambdas, et on la capture
    // ensuite par référence (&server) dans chaque lambda. On ne capture
    // JAMAIS `router` lui-même par référence dans une lambda destinée à
    // vivre longtemps : `router` est un paramètre local à cette fonction
    // (une référence vers un WebRouter temporaire construit dans
    // WebManager::router()), donc une fois registerRoutes() terminée, toute
    // référence vers `router` devient pendante (dangling). `server`, en
    // revanche, désigne l'objet WebServer réel détenu par webMgr, qui
    // survit pendant toute la durée de vie du programme.
    WebServer& server = router.raw();

    // 1) Route GET simple : renvoie un objet JSON fixe. C'est le cas le
    //    plus basique — utile pour un "ping" ou une route de diagnostic.
    router.get("/api/hello", [this, &server]() {
        server.send(200, "application/json", "{\"message\":\"bonjour\"}");
    });

    // 2) Route GET avec paramètre de requête (query string), ex :
    //    GET /api/greet?name=Alice
    //    On récupère le paramètre via server.arg("name"), exactement comme
    //    le fait WebManager en interne pour /api/files/list?path=... et
    //    /api/files/delete?path=... (voir web_manager.cpp).
    router.get("/api/greet", [this, &server]() {
        // server.hasArg() vérifie la présence du paramètre avant de le lire,
        // pour proposer une valeur par défaut si le client ne l'a pas fourni.
        String nom = server.hasArg("name") ? server.arg("name") : "inconnu";

        JsonDocument doc;
        doc["greeting"] = "Bonjour, " + nom + " !";
        String json;
        serializeJson(doc, json);
        server.send(200, "application/json", json);
    });

    // 3) Route POST avec corps JSON : on lit server.arg("plain") (le corps
    //    brut de la requête HTTP, tel qu'envoyé par le client), on le parse
    //    avec ArduinoJson, on valide son contenu, puis on répond soit 200
    //    (succès) soit 400 (entrée invalide) — utile pour apprendre à
    //    renvoyer un code d'erreur autre que 200, comme le fait par exemple
    //    /api/files/delete (403/404) dans web_manager.cpp.
    //
    //    Exemple d'appel :
    //      curl -X POST http://<ip>/api/echo -d '{"text":"salut"}'
    router.post("/api/echo", [this, &server]() {
        String body = server.arg("plain");

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, body);

        // Entrée invalide si le JSON est mal formé, ou si le champ attendu
        // "text" est absent : on répond alors 400 Bad Request avec un
        // message d'erreur explicite, plutôt que de planter ou de renvoyer
        // un 200 trompeur.
        if (err || !doc["text"].is<const char*>()) {
            server.send(400, "application/json",
                        "{\"error\":\"champ JSON 'text' (chaîne) requis\"}");
            return;
        }

        String texte = doc["text"].as<String>();
        LOG_INFO(name(), "Echo reçu : %s", texte.c_str());

        JsonDocument reponse;
        reponse["echo"] = texte;
        reponse["length"] = texte.length();
        String json;
        serializeJson(reponse, json);
        server.send(200, "application/json", json);
    });
}
