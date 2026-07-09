# Voice Toy

Voice Toy is currently an ESP32-S3 firmware base with a small web console.
It does not contain a voice effects or audio processing module in this
revision. The firmware that actually builds today provides:

- WiFi connection with saved credentials and a fallback configuration portal.
- A LittleFS web interface served by the ESP32.
- Firmware update through the web UI and ArduinoOTA.
- Serial logs plus a JSON log buffer visible from the browser.
- System information in JSON.
- LittleFS file listing, upload, download and delete.
- Optional BootLog diagnostics for reset reasons and pre-reboot logs.

The target board is `esp32-s3-devkitc-1-n16r8v` through PlatformIO and the
Arduino framework.

## Project Layout

```text
.
├── include/
│   ├── board_config.h          ESP32-S3 pin notes
│   ├── project_config.h        Feature flags and project settings
│   └── secrets_example.h       Optional WiFi credentials template
├── src/
│   ├── main.cpp                Registers project modules and starts App
│   ├── api/api_router/         Thin wrapper around WebServer routing
│   ├── core/                   App, Module and ModuleManager
│   ├── modules/boot_log/       Optional reboot diagnostics module
│   └── services/               WiFi, web, OTA, logs, storage, config, mDNS...
├── web_src/                    Editable HTML/CSS/JS web interface sources
├── docs/
│   ├── BOOT_LOG.md             BootLog user guide
│   └── WIFI_SETUP.md           First WiFi setup guide
├── tools/                      Build, release and web packaging helpers
├── platformio.ini
├── VERSION
└── CHANGELOG.md
```

`data/` is generated from `web_src/` by `tools/minify_web.py` and is not meant
to be edited by hand.

## Build And Flash

```bash
pio run
python tools/minify_web.py
pio run --target uploadfs
pio run --target upload
```

Run `uploadfs` at least once after flashing a new board, otherwise the web
pages stored in LittleFS will be missing.

Optional development WiFi credentials can be created from the template:

```bash
cp include/secrets_example.h include/secrets.h
```

If no known WiFi network is available at boot, the ESP32 starts the access
point `ESP32-Setup`. See [docs/WIFI_SETUP.md](docs/WIFI_SETUP.md).

## Web Interface

Once connected to WiFi, open:

- `http://voicetoy.local/` when mDNS works on your network.
- Or the IP address printed on the serial monitor.

Pages provided by `web_src/`:

- `/` home page.
- `/system` live system information from `GET /api/system`.
- `/files` LittleFS browser.
- `/logs` in-memory log viewer.
- `/ota` firmware upload page.
- `/debug` BootLog page when `ENABLE_BOOT_LOG` is enabled.

## HTTP API

Core routes:

- `GET /api/system`
- `GET /api/logs`
- `DELETE /api/logs`
- `GET /api/files/list?path=/`
- `GET /api/files/download?path=/name`
- `DELETE /api/files/delete?path=/name`
- `POST /api/files/upload`
- `POST /api/ota/update`

BootLog routes, only when enabled:

- `GET /api/bootlog`
- `DELETE /api/bootlog`

## Configuration

Main settings live in `include/project_config.h`:

- `PROJECT_NAME` fallback value.
- `WEB_SERVER_PORT`
- `MDNS_HOSTNAME`
- `WIFI_CONNECT_TIMEOUT_MS`
- `WIFI_PORTAL_AP_NAME`
- Feature flags such as `ENABLE_WIFI`, `ENABLE_WEB_SERVER`, `ENABLE_OTA`,
  `ENABLE_MDNS` and `ENABLE_BOOT_LOG`.

`platformio.ini` also injects `PROJECT_NAME="Voice Toy"` for normal builds.

## Current Scope

This repository is now documented as the firmware that is present in the
source tree. Voice transformation, microphone input, speaker output and audio
effects are not implemented yet. Add those as real modules under
`src/modules/` when the hardware and behavior are defined.
