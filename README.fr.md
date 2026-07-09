# Voice Toy

Voice Toy est actuellement une base firmware ESP32-S3 avec une petite console
web. Cette révision ne contient pas encore de module de traitement audio, de
microphone, de sortie haut-parleur ou d'effets vocaux. Le firmware réellement
présent dans le dépôt fournit :

- Connexion WiFi avec identifiants enregistrés et portail de configuration de
  secours.
- Interface web servie depuis LittleFS.
- Mise à jour firmware depuis le navigateur et par ArduinoOTA.
- Logs série avec tampon JSON consultable dans le navigateur.
- Informations système au format JSON.
- Liste, envoi, téléchargement et suppression de fichiers LittleFS.
- Diagnostic optionnel BootLog pour les raisons de redémarrage et les derniers
  logs avant reboot.

La cible PlatformIO est `esp32-s3-devkitc-1-n16r8v` avec le framework Arduino.

## Organisation du projet

```text
.
├── include/
│   ├── board_config.h          Notes de brochage ESP32-S3
│   ├── project_config.h        Options et réglages du projet
│   └── secrets_example.h       Modèle optionnel d'identifiants WiFi
├── src/
│   ├── main.cpp                Enregistre les modules et démarre App
│   ├── api/api_router/         Adaptateur léger autour de WebServer
│   ├── core/                   App, Module et ModuleManager
│   ├── modules/boot_log/       Module optionnel de diagnostic reboot
│   └── services/               WiFi, web, OTA, logs, stockage, config, mDNS...
├── web_src/                    Sources HTML/CSS/JS modifiables
├── docs/
│   ├── BOOT_LOG.md             Guide utilisateur BootLog
│   └── WIFI_SETUP.md           Guide de première connexion WiFi
├── tools/                      Scripts de build, release et packaging web
├── platformio.ini
├── VERSION
└── CHANGELOG.md
```

Le dossier `data/` est généré depuis `web_src/` par `tools/minify_web.py`. Il
ne doit pas être modifié à la main.

## Compiler et flasher

```bash
pio run
python tools/minify_web.py
pio run --target uploadfs
pio run --target upload
```

Il faut exécuter `uploadfs` au moins une fois après le flash d'une nouvelle
carte, sinon les pages web stockées dans LittleFS seront absentes.

Pour utiliser des identifiants WiFi de développement :

```bash
cp include/secrets_example.h include/secrets.h
```

Si aucun réseau connu n'est disponible au démarrage, l'ESP32 démarre le point
d'accès `ESP32-Setup`. Voir [docs/WIFI_SETUP.md](docs/WIFI_SETUP.md).

## Interface web

Une fois connecté au WiFi, ouvrir :

- `http://voicetoy.local/` si mDNS fonctionne sur le réseau.
- Ou l'adresse IP affichée dans le moniteur série.

Pages fournies par `web_src/` :

- `/` accueil.
- `/system` informations système depuis `GET /api/system`.
- `/files` navigateur LittleFS.
- `/logs` consultation des logs en mémoire.
- `/ota` téléversement d'un firmware.
- `/debug` page BootLog quand `ENABLE_BOOT_LOG` est activé.

## API HTTP

Routes principales :

- `GET /api/system`
- `GET /api/logs`
- `DELETE /api/logs`
- `GET /api/files/list?path=/`
- `GET /api/files/download?path=/nom`
- `DELETE /api/files/delete?path=/nom`
- `POST /api/files/upload`
- `POST /api/ota/update`

Routes BootLog, uniquement si le module est actif :

- `GET /api/bootlog`
- `DELETE /api/bootlog`

## Configuration

Les réglages principaux sont dans `include/project_config.h` :

- `PROJECT_NAME`, valeur de repli du nom projet.
- `WEB_SERVER_PORT`
- `MDNS_HOSTNAME`
- `WIFI_CONNECT_TIMEOUT_MS`
- `WIFI_PORTAL_AP_NAME`
- Les options `ENABLE_WIFI`, `ENABLE_WEB_SERVER`, `ENABLE_OTA`,
  `ENABLE_MDNS` et `ENABLE_BOOT_LOG`.

`platformio.ini` injecte aussi `PROJECT_NAME="Voice Toy"` lors des builds
normaux.

## Périmètre actuel

Cette documentation décrit volontairement le code présent dans le dépôt. Les
fonctions de transformation de voix ne sont pas encore implémentées. Elles
devront être ajoutées comme vrais modules dans `src/modules/` lorsque le
matériel audio et le comportement attendu seront définis.
