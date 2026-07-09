# Journal des modifications

Ce journal décrit uniquement l'état réel du dépôt Voice Toy.

## Non publié

### Modifié
- Documentation réécrite pour décrire le firmware réellement présent :
  console ESP32-S3 WiFi/Web/OTA, logs, fichiers LittleFS et BootLog optionnel.
- Nom réseau mDNS par défaut aligné sur le projet : `voicetoy`.
- Page d'accueil web mise à jour pour ne plus annoncer un framework générique
  ni des fonctions audio absentes.

### Supprimé
- Ancien module de démonstration et sa route `/api/example/counter`.
- Exemples et documents hérités qui décrivaient un framework générique plutôt
  que ce projet.
