/**
 * SensorModule — Exemple de module exposant une "lecture de capteur" via API.
 *
 * Objectif pédagogique : montrer le cas le plus courant d'un module métier
 * réel — lire périodiquement une valeur (capteur de température, humidité,
 * etc.), la conserver en mémoire, et l'exposer aux clients web via une route
 * JSON (GET /api/sensor/value), tout en rendant l'intervalle de lecture
 * configurable et persistant entre redémarrages (via ConfigManager/NVS).
 *
 * Choix pédagogique — capteur simulé :
 *   Ce framework ne suppose aucun matériel précis. Pour que cet exemple
 *   compile et fonctionne sur n'importe quelle carte ESP32, sans bibliothèque
 *   de capteur réelle, la "lecture" est simulée dans le .cpp avec une
 *   fonction sinusoïdale basée sur millis() (valeur qui varie doucement dans
 *   le temps, comme une température ambiante). Pour brancher un vrai capteur
 *   (ex : DHT22, BME280...) :
 *     1. Ajouter sa bibliothèque dans lib_deps (platformio.ini) ;
 *     2. Initialiser le capteur dans begin() (ex : dht.begin()) ;
 *     3. Remplacer l'appel à _readSimulatedValue() dans loop() par un appel
 *        réel à la bibliothèque (ex : dht.readTemperature()) ;
 *     4. Le reste du module (config, log, route HTTP) ne change pas.
 */

#pragma once
#include "../../../src/core/module.h"

class SensorModule : public Module {
public:
    const char* name() const override { return "Sensor"; }
    void begin() override;
    void loop() override;
    void registerRoutes(WebRouter& router) override;

private:
    // Simule une lecture de capteur (sinusoïde lente basée sur millis()).
    // À remplacer par un appel à une vraie bibliothèque de capteur si besoin.
    float _readSimulatedValue() const;

    // Dernière valeur lue, conservée en mémoire pour être renvoyée
    // instantanément par la route HTTP sans relire le capteur à chaque requête.
    float _lastValue = 0.0f;

    // Horodatage (millis()) de la dernière lecture.
    unsigned long _lastReadMs = 0;

    // Intervalle de lecture en millisecondes, chargé depuis ConfigManager
    // (clé "sensor_interval_ms") au démarrage — modifiable et persistant.
    int _intervalMs = 5000;
};
