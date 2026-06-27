#pragma once

/**
 * board_config.h — Brochage matériel du projet ESP32 Voice Toy.
 *
 * Ce fichier ne contient aucune logique métier : uniquement des numéros de
 * broches pour l'écran, l'audio I2S, le potentiomètre et les boutons.
 */

#define BOARD_NAME "ESP32-S3 Voice Toy"

// I2C — capteurs / écran (pull-up 4.7 kΩ obligatoire)
#define I2C_SDA_PIN 41
#define I2C_SCL_PIN 42

// OLED SSD1306
#define OLED_WIDTH   128
#define OLED_HEIGHT   64
#define OLED_ADDRESS 0x3C

// Microphone INMP441 — L/R vers GND => canal gauche
#define I2S_MIC_WS_PIN  4
#define I2S_MIC_SCK_PIN 5
#define I2S_MIC_SD_PIN  6

// Amplificateur MAX98357A — GAIN vers GND, SD vers VIN
#define I2S_AMP_LRC_PIN  16
#define I2S_AMP_BCLK_PIN 15
#define I2S_AMP_DIN_PIN   7

// Commandes Voice Toy
#define POTENTIOMETER_PIN 2
#define BUTTON_MUTE_PIN   0   // strapping BOOT — ne rien forcer LOW au démarrage
#define BUTTON_PREV_PIN   39
#define BUTTON_NEXT_PIN   40

// SPI — écran / SD (si utilisé par le projet)
#define SPI_SCK_PIN  12
#define SPI_MOSI_PIN 11
#define SPI_MISO_PIN 13

// Boutons génériques
#define BUTTON_BOOT_PIN BUTTON_MUTE_PIN

// Sortie LED/NeoPixel (si présente sur la carte)
#define NEOPIXEL_PIN 48

// ============================================================
// RAPPELS DE SÉCURITÉ ESP32-S3
// ============================================================
// - GPIO 3.3V uniquement (aucune broche 5V tolérante).
// - GPIO0 : strapping BOOT — ne rien connecter qui force LOW.
// - GPIO46 : entrée uniquement, attention au boot/JTAG.
// - I2C : pull-up 4.7 kΩ obligatoire.
// ============================================================
