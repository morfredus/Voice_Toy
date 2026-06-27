# Architecture — ESP32-Foundation

Pour le guide pratique pas à pas (renommer le projet, compiler, téléverser,
créer un module), voir [docs/INTEGRATION_GUIDE.md](INTEGRATION_GUIDE.md).
Ce document-ci reste centré sur l'architecture et les choix de conception.

## Origine

ESP32-Foundation fusionne et généralise les briques transversales de deux
projets : **Gateway Lab V1** (scanner réseau) et **MeteoHub-S3** (station
météo). L'analyse de ces deux projets a montré :

- **Briques transversales identiques en intention, divergentes en
  implémentation** : WiFiManager (portail captif + NVS chez Gateway Lab,
  liste statique chez MeteoHub), logs (macros + buffer chez MeteoHub,
  `Log::i/w/e/d` chez Gateway Lab), serveur web (`WebServer` synchrone chez
  Gateway Lab, `ESPAsyncWebServer` chez MeteoHub), OTA (identique dans
  l'esprit, dupliqué dans le code), système info (JSON ad hoc dans les deux).
- **Modules métier spécifiques**, à ne jamais faire remonter dans le
  framework : scanners réseau (ICMP, mDNS, SNMP, SSDP...) côté Gateway Lab ;
  capteurs, prévisions météo, écran OLED côté MeteoHub.
- **Incohérences corrigées** : mélange `String`/`std::string` selon le
  fichier, gestion NVS dupliquée par module au lieu d'un service partagé,
  génération de pages web par deux pipelines incompatibles (headers PROGMEM
  vs tableaux d'octets).

## Principe directeur

> Le framework (`core/` + `services/`) ne connaît jamais le métier.
> Les modules (`modules/`) ne connaissent jamais l'implémentation des
> services — uniquement leurs interfaces publiques.

```
core/      → App, Module, ModuleManager — orchestration uniquement
services/  → WiFi, Web, OTA, Storage, Config, Log, SystemInfo, mDNS, Time
api/       → WebRouter — façade de routage exposée aux modules
modules/   → code métier, 100% optionnel et interchangeable
```

## Flux de démarrage (`App::begin`)

1. `ConfigManager::begin` — namespace NVS partagé
2. `StorageManager::begin` — montage LittleFS
3. `WiFiManager::begin` — connexion ou portail captif
4. Au premier WiFi OK (callback) : `MdnsManager`, `OtaManager`, `TimeManager`
5. `WebManager::begin` — routes core (`/`, `/logs`, `/files`, `/system`, `/ota`)
6. `ModuleManager::registerAllRoutes` — routes métier via `WebRouter`
7. `ModuleManager::beginAll` — initialisation des modules métier

`App::loop` appelle ensuite `wifiMgr.loop()`, `otaMgr.loop()`,
`webMgr.loop()`, puis `modules.loopAll()` à chaque itération.

Trois exemples progressifs illustrent ce flux pour un module métier, du plus
simple au plus complet : `examples/minimal/`, `examples/sensor/` et
`examples/api/` (voir le détail de chacun dans
[docs/INTEGRATION_GUIDE.md](INTEGRATION_GUIDE.md#créer-son-premier-module-pas-à-pas)).
`src/modules/boot_log/` est un quatrième exemple, intégré directement au
dépôt : un module optionnel (désactivé par défaut, `ENABLE_BOOT_LOG`)
illustrant comment un module métier peut rester totalement découplé du
reste du framework — au point d'être désactivable d'une seule ligne, ou
supprimable sans impact ailleurs (voir [docs/BOOT_LOG.md](BOOT_LOG.md)).
Le nom affiché du projet (`PROJECT_NAME`) se propage de la même façon à
travers tout le framework, jusqu'à l'interface web — voir
[docs/INTEGRATION_GUIDE.md](INTEGRATION_GUIDE.md#renommer-le-projet) pour le
mécanisme exact de renommage.

## Pourquoi WebServer (synchrone) et non ESPAsyncWebServer ?

Gateway Lab utilisait `WebServer` (synchrone, intégré à arduino-esp32),
MeteoHub `ESPAsyncWebServer` (dépendance externe). Le framework retient
`WebServer` : zéro dépendance supplémentaire, suffisant pour le trafic
typique d'une interface de configuration/monitoring, et upload de fichiers
déjà supporté nativement (`HTTPUpload`). Un projet ayant des besoins de
streaming intensif peut remplacer `WebManager` sans toucher au reste du
framework, grâce au découplage via `WebRouter`.


## Préparation du support AsyncWebServer

La version actuelle repose sur WebServer (synchrone).

Le projet prépare l'introduction future d'un backend basé sur
ESPAsyncWebServer sans modifier l'API publique des modules.

Le backend synchrone reste le choix par défaut afin de privilégier :
- la simplicité ;
- la stabilité ;
- un nombre réduit de dépendances ;
- une prise en main plus facile pour les débutants.
