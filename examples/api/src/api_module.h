/**
 * ApiModule — Exemple pédagogique du routage HTTP avec WebRouter.
 *
 * Objectif : illustrer les trois usages les plus fréquents de WebRouter
 * pour un module métier, sans aucun capteur ni logique complexe :
 *   1. Une route GET qui renvoie simplement du JSON statique/calculé.
 *   2. Une route GET qui lit un paramètre de requête (query string), comme
 *      WebManager le fait déjà en interne pour /api/files/list?path=...
 *      (voir src/services/web_manager/web_manager.cpp, server.arg("path")).
 *   3. Une route POST qui reçoit un corps JSON, le valide, et répond soit
 *      200 (succès) soit 400 (entrée invalide) — démontrant qu'une route
 *      n'est pas obligée de répondre 200 systématiquement.
 *
 * Toutes les routes passent par router.raw() pour récupérer le WebServer&
 * sous-jacent — voir le commentaire détaillé dans api_module.cpp sur la
 * raison de capturer cette référence plutôt que `router` dans les lambdas.
 */

#pragma once
#include "../../../src/core/module.h"

class ApiModule : public Module {
public:
    const char* name() const override { return "Api"; }
    void begin() override;
    // Pas de logique périodique à exécuter ici : loop() reste vide (héritée
    // de Module, qui fournit déjà un no-op par défaut — on pourrait aussi
    // l'omettre complètement, comme dans examples/minimal).

    void registerRoutes(WebRouter& router) override;
};
