/**
 * MinimalModule — Le module le plus simple possible.
 *
 * Objectif pédagogique : montrer le strict minimum requis pour créer un
 * module métier au-dessus du framework ESP32-Foundation, sans capteur,
 * sans route HTTP, sans réglage persistant. C'est le point de départ à
 * copier-coller avant d'ajouter quoi que ce soit de plus complexe — voir
 * examples/sensor (capteur + API) et examples/api (routage HTTP avancé)
 * pour la suite logique.
 *
 * Un module n'est qu'une classe qui hérite de Module (src/core/module.h)
 * et qui redéfinit tout ou partie de ses méthodes virtuelles :
 *   - name()           : nom court utilisé dans les logs (obligatoire)
 *   - begin()          : initialisation, appelée une seule fois au démarrage
 *   - loop()            : appelée en boucle, ne doit jamais bloquer (pas de
 *                         delay() long : utiliser millis() comme ci-dessous)
 *   - registerRoutes() : facultatif — ce module n'en a pas besoin, donc on
 *                         ne le redéfinit pas du tout (le comportement par
 *                         défaut fourni par Module est un no-op).
 */

#pragma once
#include "../../../src/core/module.h"

class MinimalModule : public Module {
public:
    // name() est utilisé comme "tag" dans les logs (LOG_INFO(name(), ...))
    // et doit être court et unique parmi les modules du projet.
    const char* name() const override { return "Minimal"; }

    // begin() est appelée une seule fois par App::begin(), après que tous
    // les services (WiFi, stockage, etc.) soient déjà prêts. C'est l'endroit
    // où initialiser des broches, lire un réglage de démarrage, etc.
    void begin() override;

    // loop() est appelée à chaque itération de App::loop() (donc très
    // souvent, plusieurs fois par seconde) — elle ne doit jamais bloquer
    // longtemps. Ici, on se contente d'afficher un message toutes les
    // quelques secondes, en mesurant le temps écoulé avec millis().
    void loop() override;

    // Pas de registerRoutes() ici : ce module ne fournit aucune route HTTP.
    // La version par défaut héritée de Module ne fait rien (no-op), donc
    // il n'y a rien à écrire de plus.

private:
    // Horodatage (millis()) du dernier message affiché, pour ne logguer
    // qu'une fois toutes les N millisecondes sans bloquer loop().
    unsigned long _lastTick = 0;

    // Compteur trivial juste pour avoir quelque chose à observer dans
    // le moniteur série / la page /logs.
    unsigned long _tickCount = 0;
};
