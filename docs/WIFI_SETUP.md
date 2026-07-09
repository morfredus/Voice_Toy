# Configuration WiFi

Voice Toy peut se connecter au WiFi de deux façons :

- Avec un fichier local `include/secrets.h`, pratique pendant le développement.
- Avec le portail de configuration intégré, pratique pour une carte déjà
  flashée ou utilisée sur un autre réseau.

Les réseaux ajoutés par le portail sont stockés en NVS et survivent aux
redémarrages.

## Option 1 : fichier secrets.h

Copier le modèle :

```bash
cp include/secrets_example.h include/secrets.h
```

Puis renseigner le SSID et le mot de passe dans `include/secrets.h`.

Ce fichier ne doit pas être versionné. Il sert seulement de réseau par défaut
si aucun réseau n'a encore été enregistré en NVS.

## Option 2 : portail de configuration

Si Voice Toy ne connaît aucun réseau ou ne peut pas se connecter dans le délai
`WIFI_CONNECT_TIMEOUT_MS`, il démarre un point d'accès :

```text
ESP32-Setup
```

Depuis un téléphone ou un ordinateur :

1. Se connecter au WiFi `ESP32-Setup`.
2. Ouvrir `http://192.168.4.1/`.
3. Choisir un réseau détecté, ou saisir le SSID manuellement.
4. Saisir le mot de passe.
5. Valider.

L'ESP32 enregistre le réseau, redémarre, puis tente de s'y connecter.

## Accéder à l'interface

Après connexion, l'adresse IP est affichée sur le moniteur série.

Si mDNS fonctionne sur le réseau, l'interface est aussi disponible ici :

```text
http://voicetoy.local/
```

## Réinitialiser les réseaux

Le firmware actuel expose l'ajout et la suppression de réseaux côté code
(`wifiMgr.addNetwork()` et `wifiMgr.removeNetwork()`), mais l'interface web ne
fournit pas encore d'écran de gestion WiFi.

Pour repartir de zéro pendant le développement, effacer la NVS avec PlatformIO
ou flasher un firmware qui appelle explicitement `wifiMgr.removeNetwork()` pour
les SSID enregistrés.
