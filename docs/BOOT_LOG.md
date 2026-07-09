# BootLog

BootLog est un module optionnel de diagnostic de redémarrage. Il est activé par
défaut dans `include/project_config.h` avec :

```cpp
#define ENABLE_BOOT_LOG
```

Quand il est actif, Voice Toy ajoute :

- La page web `/debug`.
- L'API `GET /api/bootlog`.
- L'API `DELETE /api/bootlog`.
- Une capture des derniers logs applicatifs avant redémarrage.
- Des compteurs persistants de boots et de crashs.

## Ce qui est enregistré

À chaque démarrage, le module écrit une entrée dans `/bootlog.json` sur
LittleFS. L'entrée contient notamment :

- La raison du reset fournie par l'ESP32.
- Le nombre total de démarrages.
- Le nombre de resets considérés comme anormaux.
- La température interne au démarrage.
- Les derniers logs capturés en RAM RTC avant le reboot précédent.
- Un instantané périodique : uptime, heap libre, plus gros bloc libre, WiFi,
  nombre de pages servies et appels API si ces compteurs sont utilisés.

La RAM RTC survit à un redémarrage logiciel, à un watchdog ou à un panic. Elle
ne survit pas à une coupure d'alimentation.

## Utilisation

Ouvrir :

```text
http://voicetoy.local/debug
```

ou remplacer `voicetoy.local` par l'adresse IP affichée dans le moniteur série.

Le bouton `Vider` supprime l'historique stocké dans `/bootlog.json`. Il ne
remet pas les compteurs NVS à zéro.

## Désactiver

Commenter la macro dans `include/project_config.h` :

```cpp
// #define ENABLE_BOOT_LOG
```

Puis recompiler et reflasher le firmware.

Quand le module est désactivé :

- `/api/bootlog` n'est plus enregistré.
- `/debug` n'est plus enregistré.
- `web_src/menu.js` ne montre plus le lien `Debug`, car il sonde l'API avant
  d'ajouter l'entrée de menu.

## Supprimer complètement

Pour retirer le module du projet au lieu de seulement le désactiver :

1. Retirer les blocs `#ifdef ENABLE_BOOT_LOG` de `src/main.cpp`.
2. Retirer le hook BootLog dans `src/services/log_manager/log_manager.cpp`.
3. Supprimer `src/modules/boot_log/`.
4. Supprimer `web_src/debug.html`.
5. Supprimer `ENABLE_BOOT_LOG` et les constantes BootLog de
   `include/project_config.h`.
6. Retirer la sonde `/api/bootlog` de `web_src/menu.js`.

Le reste du firmware fonctionne sans BootLog.
