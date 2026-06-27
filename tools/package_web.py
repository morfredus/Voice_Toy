#!/usr/bin/env python3
"""
package_web.py — Construit l'image LittleFS à partir de data/ (généré par
minify_web.py depuis web_src/) et l'écrit dans .pio/build/<env>/littlefs.bin,
prête à être flashée avec :
    pio run --target uploadfs

Wrapper fin autour de la commande PlatformIO pour l'inclure dans un pipeline
de release (voir release.py).
"""

import subprocess
import sys


def run():
    result = subprocess.run(["pio", "run", "--target", "buildfs"])
    return result.returncode == 0


if __name__ == "__main__":
    sys.exit(0 if run() else 1)
