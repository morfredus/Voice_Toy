
# Future AsyncWebServer integration

Objectif :
- conserver WebServer comme backend par défaut ;
- permettre une future implémentation ESPAsyncWebServer ;
- éviter que les modules métier dépendent du backend HTTP.

Étapes futures :
1. Créer AsyncWebServerAdapter.
2. Faire évoluer WebRouter vers une abstraction indépendante du backend.
3. Ajouter la sélection via WEB_SERVER_BACKEND.
