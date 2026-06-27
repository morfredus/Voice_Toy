# Journal des modifications

Toutes les évolutions notables du projet sont consignées dans ce fichier.

Le format s'inspire de [Keep a Changelog](https://keepachangelog.com/fr/1.0.0/),
et le projet suit le [versionnage sémantique](https://semver.org/lang/fr/)
(`VERSION` à la racine).

## [0.2.1]

### Ajouté
- `docs/WIFI_SETUP.md` : guide détaillé du premier démarrage WiFi
  (hiérarchie NVS / `secrets.h` / portail de configuration, procédure de
  connexion au portail, ajout/retrait de réseaux).
- Barre de progression réelle sur la page `/ota` (suivi de l'upload via
  `XMLHttpRequest`), confirmation inline de réussite/échec, et retour
  automatique à la page d'accueil une fois l'ESP32 redémarré.

### Modifié
- Séparation des sources web et de l'image LittleFS : les fichiers HTML/CSS/JS
  non minifiés vivent désormais dans `web_src/` (à éditer), tandis que `data/`
  est un dossier généré par `tools/minify_web.py` (gitignored, ne jamais
  modifier à la main).
- `tools/minify_web.py` est désormais entièrement manuel : retiré de
  `extra_scripts` dans `platformio.ini` (il ne s'exécute plus à chaque
  compilation du firmware). Après minification, le script propose
  d'enchaîner directement avec `pio run --target uploadfs`.
- Documentation (`README.md`, `README.fr.md`, `docs/INTEGRATION_GUIDE.md`,
  `docs/BOOT_LOG.md`) mise à jour pour refléter la séparation `web_src/` / `data/`.

## [0.2.0]

### Ajouté
- Module optionnel `bootLogModule` : journal de redémarrage (raison du
  reset, derniers logs, instantané système avant crash), désactivé par
  défaut (`#define ENABLE_BOOT_LOG`). Voir `docs/BOOT_LOG.md`.
- Captures d'écran des principales pages de l'interface web dans le README.

## [0.1.0]

### Ajouté
- Version initiale du framework ESP32-Foundation, généralisée à partir de
  Gateway Lab V1 et MeteoHub-S3 : `wifiMgr` (multi-réseaux NVS, portail
  captif, reconnexion auto), `webMgr` (serveur HTTP et interface web),
  `otaMgr` (mises à jour firmware), `storage` (LittleFS), `config` (réglages
  persistants NVS), `logMgr` (logs série + tampon JSON), `systemInfo`,
  `mdnsMgr`, `timeMgr`.
- Système de modules optionnels (`Module`, `ModuleManager`) et exemples
  fournis (`examples/minimal`, `examples/sensor`, `examples/api`,
  `examples/example_project`).
- Outils de build (`tools/build_info.py`, `tools/version_generator.py`,
  `tools/release.py`) et documentation (`docs/ARCHITECTURE.md`,
  `docs/INTEGRATION_GUIDE.md`).
- Licence MIT.
