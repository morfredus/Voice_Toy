#!/usr/bin/env python3
"""
release.py — Pipeline de release complet :
  1. version_generator.py (bump de version)
  2. minify_web.py (assets web_src/ -> data/)
  3. pio run (build firmware)
  4. package_web.py (build image LittleFS)

Usage :
  python tools/release.py [patch|minor|major]
"""

import subprocess
import sys
from pathlib import Path

TOOLS_DIR = Path(__file__).resolve().parent


def step(description: str, cmd: list) -> bool:
    print(f"\n=== {description} ===")
    result = subprocess.run(cmd)
    if result.returncode != 0:
        print(f"ÉCHEC : {description}")
        return False
    return True


def main():
    bump = sys.argv[1] if len(sys.argv) > 1 else "patch"

    steps = [
        ("Incrémentation de version", [sys.executable, str(TOOLS_DIR / "version_generator.py"), bump]),
        ("Minification des assets web", [sys.executable, str(TOOLS_DIR / "minify_web.py")]),
        ("Compilation du firmware", ["pio", "run"]),
        ("Construction de l'image LittleFS", [sys.executable, str(TOOLS_DIR / "package_web.py")]),
    ]

    for description, cmd in steps:
        if not step(description, cmd):
            sys.exit(1)

    print("\nRelease prête : .pio/build/<env>/firmware.bin + littlefs.bin")


if __name__ == "__main__":
    main()
