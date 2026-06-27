# Guide d'intégration — démarrer un nouveau projet avec ESP32-Foundation

Ce guide couvre, pas à pas et sans prérequis sur ce dépôt, tout ce qu'il faut
pour partir d'ESP32-Foundation et construire un projet ESP32 concret. Pour le
détail des choix de conception et leur origine (Gateway Lab / MeteoHub), voir
[docs/ARCHITECTURE.md](ARCHITECTURE.md).

## Renommer le projet

Changer le nom affiché du projet (dans les logs, dans `/api/system`, et donc
dans l'interface web qui lit ce champ) ne demande qu'**une seule
modification** :

- Ouvrir `platformio.ini`, section `[env:...]` (ex. `[env:esp32s3_n16r8]`),
  et modifier la ligne suivante sous `build_flags` :

  ```ini
  build_flags =
      -std=gnu++17
      -D PROJECT_NAME='"MonProjet"'
  ```

  Remplacer `MonProjet` par le nom souhaité (conserver les guillemets
  imbriqués `'"..."'`, requis par la syntaxe `build_flags` de PlatformIO).

La seule autre donnée à mettre à jour, purement informative et sans aucun
effet sur la compilation ni sur le firmware, est la ligne `description` dans
la section `[platformio]` du même fichier :

```ini
[platformio]
description  = MonProjet - description courte du projet
```

Aucun autre fichier n'a besoin d'être modifié pour ce renommage : la valeur
de `PROJECT_NAME` se propage automatiquement à tout le code C++ via la macro
du même nom (utilisée par exemple dans `src/core/app.cpp` pour les logs de
démarrage, et dans `src/services/system_info/system_info.cpp` pour le champ
JSON `project` de `/api/system`), et à l'interface web (`web_src/index.html`,
`web_src/menu.js`) qui lit dynamiquement ce même champ `project` au chargement
de chaque page plutôt que d'avoir un nom écrit en dur.

`include/project_config.h` contient également :

```cpp
#ifndef PROJECT_NAME
#define PROJECT_NAME "ESP32-Foundation"
#endif
```

Cette ligne est une valeur de repli (fallback), utilisée uniquement si
`PROJECT_NAME` n'est pas défini via `build_flags` (par exemple en cas de
compilation manuelle hors PlatformIO) — elle n'a pas besoin d'être modifiée
dans l'usage normal, puisque `platformio.ini` définit toujours `PROJECT_NAME`.

Enfin, noter que `MDNS_HOSTNAME` (dans `include/project_config.h`, ou
surchargé dans le `project_config.h` propre à chaque projet — voir les
exemples sous `examples/`) est une **variable distincte** : c'est le nom
réseau utilisé pour la résolution mDNS (ex. `monprojet.local`). Il est
recommandé de le faire correspondre au nouveau nom du projet pour la
cohérence (ex. `MDNS_HOSTNAME "monprojet"`), mais ce n'est pas obligatoire :
les deux valeurs sont totalement indépendantes.

## Compiler et téléverser le firmware

Deux façons équivalentes de procéder : en ligne de commande (`pio`, fourni
par l'extension PlatformIO) ou via les boutons de la barre PlatformIO dans
VSCode (en bas de la fenêtre, ou dans l'onglet PROJECT TASKS de la barre
latérale).

| Action | Commande CLI | Bouton VSCode |
|---|---|---|
| Compiler le firmware | `pio run` | ✓ (coche, "Build") |
| Téléverser le firmware | `pio run -t upload` | → (flèche, "Upload") |
| Ouvrir le moniteur série | `pio device monitor` | 🔌 (prise, "Monitor") |

Si plusieurs cartes/ports série sont détectés, PlatformIO demande de choisir
le port à utiliser (ou le déduit automatiquement s'il n'y en a qu'un). La
vitesse du moniteur série est définie par `monitor_speed = 115200` dans
`platformio.ini` — il n'y a rien à changer côté terminal, PlatformIO
applique cette vitesse automatiquement à l'ouverture du moniteur.

## Téléverser l'interface web (LittleFS) — éviter la page blanche au premier démarrage

Point essentiel à comprendre, source numéro un de confusion chez les
débutants : **le firmware (code C++) et le système de fichiers LittleFS
(contenu de `data/`) sont deux éléments totalement séparés**, téléversés
indépendamment l'un de l'autre sur la carte ESP32.

- Le firmware (`pio run -t upload`) contient uniquement le code compilé
  (services + modules). Il ne contient PAS le contenu HTML/CSS/JS de
  l'interface web.
- Le système de fichiers LittleFS contient les fichiers du dossier `data/`
  (`index.html`, `style.css`, `menu.js`, etc.), servis tels quels par
  `WebManager` à chaque requête HTTP.

Conséquence concrète : après un flash uniquement du firmware, la mémoire
flash LittleFS est vide. Toute requête HTTP vers la carte renvoie alors une
page blanche ou une erreur 404, **même si le firmware fonctionne
parfaitement** — ce n'est pas un bug, juste un système de fichiers vide.

`data/` est un dossier **généré**, jamais édité à la main : les sources web
vivent dans `web_src/` (`index.html`, `style.css`, `menu.js`, etc.). Lancer
`tools/minify_web.py` les minifie vers `data/`. C'est `data/` qui est
téléversé sur la carte.

Workflow pour modifier l'interface web :

```bash
# 1. Éditer les sources dans web_src/
# 2. Minifier vers data/ (répond ensuite "o" au prompt pour lancer
#    uploadfs directement, ou "N" pour le faire soi-même à l'étape 3)
python tools/minify_web.py
# 3. Téléverser data/ vers LittleFS
pio run -t uploadfs
```

Ou, dans VSCode : onglet PROJECT TASKS (barre latérale PlatformIO) → choisir
l'environnement (ex. `esp32s3_n16r8`) → "Upload Filesystem Image".

À retenir :
- Exécuter `uploadfs` **une fois après le tout premier flash du firmware**,
  pour que l'interface web soit disponible dès le départ.
- Ré-exécuter `python tools/minify_web.py` puis `uploadfs` **chaque fois que
  le contenu de `web_src/` change** (nouvelle page, modification de style,
  etc.) — le flash du firmware seul ne suffit jamais à mettre à jour
  l'interface web.
- `tools/minify_web.py` est **volontairement manuel** (pas dans
  `extra_scripts` de `platformio.ini`) : il ne doit s'exécuter que lorsque
  `web_src/` a réellement changé, pas à chaque compilation du firmware
  (`pio run -t upload` n'a aucune raison de toucher à `data/`). Voir
  `tools/release.py` pour le pipeline complet qui l'enchaîne automatiquement
  avant un build de release.

## Comprendre l'architecture Module/Service

Deux notions à bien distinguer dans ce framework :

- **Service** (`src/services/*`, plus `src/core/*` et `src/api/*`) : brique
  générique fournie par le framework — gestion WiFi (`WiFiManager`), serveur
  web (`WebManager`), mises à jour OTA (`OtaManager`), stockage de fichiers
  (`StorageManager`), réglages persistants (`ConfigManager`), journalisation
  (`LogManager`), informations système (`SystemInfo`), résolution réseau
  (`MdnsManager`), heure réseau (`TimeManager`). Un nouveau projet **ne
  modifie jamais** ces services : ils sont communs à tous les projets basés
  sur ce framework.
- **Module** (`src/modules/*`) : code métier propre à un projet — ce que le
  projet *fait* réellement (lire un capteur, exposer une API spécifique,
  piloter une LED...). C'est la seule partie du code qu'un nouveau projet
  écrit ou modifie. Voir `src/modules/example_module/` et les trois exemples
  sous `examples/` (`minimal`, `sensor`, `api`) comme modèles.

Ordre d'orchestration au démarrage, défini dans `src/core/app.cpp`
(`App::begin()`) — voir aussi [docs/ARCHITECTURE.md](ARCHITECTURE.md#flux-de-démarrage-appbegin)
pour le détail :

1. `config.begin()` — initialise le stockage persistant (NVS).
2. `storage.begin()` — monte le système de fichiers LittleFS.
3. `wifiMgr.begin()` — connecte au WiFi (ou démarre le portail captif).
4. `webMgr.begin()` — démarre le serveur HTTP et ses routes internes
   (`/`, `/logs`, `/files`, `/system`, `/ota`).
5. `modules.registerAllRoutes(router)` — chaque module ajouté via
   `app.modules.add(...)` enregistre ses propres routes HTTP, s'il en a.
6. `modules.beginAll()` — chaque module exécute son initialisation propre.

Ensuite, à chaque itération de `App::loop()` : les services WiFi/OTA/web
exécutent leur traitement périodique, puis `modules.loopAll()` appelle
`loop()` sur chaque module métier enregistré, dans l'ordre d'ajout.

## Créer son premier module, pas à pas

Les trois exemples fournis illustrent ces étapes avec une complexité
croissante :
- `examples/minimal/` — cycle de vie minimal (begin/loop), sans route ni
  réglage. À lire en premier.
- `examples/sensor/` — lecture périodique d'une valeur simulée, réglage
  persistant (`config.getInt/setInt`), route HTTP de lecture.
- `examples/api/` — routage HTTP avancé : GET simple, GET avec paramètre de
  requête, POST avec corps JSON et validation.
- `src/modules/example_module/` — exemple intégré directement dans ce
  dépôt, combinant log + réglage persistant + route HTTP.
- `src/modules/boot_log/` — exemple intégré directement dans ce dépôt d'un
  module **optionnel** (désactivé par défaut, activable via
  `#define ENABLE_BOOT_LOG` dans `project_config.h`) et **entièrement
  supprimable** sans impact ailleurs dans le framework — voir
  [docs/BOOT_LOG.md](BOOT_LOG.md) pour le détail et la procédure de retrait.
  Illustre aussi un pattern réutilisable pour un lien de menu conditionnel :
  `web_src/menu.js` sonde la route HTTP du module (`GET /api/bootlog`) au
  chargement de chaque page et n'ajoute le lien de navigation que si elle
  répond, évitant tout lien mort quand le module est désactivé ou retiré.

Étapes pour créer un nouveau module dans un projet basé sur ce framework :

**a) Créer le fichier d'en-tête `src/modules/<nom>/<nom>.h`**

Définir une classe héritant de `Module` (`src/core/module.h`) et redéfinir
au minimum `name()` :

```cpp
#pragma once
#include "../../core/module.h"

class MonModule : public Module {
public:
    const char* name() const override { return "MonModule"; }
    void begin() override;
    void loop() override;
    void registerRoutes(WebRouter& router) override;   // facultatif
};
```

`registerRoutes()` peut être complètement omise si le module n'expose aucune
route HTTP — `Module` fournit déjà une version par défaut qui ne fait rien
(voir `examples/minimal/src/minimal_module.h`).

**b) Créer le fichier source `src/modules/<nom>/<nom>.cpp`** correspondant,
avec l'implémentation de chaque méthode redéfinie.

**c) Inclure les bons en-têtes**

Depuis un module placé dans `src/modules/<nom>/`, utiliser le chemin relatif
déjà en usage dans le reste du dépôt :

```cpp
#include "../../core/module.h"                      // classe Module
#include "../../services/log_manager/log_manager.h"  // LOG_INFO, etc.
#include "../../services/config_manager/config_manager.h" // config
#include "../../api/api_router/api_router.h"         // WebRouter complet (si routes)
```

(Dans un exemple sous `examples/`, qui vit hors de `src/`, le chemin relatif
est plus long — voir `examples/sensor/src/sensor_module.cpp` :
`../../../src/services/...`.)

**d) Instancier et enregistrer le module dans `src/main.cpp`**

```cpp
#include "modules/mon_module/mon_module.h"

static MonModule monModule;

void setup() {
    Serial.begin(115200);
    app.modules.add(&monModule);   // enregistrer AVANT app.begin()
    app.begin();
}

void loop() {
    app.loop();
}
```

`app.modules.add(&monModule)` doit être appelé avant `app.begin()`, afin que
`registerRoutes()` et `begin()` soient bien invoquées sur ce module pendant
le démarrage de l'application.

**e) Utiliser les macros `LOG_*` avec un tag cohérent**

Convention courante dans ce dépôt : utiliser `name()` comme tag de log
(comme dans `example_module.cpp`), ou définir un tag statique dédié :

```cpp
static const char* TAG = "MonModule";
LOG_INFO(TAG, "Valeur lue : %d", valeur);
```

Les niveaux disponibles sont `LOG_DEBUG`, `LOG_INFO`, `LOG_WARNING`,
`LOG_ERROR` (voir `src/services/log_manager/log_manager.h`) — chacun prend
un tag, puis un format `printf`-style et ses arguments.

**f) Persister un réglage via ConfigManager**

```cpp
int intervalle = config.getInt("mon_intervalle_ms", 5000); // lecture (avec défaut)
config.setInt("mon_intervalle_ms", 2000);                   // écriture
```

Voir aussi `getString/setString` et `getBool/setBool` pour d'autres types.
Ces réglages sont stockés en NVS (mémoire flash dédiée, persistante entre
redémarrages), dans un espace de noms partagé par défaut ("app").

**g) Lire/écrire un fichier via StorageManager**

```cpp
String contenu = storage.readFile("/mon_fichier.txt");
storage.writeFile("/mon_fichier.txt", "nouveau contenu");
bool present = storage.exists("/mon_fichier.txt");
```

Ces fichiers vivent dans LittleFS — donc dans le même système de fichiers
que l'interface web (`data/`), mais sous des chemins distincts si on évite
de réutiliser les noms déjà utilisés par l'interface (`index.html`, etc.).

**h) Ajouter une route HTTP via WebRouter — et le piège des lambdas**

```cpp
void MonModule::registerRoutes(WebRouter& router) {
    WebServer& server = router.raw();
    router.get("/api/mon_module/valeur", [this, &server]() {
        server.send(200, "application/json", "{\"valeur\":42}");
    });
}
```

Point crucial à retenir : **toujours extraire `WebServer& server =
router.raw();` puis capturer `&server` dans la lambda — jamais capturer
`router` lui-même par référence**. Le paramètre `router` reçu par
`registerRoutes()` est une référence vers un objet `WebRouter` temporaire
(construit à la volée par `WebManager::router()`), qui n'existe plus une
fois `registerRoutes()` terminée. Si une lambda enregistrée comme gestionnaire
HTTP (et donc stockée durablement par `WebServer` sous forme de
`std::function`) capturait `router` par référence, cette référence
deviendrait pendante (dangling reference) dès la fin de `registerRoutes()` —
provoquant un comportement indéfini (souvent un crash) à la première requête
HTTP reçue. `server`, en revanche, désigne l'objet `WebServer` réel détenu
par `webMgr`, qui survit pendant toute la durée de vie du programme : le
capturer par référence est donc sûr.

## Liste des services disponibles et leur API publique

| Service | Rôle et API publique |
|---|---|
| **LogManager** (`logMgr`) | Journalisation unifiée. Macros `LOG_DEBUG/INFO/WARNING/ERROR(tag, format, ...)`. Tampon mémoire circulaire consultable via `logMgr.count()/entry()/clear()`, exposé en JSON sur `GET /api/logs` (et `DELETE /api/logs` pour vider) et affiché sur la page web `/logs`. Niveau de verbosité réglable via `-D LOG_LEVEL=N` (0=off, 1=error, 2=warn, 3=info [défaut], 4=debug). |
| **WiFiManager** (`wifiMgr`) | Connexion WiFi multi-réseaux. `begin(callback)`, `loop()`, `isConnected()`, `isApMode()`, `ssid()`, `localIP()`, `rssi()`, plus gestion des réseaux enregistrés (`savedNetworks()`, `addNetwork()`, `removeNetwork()`). Si aucun réseau connu ne répond, démarre automatiquement un point d'accès et un portail de configuration captif. |
| **StorageManager** (`storage`) | Fichiers LittleFS. `readFile(path)`, `writeFile(path, contenu)`, `deleteFile(path)`, `listFiles(path)`, `exists(path)`, `totalBytes()/usedBytes()`. Utilisé à la fois par les modules métier et par `WebManager` pour servir les pages de `data/`. |
| **ConfigManager** (`config`) | Réglages persistants clé/valeur, stockés en NVS (Preferences), namespace partagé "app" par défaut. `getInt/setInt`, `getString/setString`, `getBool/setBool`, `remove(key)`. |
| **OtaManager** (`otaMgr`) | Mise à jour du firmware par le réseau. Deux mécanismes : ArduinoOTA (réseau local, `pio run -t upload --upload-port <host>.local`) et upload web (page `/ota`, route `POST /api/ota/update`). Activé via `#define ENABLE_OTA` dans `project_config.h` (activé par défaut). |
| **MdnsManager** (`mdnsMgr`) | Résolution du nom `http://<MDNS_HOSTNAME>.local` sur le réseau local, démarrée automatiquement à la première connexion WiFi si `ENABLE_MDNS` est défini (par défaut, oui). |
| **TimeManager** (`timeMgr`) | Synchronisation NTP. `begin(tz, ntpServer)`, `isSynced()`, `now()`, `isoTimestamp()`. **Désactivé par défaut** (`// #define ENABLE_TIME_SYNC` est commenté dans `project_config.h`) — pour l'activer, décommenter cette ligne dans le `project_config.h` du projet, et éventuellement ajuster `TIME_ZONE`/`NTP_SERVER`. |
| **SystemInfo** (`systemInfo`) | État système exposé en JSON sur `GET /api/system` (affiché sur la page `/system`), avec les champs suivants (voir `src/services/system_info/system_info.cpp`) : `project` (nom du projet, `PROJECT_NAME`), `version` (`PROJECT_VERSION`), `build_date`, `build_time`, `git_commit`, `uptime` (secondes), `free_heap`, `chip_model`, `cpu_freq_mhz`, `wifi` (SSID ou `"disconnected"`), `rssi`, `ip`, `littlefs.total`/`littlefs.used`, et `free_psram`/`total_psram` si la carte dispose de PSRAM. |
| **WebManager** (`webMgr`) | Serveur HTTP (`WebServer`, synchrone). Routes internes : `/`, `/logs`, `/files`, `/system`, `/ota` (pages HTML servies depuis `data/`), plus les API listées ci-dessus. Sert aussi en fallback tout fichier statique présent dans `data/` et non repris par une route explicite. Expose `router()` pour permettre aux modules d'ajouter leurs propres routes via `WebRouter` (voir section précédente). |

Aucune dépendance métier n'est imposée par ces services : chacun reste
utilisable indépendamment des autres.
