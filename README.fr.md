# ESP32-Foundation

Framework applicatif générique pour projets ESP32 (PlatformIO / Arduino),
extrait et généralisé à partir de **Gateway Lab V1** et **MeteoHub-S3**.

*Read this in English: [README.md](README.md)*

ESP32 Foundation est un framework d'applications réutilisable, issu de
projets ESP32 concrets tels que Gateway Lab et MeteoHub. Il fournit des
services communs comme la gestion du Wi-Fi, les mises à jour OTA, la
journalisation, le stockage, une interface web, le routage API et des outils
de compilation, permettant ainsi aux nouveaux projets de se concentrer sur
les fonctionnalités spécifiques à leur application.

Ce framework n'a pas été conçu en premier. Il a été extrait ultérieurement
de plusieurs projets ESP32 après avoir identifié les composants qui étaient
fréquemment réutilisés.

Le framework fournit l'infrastructure commune à tout projet ESP32 connecté
(WiFi, serveur web, logs, OTA, stockage, configuration persistante,
informations système, mDNS, heure réseau) à travers un système de **modules**
optionnels : le framework ne connaît jamais le code métier, le métier ne
connaît jamais l'implémentation des services.

## Structure

```
ESP32-Foundation/
├── platformio.ini
├── README.md                       version anglaise
├── README.fr.md                    ce fichier
├── CHANGELOG.md                    historique des versions
├── VERSION
├── web_src/                         sources de l'interface web (HTML/CSS/JS), à modifier ici
│       debug.html                  page /debug, servie seulement si ENABLE_BOOT_LOG
│       files.html
│       index.html
│       logs.html
│       menu.js
│       ota.html
│       style.css
│       system.html
├── data/                            généré par tools/minify_web.py depuis web_src/, ne pas modifier - servi depuis LittleFS
├── docs/
│       ARCHITECTURE.md
│       BOOT_LOG.md                 module optionnel BootLog (journal de redémarrage)
│       INTEGRATION_GUIDE.md
│       WIFI_SETUP.md              premier démarrage, secrets.h, portail de configuration
├── examples/
│   ├── api/                        exemple de routage API (GET/POST/JSON)
│   │       platformio.ini
│   │       include/project_config.h
│   │       src/api_module.cpp, api_module.h, main.cpp
│   ├── example_project/            exemple historique (module "Blink")
│   │       platformio.ini
│   │       include/project_config.h
│   │       src/blink_module.cpp, blink_module.h, main.cpp
│   ├── minimal/                    exemple le plus simple (cycle de vie seul)
│   │       platformio.ini
│   │       include/project_config.h
│   │       src/main.cpp, minimal_module.cpp, minimal_module.h
│   └── sensor/                     exemple capteur simulé + config persistante
│           platformio.ini
│           include/project_config.h
│           src/main.cpp, sensor_module.cpp, sensor_module.h
├── include/
│       board_config.h              brochage générique
│       project_config.h            réglages globaux du framework
│       secrets_example.h           modèle pour include/secrets.h (gitignored)
├── src/
│   │   main.cpp                    point d'entrée du projet racine
│   ├── api/
│   │   └── api_router/
│   │           api_router.h        WebRouter
│   ├── core/
│   │       app.cpp, app.h          orchestrateur App
│   │       module.h                classe Module
│   │       module_manager.h        ModuleManager
│   ├── modules/
│   │   ├── example_module/
│   │   │       example_module.cpp, example_module.h
│   │   └── boot_log/                optionnel, désactivé par défaut — voir docs/BOOT_LOG.md
│   │           boot_log.cpp, boot_log.h
│   └── services/
│       ├── config_manager/         config_manager.cpp, config_manager.h
│       ├── log_manager/            log_manager.cpp, log_manager.h
│       ├── mdns_manager/           mdns_manager.cpp, mdns_manager.h
│       ├── ota_manager/            ota_manager.cpp, ota_manager.h
│       ├── storage_manager/        storage_manager.cpp, storage_manager.h
│       ├── system_info/            system_info.cpp, system_info.h
│       ├── time_manager/           time_manager.cpp, time_manager.h
│       ├── web_manager/            web_manager.cpp, web_manager.h
│       └── wifi_manager/           wifi_manager.cpp, wifi_manager.h
└── tools/
        build_info.py
        minify_web.py
        package_web.py
        release.py
        version_generator.py
```

## Démarrage rapide

```bash
cp include/secrets_example.h include/secrets.h   # WiFi de développement (optionnel)
pio run                                            # compile le firmware
pio run --target uploadfs                          # flashe data/ (interface web, générée depuis web_src/)
pio run --target upload                            # flashe le firmware
```

Au premier démarrage, si aucun réseau WiFi n'est encore connu (pas de
`secrets.h` et rien d'enregistré), l'ESP32 démarre un point d'accès de
configuration permettant de saisir le SSID et le mot de passe depuis un
téléphone ou un ordinateur. Voir
[docs/WIFI_SETUP.md](docs/WIFI_SETUP.md) pour le déroulé complet.

Important : exécuter `pio run -t uploadfs` au moins une fois après le tout
premier flash (et à chaque modification de `web_src/`) — sans cette étape,
l'interface web LittleFS est vide et toute page renvoie une page blanche ou
une erreur 404, même si le firmware fonctionne correctement. Voir l'explication
détaillée dans
[docs/INTEGRATION_GUIDE.md](docs/INTEGRATION_GUIDE.md#téléverser-linterface-web-littlefs--éviter-la-page-blanche-au-premier-démarrage).

Pour changer le nom du projet (logs, `/api/system`, interface web), voir la
section [Renommer le projet](docs/INTEGRATION_GUIDE.md#renommer-le-projet)
du guide d'intégration.

Voir [docs/INTEGRATION_GUIDE.md](docs/INTEGRATION_GUIDE.md) pour démarrer un
nouveau projet, et [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) pour le détail
des choix de conception et leur origine (Gateway Lab / MeteoHub).

## Exemples fournis

| Exemple | Description |
|---|---|
| `examples/example_project/` | Module "Blink" minimal pilotant la LED embarquée — exemple historique de référence. |
| `examples/minimal/` | Le module le plus simple possible : cycle de vie `begin()/loop()` uniquement, sans capteur ni route HTTP. À lire en premier. |
| `examples/sensor/` | Lecture périodique d'une valeur simulée, intervalle de lecture configurable et persistant, route HTTP `GET /api/sensor/value`. |
| `examples/api/` | Trois routes HTTP de démonstration via `WebRouter` : GET simple, GET avec paramètre de requête, POST avec corps JSON et validation. |

Chaque exemple est un projet PlatformIO autonome (voir son propre
`platformio.ini`, qui référence le framework via `lib_extra_dirs = ../../`).
Le guide complet, pas à pas, pour créer son propre module à partir de ces
exemples se trouve dans
[docs/INTEGRATION_GUIDE.md](docs/INTEGRATION_GUIDE.md#créer-son-premier-module-pas-à-pas).

## Services fournis

| Service          | Rôle                                                         |
|-------------------|---------------------------------------------------------------|
| `wifiMgr`         | Multi-réseaux NVS, portail captif de secours, reconnexion auto |
| `webMgr`          | Serveur HTTP : `/`, `/logs`, `/files`, `/system`, `/ota`        |
| `otaMgr`          | Mise à jour firmware (ArduinoOTA + upload web)                |
| `storage`         | Fichiers LittleFS (lecture/écriture/liste/suppression)         |
| `config`          | Réglages persistants clé/valeur (NVS)                          |
| `logMgr`          | Logs série + tampon JSON (`LOG_INFO`, `LOG_ERROR`, ...)         |
| `systemInfo`      | État système en JSON (heap, version, WiFi, build...)           |
| `mdnsMgr`         | Résolution `http://<hostname>.local`                            |
| `timeMgr`         | Synchronisation NTP                                              |

Aucune dépendance métier n'est imposée : chaque service est utilisable
indépendamment des autres.

## Modules optionnels

| Module | Rôle |
|---|---|
| `exampleModule` | Module de démonstration minimal (`src/modules/example_module/`), toujours actif. |
| `bootLogModule` | Journal de redémarrage (raison du reset, derniers logs, instantané système avant crash). **Désactivé par défaut**, à activer avec `#define ENABLE_BOOT_LOG` dans `include/project_config.h`. Le lien de menu `/debug` apparaît automatiquement (`web_src/menu.js` sonde `GET /api/bootlog`) — aucune édition du menu nécessaire dans un sens comme dans l'autre. Exemple concret de module entièrement supprimable — voir [docs/BOOT_LOG.md](docs/BOOT_LOG.md). |

## Captures d'écran

| Page | Capture |
|---|---|
| Accueil (`/`) | ![Accueil](docs/pictures/ESP32-Foundation-Accueil.jpeg) |
| Système (`/system`) | ![Système](docs/pictures/ESP32-Foundation-Systeme.jpeg) |
| Fichiers (`/files`) | ![Fichiers](docs/pictures/ESP32-Foundation-Fichiers.jpeg) |
| Logs (`/logs`) | ![Logs](docs/pictures/ESP32-Foundation-Logs.jpeg) |
| Mise à jour (`/ota`) | ![Mise à jour](docs/pictures/ESP32-Foundation-Mise-a-Jour.jpeg) |

## Journal des modifications

Voir [CHANGELOG.md](CHANGELOG.md) pour l'historique des versions.

## Licence

Distribué sous [licence MIT](LICENSE).
